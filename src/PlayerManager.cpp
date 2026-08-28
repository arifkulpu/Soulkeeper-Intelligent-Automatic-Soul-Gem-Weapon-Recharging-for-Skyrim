#include "PlayerManager.h"
#include "Settings.h"
#include "WeaponManager.h"
#include "SoulGemManager.h"

namespace Soulkeeper
{
    PlayerManager* PlayerManager::GetSingleton()
    {
        static PlayerManager singleton;
        return std::addressof(singleton);
    }

    void PlayerManager::ProcessPlayer()
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || player->IsDead()) return;

        auto settings = Settings::GetSingleton();
        if (!settings->bEnablePlayerAutoCharge) return;

        auto weapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(player);

        if (weapons.empty()) {
            logger::info("[PlayerCheck] No enchanted weapons detected on player.");
        } else {
            for (const auto& weaponInfo : weapons) {
                logger::info("[PlayerCheck] '{}' {} hand: {:.0f}/{:.0f} ({:.1f}%) | threshold: {:.0f}% | targetCap: {:.0f}%",
                    weaponInfo.weapon->GetName(),
                    weaponInfo.isLeftHand ? "Left" : "Right",
                    weaponInfo.currentCharge, weaponInfo.maxCharge, weaponInfo.chargePercent,
                    settings->fChargeThreshold, settings->fAutoChargeTargetPercent);

                if (weaponInfo.chargePercent <= settings->fChargeThreshold) {
                    float targetCapPercent = std::clamp(settings->fAutoChargeTargetPercent, 1.0f, 100.0f);
                    float targetMaxCharge = weaponInfo.maxCharge * (targetCapPercent / 100.0f);

                    EnchantedWeaponInfo current = weaponInfo;
                    while (current.currentCharge < targetMaxCharge) {
                        float missing = targetMaxCharge - current.currentCharge;
                        auto bestGem = SoulGemManager::GetSingleton()->FindBestSoulGemForCharge(player, missing);
                        if (!bestGem) {
                            logger::info("[PlayerCheck] No suitable soul gem found for '{}'", current.weapon->GetName());
                            break;
                        }

                        float previousCharge = current.currentCharge;
                        bool ok = SoulGemManager::GetSingleton()->ChargeWeaponWithSoulGem(player, current, *bestGem);
                        if (!ok) break;

                        // Şarj değerini yerel değişkende birikimli arttır (engine gecikmesinden etkilenmemek için)
                        current.currentCharge = std::min(previousCharge + static_cast<float>(bestGem->chargeValue), current.maxCharge);
                        current.chargePercent = (current.currentCharge / current.maxCharge) * 100.0f;
                    }
                }
            }
        }
    }
}
