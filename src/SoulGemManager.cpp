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

    // Azura's Star / The Black Star / Varla Stones: reusable, self-refilling gems.
    // Matched by stable vanilla FormID or by name (Varla stones come from CC plugins
    // with load-order dependent FormIDs).
    static bool IsReusableSoulGem(RE::TESSoulGem* a_gem)
    {
        if (!a_gem) return false;

        switch (a_gem->GetFormID()) {
        case 0x00063B27: // Azura's Star (AzurasStar)
        case 0x00063B29: // The Black Star (AzurasStarBlack)
            return true;
        default:
            break;
        }

        std::string_view edid = a_gem->GetFormEditorID() ? a_gem->GetFormEditorID() : "";
        if (edid.find("Varla") != std::string_view::npos) return true;

        if (const char* name = a_gem->GetName()) {
            std::string_view n(name);
            if (n.find("Varla") != std::string_view::npos ||
                n.find("Azura") != std::string_view::npos ||
                n.find("Black Star") != std::string_view::npos) {
                return true;
            }
        }
        return false;
    }

    uint32_t SoulGemManager::GetSoulChargeValue(RE::SOUL_LEVEL a_level)
    {
        auto settings = Settings::GetSingleton();
        switch (a_level) {
        case RE::SOUL_LEVEL::kPetty:   return settings->uChargePetty;
        case RE::SOUL_LEVEL::kLesser:  return settings->uChargeLesser;
        case RE::SOUL_LEVEL::kCommon:  return settings->uChargeCommon;
        case RE::SOUL_LEVEL::kGreater: return settings->uChargeGreater;
        case RE::SOUL_LEVEL::kGrand:   return settings->uChargeGrand;
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

                // Reusable gems (Azura's Star / Black Star / Varla) bypass tier
                // restrictions — the player deliberately carries them for recharging.
                bool isReusable = IsReusableSoulGem(soulGem);

                if (!isReusable) {
                    if (soulContained == RE::SOUL_LEVEL::kGrand && !settings->bAllowGrandSoulGems) {
                        continue;
                    }

                    if (isBlack && !settings->bAllowBlackSoulGems) {
                        continue;
                    }
                }

                AvailableSoulGem entry;
                entry.soulGem = soulGem;
                entry.soulLevel = soulContained;
                entry.chargeValue = GetSoulChargeValue(soulContained);
                entry.count = invData.first;
                entry.isBlack = isBlack;
                entry.isReusable = isReusable;

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

        // Reusable gems (Azura's Star / Black Star / Varla) always take priority:
        // they are never consumed, so spending their trapped soul costs nothing permanent.
        {
            const AvailableSoulGem* fallback = nullptr;
            for (const auto& gem : gems) {
                if (!gem.isReusable) continue;
                if (static_cast<float>(gem.chargeValue) >= a_missingCharge) {
                    return gem; // smallest reusable that fully covers the missing charge
                }
                if (!fallback) fallback = &gem;
            }
            if (fallback) return *fallback; // partial fill; loop may continue with other gems
        }

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
            if (a_gem.isReusable) {
                // Reusable gem: keep the item, only release the trapped soul so it can
                // be refilled by Soul Trap again.
                bool soulCleared = false;
                auto* invChanges = a_actor->GetInventoryChanges(false);
                if (invChanges && invChanges->entryList) {
                    for (auto* entry : *invChanges->entryList) {
                        if (!entry || entry->object != a_gem.soulGem || !entry->extraLists) continue;
                        for (auto* xList : *entry->extraLists) {
                            if (!xList) continue;
                            if (auto* xSoul = xList->GetByType<RE::ExtraSoul>()) {
                                xList->Remove(xSoul);
                                soulCleared = true;
                            }
                        }
                    }
                }
                if (soulCleared) {
                    a_actor->AddChange(RE::TESObjectREFR::ChangeFlags::kItemExtraData);
                }

                logger::info("Recharged {} for actor {} using reusable {} (charge: +{}, soul released)",
                    a_weapon.weapon->GetName(),
                    a_actor->GetDisplayFullName() ? a_actor->GetDisplayFullName() : "Actor",
                    a_gem.soulGem->GetName(),
                    a_gem.chargeValue
                );
            } else {
                // Remove 1 soul gem from inventory
                a_actor->RemoveItem(a_gem.soulGem, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);

                logger::info("Recharged {} for actor {} using {} (charge: +{})",
                    a_weapon.weapon->GetName(),
                    a_actor->GetDisplayFullName() ? a_actor->GetDisplayFullName() : "Actor",
                    a_gem.soulGem->GetName(),
                    a_gem.chargeValue
                );
            }

            NotificationManager::GetSingleton()->NotifyWeaponCharged(a_actor, a_weapon.weapon->GetName());
            return true;
        }

        logger::warn("[ChargeWeaponWithSoulGem] Charge failed — weapon not updated.");
        return false;
    }
}
