#include "SoulkeeperManager.h"
#include "Settings.h"
#include "PlayerManager.h"
#include "FollowerManager.h"
#include "WeaponManager.h"
#include "SoulGemManager.h"
#include "PurchaseManager.h"
#include "PassiveRechargeManager.h"
#include "NotificationManager.h"
#include "MenuManager.h"

namespace Soulkeeper
{
    SoulkeeperManager* SoulkeeperManager::GetSingleton()
    {
        static SoulkeeperManager singleton;
        return std::addressof(singleton);
    }

    void SoulkeeperManager::Initialize()
    {
        Settings::GetSingleton()->Load();
        NotificationManager::GetSingleton()->ClearCooldowns();
        PassiveRechargeManager::GetSingleton()->ResetGameTimeTracker();
        MenuManager::GetSingleton()->Register();

        logger::info("Soulkeeper Manager initialized.");
    }

    // Re-asserts weapon charges for player and all active followers (Legacy / no-op as extraLists are persistent)
    void SoulkeeperManager::ReassertAllWeaponCharges()
    {
    }

    RE::BSEventNotifyControl SoulkeeperManager::ProcessEvent(
        const RE::MenuOpenCloseEvent*,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
        return RE::BSEventNotifyControl::kContinue;
    }

    void SoulkeeperManager::ProcessFollowers()
    {
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableFollowerAutoCharge) return;

        auto followers = FollowerManager::GetSingleton()->GetActiveFollowers();
        for (auto* follower : followers) {
            if (!follower) continue;

            // Restock soul gems if follower is in town near a vendor
            PurchaseManager::GetSingleton()->TryFollowerRestockStockGems(follower);

            auto weapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(follower);
            for (const auto& wpn : weapons) {
                if (wpn.chargePercent > settings->fChargeThreshold) continue;

                float targetCapPercent = std::clamp(settings->fAutoChargeTargetPercent, 1.0f, 100.0f);
                float targetMaxCharge = wpn.maxCharge * (targetCapPercent / 100.0f);

                EnchantedWeaponInfo current = wpn;
                bool anyPurchaseAttempted = false;

                while (current.currentCharge < targetMaxCharge) {
                    float missing = targetMaxCharge - current.currentCharge;
                    auto bestGem = SoulGemManager::GetSingleton()->FindBestSoulGemForCharge(follower, missing);

                    if (bestGem) {
                        // 1. Charge using follower's own soul gem
                        float previousCharge = current.currentCharge;
                        bool ok = SoulGemManager::GetSingleton()->ChargeWeaponWithSoulGem(follower, current, *bestGem);
                        if (!ok) break;

                        current.currentCharge = std::min(previousCharge + static_cast<float>(bestGem->chargeValue), current.maxCharge);
                        current.chargePercent = (current.currentCharge / current.maxCharge) * 100.0f;
                    } else {
                        // 2. No soul gem in follower inventory -> Attempt follower purchase (once per weapon per check)
                        bool purchased = false;
                        if (settings->bEnableFollowerPurchase && !anyPurchaseAttempted) {
                            anyPurchaseAttempted = true;
                            purchased = PurchaseManager::GetSingleton()->TryFollowerPurchaseSoulGem(follower, current);
                        }

                        // 3. If purchase is disabled, already attempted, or failed, notify lack of soul gems and stop
                        if (!purchased) {
                            NotificationManager::GetSingleton()->NotifyFollowerNoSoulGems(follower);
                            break;
                        }

                        // Satın alma sonrası bir defaya mahsus refreshed oku
                        auto refreshed = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(follower);
                        auto it = std::find_if(refreshed.begin(), refreshed.end(), [&](const EnchantedWeaponInfo& w) {
                            return w.weapon == current.weapon && w.isLeftHand == current.isLeftHand;
                        });
                        if (it == refreshed.end()) break;
                        current = *it;
                    }
                }
            }
        }
    }

    void SoulkeeperManager::Update(float a_deltaSeconds)
    {
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableMod) return;

        auto ui = RE::UI::GetSingleton();
        if (ui && ui->GameIsPaused()) {
            return;
        }

        // 1. Process Passive Recharge
        PassiveRechargeManager::GetSingleton()->ProcessPassiveRecharge(a_deltaSeconds);

        // 2. Process Periodic Weapon Auto-Charge checks
        _checkTimer += a_deltaSeconds;
        if (_checkTimer >= settings->fCheckIntervalSeconds) {
            _checkTimer = 0.0f;
            logger::info("[SoulkeeperManager] Periodic check fired (interval: {:.0f}s).", settings->fCheckIntervalSeconds);

            // Player check
            PlayerManager::GetSingleton()->ProcessPlayer();

            // Follower check
            ProcessFollowers();
        }
    }
}
