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

        if (auto* ui = RE::UI::GetSingleton()) {
            ui->AddEventSink<RE::MenuOpenCloseEvent>(this);
            logger::info("[SoulkeeperManager] Registered MenuOpenCloseEvent sink.");
        }

        if (auto* scriptEventHolder = RE::ScriptEventSourceHolder::GetSingleton()) {
            scriptEventHolder->AddEventSink<RE::TESHitEvent>(this);
            scriptEventHolder->AddEventSink<RE::TESPlayerBowShotEvent>(this);
            logger::info("[SoulkeeperManager] Registered TESHitEvent and TESPlayerBowShotEvent sinks.");
        }

        logger::info("Soulkeeper Manager initialized.");
    }

    // Re-asserts weapon charges for player and all active followers (Legacy / no-op as extraLists are persistent)
    void SoulkeeperManager::ReassertAllWeaponCharges()
    {
    }

    RE::BSEventNotifyControl SoulkeeperManager::ProcessEvent(
        const RE::MenuOpenCloseEvent* a_event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>*)
    {
        if (!a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // When inventory or container (follower trade/exchange) menu is closed (!opening)
        if (!a_event->opening) {
            if (a_event->menuName == RE::ContainerMenu::MENU_NAME ||
                a_event->menuName == RE::InventoryMenu::MENU_NAME ||
                a_event->menuName == RE::BarterMenu::MENU_NAME) {
                
                logger::info("[SoulkeeperManager] Menu '{}' closed. Immediately processing weapon auto-charges...", a_event->menuName.c_str());
                
                // Immediately process player and follower charging with newly given soul gems
                PlayerManager::GetSingleton()->ProcessPlayer();
                ProcessFollowers();
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl SoulkeeperManager::ProcessEvent(
        const RE::TESHitEvent* a_event,
        RE::BSTEventSource<RE::TESHitEvent>*)
    {
        if (!a_event) return RE::BSEventNotifyControl::kContinue;

        // Check aggressor: must be player or active follower
        auto aggressorRef = a_event->cause.get();
        if (!aggressorRef || !aggressorRef->Is(RE::FormType::ActorCharacter))
            return RE::BSEventNotifyControl::kContinue;

        auto* actor = aggressorRef->As<RE::Actor>();
        if (!actor) return RE::BSEventNotifyControl::kContinue;

        bool isPlayer = actor->IsPlayerRef();
        bool isFollower = !isPlayer && FollowerManager::GetSingleton()->IsActiveFollower(actor);
        if (!isPlayer && !isFollower) return RE::BSEventNotifyControl::kContinue;

        // ---------------------------------------------------------------
        // Classify hit type from projectile and source fields:
        //   projectile == 0            → melee
        //   projectile != 0, source = weapon FormID (kCrossbow) → crossbow bolt
        //   projectile != 0, source = weapon FormID (kBow)      → bow (skip — TESPlayerBowShotEvent)
        //   projectile != 0, source = spell/enchantment FormID  → staff
        // ---------------------------------------------------------------
        enum class HitType { kMelee, kCrossbow, kBow, kStaff, kUnknown };
        HitType hitType = HitType::kUnknown;

        if (a_event->projectile == 0) {
            hitType = HitType::kMelee;
        } else {
            // Projectile hit: check what the source form is
            RE::TESObjectWEAP* sourceWeapon = nullptr;
            if (a_event->source != 0) {
                if (auto* sf = RE::TESForm::LookupByID(a_event->source)) {
                    sourceWeapon = sf->As<RE::TESObjectWEAP>();
                }
            }

            if (sourceWeapon) {
                // Source is a physical weapon
                auto wt = sourceWeapon->GetWeaponType();
                if (wt == RE::WEAPON_TYPE::kCrossbow) {
                    hitType = HitType::kCrossbow;
                } else if (wt == RE::WEAPON_TYPE::kBow || sourceWeapon->IsRanged()) {
                    hitType = HitType::kBow; // Handled by TESPlayerBowShotEvent
                }
                // Other weapon types with projectile are unusual — leave as kUnknown
            } else {
                // Source is a spell / enchantment FormID → staff
                hitType = HitType::kStaff;
            }
        }

        // Bows handled elsewhere; unknown types skipped
        if (hitType == HitType::kBow || hitType == HitType::kUnknown)
            return RE::BSEventNotifyControl::kContinue;

        // Get equipped enchanted weapons
        auto weapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(actor);
        if (weapons.empty()) return RE::BSEventNotifyControl::kContinue;

        for (const auto& wpn : weapons) {
            if (!wpn.IsValid() || !wpn.enchantment) continue;

            auto wt = wpn.weapon->GetWeaponType();

            // --- Match weapon to hit type ---
            if (hitType == HitType::kMelee) {
                // Skip ranged and staves
                if (wt == RE::WEAPON_TYPE::kBow || wt == RE::WEAPON_TYPE::kCrossbow ||
                    wt == RE::WEAPON_TYPE::kStaff || wpn.weapon->IsRanged()) continue;
                // Match by source FormID if provided
                if (a_event->source != 0 && a_event->source != wpn.weapon->GetFormID()) continue;

            } else if (hitType == HitType::kCrossbow) {
                // Only crossbows; source FormID == weapon FormID
                if (wt != RE::WEAPON_TYPE::kCrossbow) continue;
                if (a_event->source != 0 && a_event->source != wpn.weapon->GetFormID()) continue;

            } else { // kStaff
                // Source is enchantment FormID — match against staff's enchantment or weapon
                if (wt != RE::WEAPON_TYPE::kStaff) continue;
                if (a_event->source != 0) {
                    bool matchesWeapon = (a_event->source == wpn.weapon->GetFormID());
                    bool matchesEnch   = (wpn.enchantment &&
                                          a_event->source == wpn.enchantment->GetFormID());
                    if (!matchesWeapon && !matchesEnch) continue;
                }
            }

            // --- Deduct charge ---
            float cost = wpn.enchantment->CalculateMagickaCost(actor);
            if (cost <= 0.0f) {
                cost = (wpn.enchantment->data.costOverride > 0)
                    ? static_cast<float>(wpn.enchantment->data.costOverride)
                    : 25.0f;
            }

            float newCharge = std::max(0.0f, wpn.currentCharge - cost);

            const char* typeStr = (hitType == HitType::kMelee)    ? "Melee"
                                : (hitType == HitType::kCrossbow) ? "Crossbow"
                                :                                    "Staff";
            logger::info("[TESHitEvent] {} hit by '{}' with '{}' (cost: {:.1f}) -> {:.0f} -> {:.0f}/{:.0f}",
                typeStr,
                actor->GetDisplayFullName() ? actor->GetDisplayFullName() : "Actor",
                wpn.weapon->GetName(), cost,
                wpn.currentCharge, newCharge, wpn.maxCharge);

            WeaponManager::GetSingleton()->SetWeaponCharge(actor, wpn, newCharge);
            break;
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl SoulkeeperManager::ProcessEvent(
        const RE::TESPlayerBowShotEvent* a_event,
        RE::BSTEventSource<RE::TESPlayerBowShotEvent>*)
    {
        if (!a_event || a_event->weapon == 0) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto* player = RE::PlayerCharacter::GetSingleton();
        if (!player) {
            return RE::BSEventNotifyControl::kContinue;
        }

        // Find the equipped enchanted bow/crossbow that matches the fired weapon.
        // Then proactively deduct charge — same approach as TESHitEvent for melee.
        // The engine does NOT flush ExtraCharge to liveEntry in real time for ranged
        // weapons, so we track it ourselves via SetWeaponCharge.
        auto weapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(player);
        for (const auto& wpn : weapons) {
            if (!wpn.IsValid() || !wpn.enchantment) continue;
            if (wpn.weapon->GetFormID() != a_event->weapon) continue;

            auto wt = wpn.weapon->GetWeaponType();
            if (wt != RE::WEAPON_TYPE::kBow && wt != RE::WEAPON_TYPE::kCrossbow) continue;

            float cost = wpn.enchantment->CalculateMagickaCost(player);
            if (cost <= 0.0f) {
                if (wpn.enchantment->data.costOverride > 0)
                    cost = static_cast<float>(wpn.enchantment->data.costOverride);
                else
                    cost = 25.0f;
            }

            float newCharge = std::max(0.0f, wpn.currentCharge - cost);
            logger::info("[TESPlayerBowShotEvent] '{}' fired (cost: {:.1f}) -> {:.0f} -> {:.0f}/{:.0f}",
                wpn.weapon->GetName(), cost, wpn.currentCharge, newCharge, wpn.maxCharge);

            WeaponManager::GetSingleton()->SetWeaponCharge(player, wpn, newCharge);
            break;
        }

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
