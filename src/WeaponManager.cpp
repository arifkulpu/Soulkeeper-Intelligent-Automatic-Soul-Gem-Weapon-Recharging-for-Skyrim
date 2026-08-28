#include "WeaponManager.h"

namespace Soulkeeper
{
    WeaponManager* WeaponManager::GetSingleton()
    {
        static WeaponManager singleton;
        return std::addressof(singleton);
    }

    // -------------------------------------------------------------------------
    // Public: GetEquippedEnchantedWeapons
    // -------------------------------------------------------------------------
    std::vector<EnchantedWeaponInfo> WeaponManager::GetEquippedEnchantedWeapons(RE::Actor* a_actor)
    {
        std::vector<EnchantedWeaponInfo> result;
        if (!a_actor) return result;

        auto isTwoHandedWeapon = [](RE::WEAPON_TYPE wt) -> bool {
            return wt == RE::WEAPON_TYPE::kBow
                || wt == RE::WEAPON_TYPE::kCrossbow
                || wt == RE::WEAPON_TYPE::kTwoHandSword
                || wt == RE::WEAPON_TYPE::kTwoHandAxe;
        };

        // Right Hand (Main)
        auto right = InspectEquippedSlot(a_actor, false);
        if (right) {
            result.push_back(*right);
        }

        // Skip left hand if right hand holds a two-handed / bow weapon
        bool skipLeftHand = false;
        if (right) {
            skipLeftHand = isTwoHandedWeapon(right->weapon->GetWeaponType());
        } else {
            auto obj = a_actor->GetEquippedObject(false);
            if (obj && obj->IsWeapon()) {
                auto* w = obj->As<RE::TESObjectWEAP>();
                if (w) skipLeftHand = isTwoHandedWeapon(w->GetWeaponType());
            }
        }

        if (!skipLeftHand) {
            if (auto left = InspectEquippedSlot(a_actor, true)) {
                result.push_back(*left);
            }
        }

        return result;
    }

    // -------------------------------------------------------------------------
    // Private: InspectEquippedSlot
    // Now reads from the charge cache when no ExtraCharge is found in memory,
    // and immediately tries to re-apply the cached value.
    // -------------------------------------------------------------------------
    std::optional<EnchantedWeaponInfo> WeaponManager::InspectEquippedSlot(RE::Actor* a_actor, bool a_leftHand)
    {
        if (!a_actor) return std::nullopt;

        RE::TESObjectWEAP* weapon = nullptr;
        RE::InventoryEntryData* matchingEntry = a_actor->GetEquippedEntryData(a_leftHand);

        if (matchingEntry && matchingEntry->object && matchingEntry->object->IsWeapon()) {
            weapon = matchingEntry->object->As<RE::TESObjectWEAP>();
        }

        if (!weapon) {
            auto equippedObj = a_actor->GetEquippedObject(a_leftHand);
            if (equippedObj && equippedObj->IsWeapon()) {
                weapon = equippedObj->As<RE::TESObjectWEAP>();
            }
        }

        if (!weapon) return std::nullopt;

        RE::EnchantmentItem* enchantment = weapon->formEnchanting;
        float maxCharge = static_cast<float>(weapon->amountofEnchantment);
        float currentCharge = -1.0f;
        bool foundExtraCharge = false;

        // 1. Prioritize InventoryChanges worn entry for absolute accurate live data
        auto* invChanges = a_actor->GetInventoryChanges(true);
        if (invChanges && invChanges->entryList) {
            for (auto* entry : *invChanges->entryList) {
                if (entry && entry->object == weapon && entry->extraLists) {
                    for (auto* xList : *entry->extraLists) {
                        if (!xList) continue;
                        bool isWorn = a_leftHand ?
                            xList->HasType(RE::ExtraDataType::kWornLeft) :
                            xList->HasType(RE::ExtraDataType::kWorn);
                        if (isWorn) {
                            matchingEntry = entry;
                            if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                                if (xEnch->enchantment) {
                                    enchantment = xEnch->enchantment;
                                }
                                if (xEnch->charge > 0) {
                                    maxCharge = static_cast<float>(xEnch->charge);
                                }
                            }
                            if (auto* xCharge = xList->GetByType<RE::ExtraCharge>()) {
                                if (xCharge->charge > 0.0f) {
                                    currentCharge = xCharge->charge;
                                    foundExtraCharge = true;
                                }
                            }
                            break;
                        }
                    }
                    if (matchingEntry) break;
                }
            }
        }

        // 2. ExtraEnchantment and ExtraCharge from equippedEntry if not found yet
        if (matchingEntry && matchingEntry->extraLists && !foundExtraCharge) {
            for (auto* xList : *matchingEntry->extraLists) {
                if (!xList) continue;
                if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                    if (xEnch->enchantment) {
                        enchantment = xEnch->enchantment;
                    }
                    if (xEnch->charge > 0) {
                        maxCharge = static_cast<float>(xEnch->charge);
                    }
                }
                if (auto* xCharge = xList->GetByType<RE::ExtraCharge>()) {
                    if (xCharge->charge > 0.0f) {
                        currentCharge = xCharge->charge;
                        foundExtraCharge = true;
                    }
                }
            }
        }

        // If maxCharge wasn't explicitly set in weapon or ExtraEnchantment, calculate reasonable default
        if (maxCharge <= 0.0f) {
            if (weapon->amountofEnchantment > 0) {
                maxCharge = static_cast<float>(weapon->amountofEnchantment);
            } else if (enchantment && enchantment->data.chargeOverride > 0) {
                maxCharge = static_cast<float>(enchantment->data.chargeOverride);
            } else {
                maxCharge = 2000.0f; // Standard default for enchanted items without explicit capacity
            }
        }

        if (!enchantment || maxCharge <= 0.0f) {
            return std::nullopt;
        }

        // 3. Native InventoryEntryData charge calculation (Live active caster charge)
        if (matchingEntry && !foundExtraCharge) {
            if (auto nativeCharge = matchingEntry->GetEnchantmentCharge()) {
                float val = static_cast<float>(*nativeCharge);
                // Note: If GetEnchantmentCharge returns a percentage (e.g. 100.0) or <= 100 on a 1500-charge weapon:
                if (val > 100.0f) {
                    currentCharge = val;
                    foundExtraCharge = true;
                } else if (val >= 99.0f) {
                    // 100% full
                    currentCharge = maxCharge;
                    foundExtraCharge = true;
                }
            }
        }

        // If no ExtraCharge was found in memory, or it was 100% full fresh weapon
        if (!foundExtraCharge || currentCharge < 0.0f) {
            currentCharge = maxCharge;
        }

        EnchantedWeaponInfo info;
        info.weapon = weapon;
        info.entryData = matchingEntry;
        info.enchantment = enchantment;
        info.isLeftHand = a_leftHand;
        info.currentCharge = std::clamp(currentCharge, 0.0f, maxCharge);
        info.maxCharge = maxCharge;
        info.chargePercent = (info.currentCharge / maxCharge) * 100.0f;

        return info;
    }

    // -------------------------------------------------------------------------
    // Public: SetWeaponCharge
    // -------------------------------------------------------------------------
    bool WeaponManager::SetWeaponCharge(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weaponInfo, float a_newCharge)
    {
        if (!a_actor || !a_weaponInfo.IsValid()) return false;

        float clampedCharge = std::clamp(a_newCharge, 0.0f, a_weaponInfo.maxCharge);
        bool updated = false;

        RE::ExtraDataList* primaryXList = nullptr;

        auto applyToExtraList = [&](RE::ExtraDataList* xList) -> bool {
            if (!xList) return false;

            float targetCharge = clampedCharge;
            auto* xCharge = xList->GetByType<RE::ExtraCharge>();

            if (xCharge) {
                xCharge->charge = targetCharge;
                if (!primaryXList) primaryXList = xList;
                return true;
            } else {
                auto* newCharge = RE::BSExtraData::Create<RE::ExtraCharge>();
                if (newCharge) {
                    newCharge->charge = targetCharge;
                    xList->Add(newCharge);
                    if (!primaryXList) primaryXList = xList;
                    return true;
                }
            }
            return false;
        };

        // 1. GetEquippedEntryData fast-path (Direct equipped weapon reference for HUD and active gameplay)
        if (auto* equippedEntry = a_actor->GetEquippedEntryData(a_weaponInfo.isLeftHand)) {
            if (equippedEntry->object == a_weaponInfo.weapon && equippedEntry->extraLists) {
                for (auto* xList : *equippedEntry->extraLists) {
                    if (!xList) continue;
                    if (applyToExtraList(xList)) updated = true;
                }
            }
        }

        // 2. InventoryChanges traversal — the persistent container source of truth
        auto* invChanges = a_actor->GetInventoryChanges(true);
        if (invChanges && invChanges->entryList) {
            for (auto* entry : *invChanges->entryList) {
                if (entry && entry->object == a_weaponInfo.weapon) {
                    bool hasWorn = false;
                    if (entry->extraLists) {
                        for (auto* xList : *entry->extraLists) {
                            if (!xList) continue;
                            bool isWorn = a_weaponInfo.isLeftHand ?
                                xList->HasType(RE::ExtraDataType::kWornLeft) :
                                xList->HasType(RE::ExtraDataType::kWorn);
                            if (isWorn) {
                                hasWorn = true;
                                if (applyToExtraList(xList)) updated = true;
                            }
                        }
                    }
                    if (!hasWorn && entry->extraLists) {
                        for (auto* xList : *entry->extraLists) {
                            if (xList && applyToExtraList(xList)) {
                                updated = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // 3. Fallback entryData snapshot
        if (a_weaponInfo.entryData && a_weaponInfo.entryData->extraLists) {
            for (auto* xList : *a_weaponInfo.entryData->extraLists) {
                if (!xList) continue;
                if (applyToExtraList(xList)) updated = true;
            }
        }

        if (updated) {
            logger::info("[SetWeaponCharge] '{}' (0x{:08X}) -> {:.0f}/{:.0f} ({:.1f}%)",
                a_weaponInfo.weapon->GetName(),
                a_weaponInfo.weapon->GetFormID(),
                clampedCharge, a_weaponInfo.maxCharge,
                (clampedCharge / a_weaponInfo.maxCharge) * 100.0f);

            // 1. Mark reference inventory dirty so SkyUI and UI engine refresh UI lists
            a_actor->AddChange(RE::TESObjectREFR::ChangeFlags::kInventory | RE::TESObjectREFR::ChangeFlags::kItemExtraData);

            // 2. Notify container changes
            if (invChanges) {
                invChanges->changed = true;
                invChanges->SendContainerChangedEvent(primaryXList, a_actor, a_weaponInfo.weapon, 0);
            }

            // 3. Update the actor's active weapon enchant ability directly (SKSE Native Method)
            // This synchronizes Skyrim's internal MagicCaster and HUD bars without unequipping!
            a_actor->UpdateWeaponAbility(a_weaponInfo.weapon, primaryXList, a_weaponInfo.isLeftHand);

            // 4. Update charge cache with timestamp
            {
                const uint64_t cacheKey = MakeCacheKey(a_actor, a_weaponInfo.isLeftHand);
                std::lock_guard lock(_cacheMutex);
                _chargeCache[cacheKey] = { a_weaponInfo.weapon->GetFormID(), clampedCharge, a_weaponInfo.maxCharge };
            }
        } else {
            logger::warn("[SetWeaponCharge] FAILED to update charge for '{}' (0x{:08X}, isLeft={})",
                a_weaponInfo.weapon->GetName(),
                a_weaponInfo.weapon->GetFormID(),
                a_weaponInfo.isLeftHand);
        }

        return updated;
    }

    // -------------------------------------------------------------------------
    // Public: AddWeaponCharge
    // -------------------------------------------------------------------------
    bool WeaponManager::AddWeaponCharge(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weaponInfo, float a_chargeAmount)
    {
        if (a_chargeAmount <= 0.0f) return false;
        float target = a_weaponInfo.currentCharge + a_chargeAmount;
        return SetWeaponCharge(a_actor, a_weaponInfo, target);
    }

    // -------------------------------------------------------------------------
    // Public: RestoreCachedCharges
    // Called periodically / on menu close to re-assert cached charges if the
    // game engine (or another mod) wiped the ExtraCharge entries.
    // -------------------------------------------------------------------------
    void WeaponManager::RestoreCachedCharges(RE::Actor* a_actor)
    {
        if (!a_actor) return;
        auto weapons = GetEquippedEnchantedWeapons(a_actor);
        for (const auto& wpn : weapons) {
            if (wpn.IsValid()) {
                SetWeaponCharge(a_actor, wpn, wpn.currentCharge);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Public: ClearCache
    // -------------------------------------------------------------------------
    void WeaponManager::ClearCache(RE::Actor* a_actor)
    {
        if (!a_actor) return;
        std::lock_guard lock(_cacheMutex);
        _chargeCache.erase(MakeCacheKey(a_actor, false));
        _chargeCache.erase(MakeCacheKey(a_actor, true));
    }
}
