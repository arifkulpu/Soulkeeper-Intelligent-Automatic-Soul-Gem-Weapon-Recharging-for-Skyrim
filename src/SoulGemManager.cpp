#include "SoulGemManager.h"
#include "Settings.h"
#include "NotificationManager.h"

namespace Soulkeeper
{
    SoulGemManager* SoulGemManager::GetSingleton()
    {
        static SoulGemManager singleton;
        return std::addressof(singleton);
    }

    uint32_t SoulGemManager::GetSoulChargeValue(RE::SOUL_LEVEL a_level)
    {
        switch (a_level) {
        case RE::SOUL_LEVEL::kPetty:   return 250;
        case RE::SOUL_LEVEL::kLesser:  return 500;
        case RE::SOUL_LEVEL::kCommon:  return 1000;
        case RE::SOUL_LEVEL::kGreater: return 2000;
        case RE::SOUL_LEVEL::kGrand:   return 3000;
        default:                       return 0;
        }
    }

    std::vector<AvailableSoulGem> SoulGemManager::GetAvailableSoulGems(RE::Actor* a_actor)
    {
        std::vector<AvailableSoulGem> gems;
        if (!a_actor) return gems;

        auto inv = a_actor->GetInventory();
        auto settings = Settings::GetSingleton();

        for (const auto& [boundObj, invData] : inv) {
            if (!boundObj || invData.first <= 0) continue;

            if (auto* soulGem = boundObj->As<RE::TESSoulGem>()) {
                // Determine soul level from base form or extra list
                RE::SOUL_LEVEL soulContained = soulGem->GetContainedSoul();

                // If base form is empty, check extra lists for captured soul
                if (soulContained == RE::SOUL_LEVEL::kNone && invData.second) {
                    if (invData.second->extraLists) {
                        for (auto* xList : *invData.second->extraLists) {
                            if (xList) {
                                if (auto* xSoul = xList->GetByType<RE::ExtraSoul>()) {
                                    soulContained = static_cast<RE::SOUL_LEVEL>(*xSoul->soul);
                                    break;
                                }
                            }
                        }
                    }
                }

                if (soulContained == RE::SOUL_LEVEL::kNone) {
                    continue; // Skip empty soul gems
                }

                bool isBlack = soulGem->CanHoldNPCSoul() ||
                               (soulGem->GetFormEditorID() && std::string_view(soulGem->GetFormEditorID()).find("Black") != std::string_view::npos);

                if (soulContained == RE::SOUL_LEVEL::kGrand && !settings->bAllowGrandSoulGems) {
                    continue;
                }

                if (isBlack && !settings->bAllowBlackSoulGems) {
                    continue;
                }

                AvailableSoulGem entry;
                entry.soulGem = soulGem;
                entry.soulLevel = soulContained;
                entry.chargeValue = GetSoulChargeValue(soulContained);
                entry.count = invData.first;
                entry.isBlack = isBlack;

                gems.push_back(entry);
            }
        }

        // Sort ascending by charge value (Smallest first: Petty -> Lesser -> Common -> Greater -> Grand)
        std::sort(gems.begin(), gems.end(), [](const AvailableSoulGem& a, const AvailableSoulGem& b) {
            return a.chargeValue < b.chargeValue;
        });

        return gems;
    }

    std::optional<AvailableSoulGem> SoulGemManager::FindBestSoulGemForCharge(RE::Actor* a_actor, float a_missingCharge)
    {
        auto gems = GetAvailableSoulGems(a_actor);
        if (gems.empty()) return std::nullopt;

        auto settings = Settings::GetSingleton();

        if (settings->bPreferSmallestSoulGem) {
            // Find smallest gem that can satisfy or closest to missing charge
            for (const auto& gem : gems) {
                if (static_cast<float>(gem.chargeValue) >= a_missingCharge) {
                    return gem;
                }
            }
            // If none single gem satisfies completely, use largest available or smallest
            return gems.back(); // Largest available to maximize recovery
        } else {
            // Default to largest available
            return gems.back();
        }
    }

    bool SoulGemManager::ChargeWeaponWithSoulGem(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weapon, const AvailableSoulGem& a_gem)
    {
        if (!a_actor || !a_weapon.IsValid() || !a_gem.soulGem) return false;

        logger::info("[ChargeWeaponWithSoulGem] Actor: '{}' | Weapon: '{}' | Gem: '{}' (chargeValue: {})",
            a_actor->GetDisplayFullName() ? a_actor->GetDisplayFullName() : "Actor",
            a_weapon.weapon->GetName(),
            a_gem.soulGem->GetName(),
            a_gem.chargeValue);

        // Add charge
        bool charged = WeaponManager::GetSingleton()->AddWeaponCharge(a_actor, a_weapon, static_cast<float>(a_gem.chargeValue));
        logger::info("[ChargeWeaponWithSoulGem] AddWeaponCharge returned: {}", charged);

        if (charged) {
            // Remove 1 soul gem from inventory
            a_actor->RemoveItem(a_gem.soulGem, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

            logger::info("Recharged {} for actor {} using {} (charge: +{})",
                a_weapon.weapon->GetName(),
                a_actor->GetDisplayFullName() ? a_actor->GetDisplayFullName() : "Actor",
                a_gem.soulGem->GetName(),
                a_gem.chargeValue
            );

            NotificationManager::GetSingleton()->NotifyWeaponCharged(a_actor, a_weapon.weapon->GetName());
            return true;
        }

        logger::warn("[ChargeWeaponWithSoulGem] Charge failed — weapon not updated.");
        return false;
    }
}
