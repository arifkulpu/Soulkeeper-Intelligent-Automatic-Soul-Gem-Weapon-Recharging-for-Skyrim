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
    std::optional<EnchantedWeaponInfo> WeaponManager::InspectEquippedSlot(RE::Actor* a_actor, bool a_leftHand)
    {
        if (!a_actor) return std::nullopt;

        // --- Step 1: Resolve weapon ---
        // GetEquippedEntryData → AIProcess::MiddleHighProcessData::rightHand/leftHand
        // This is the LIVE equipped item entry updated by the game engine during combat.
        RE::InventoryEntryData* liveEntry = a_actor->GetEquippedEntryData(a_leftHand);

        RE::TESObjectWEAP* weapon = nullptr;
        if (liveEntry && liveEntry->object && liveEntry->object->IsWeapon()) {
            weapon = liveEntry->object->As<RE::TESObjectWEAP>();
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

        // --- Step 2: Read ExtraCharge from LIVE entry first ---
        // For both bows and melee, the engine writes charge deductions here in real-time.
        // For bows: updated on arrow fire via projectile system.
        // For melee: updated on hit via ActorMagicCaster enchantment handler.
        // IMPORTANT: We do NOT overwrite this with invChanges data (invChanges is lazily synced).
        if (liveEntry && liveEntry->extraLists) {
            for (auto* xList : *liveEntry->extraLists) {
                if (!xList) continue;
                if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                    if (xEnch->enchantment) enchantment = xEnch->enchantment;
                    if (xEnch->charge > 0) maxCharge = static_cast<float>(xEnch->charge);
                }
                if (!foundExtraCharge) {
                    if (auto* xCharge = xList->GetByType<RE::ExtraCharge>()) {
                        currentCharge = xCharge->charge;
                        foundExtraCharge = true;
                    }
                }
            }
        }

        // --- Step 3: InventoryChanges worn entry — enchantment info only ---
        // invChanges is authoritative for enchantment data (ExtraEnchantment),
        // but its ExtraCharge may be stale (only synced on inventory open for melee).
        // We use it for enchantment resolution, NOT as the primary charge source.
        RE::InventoryEntryData* invEntry = nullptr;
        auto* invChanges = a_actor->GetInventoryChanges(false);
        if (invChanges && invChanges->entryList) {
            for (auto* entry : *invChanges->entryList) {
                if (!entry || entry->object != weapon || !entry->extraLists) continue;
                for (auto* xList : *entry->extraLists) {
                    if (!xList) continue;
                    bool isWorn = a_leftHand ?
                        xList->HasType(RE::ExtraDataType::kWornLeft) :
                        xList->HasType(RE::ExtraDataType::kWorn);
                    if (isWorn) {
                        invEntry = entry;
                        // Update enchantment info from the worn xList
                        if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                            if (xEnch->enchantment) enchantment = xEnch->enchantment;
                            if (xEnch->charge > 0) maxCharge = static_cast<float>(xEnch->charge);
                        }
                        // Only read charge from invChanges if liveEntry had nothing
                        if (!foundExtraCharge) {
                            if (auto* xCharge = xList->GetByType<RE::ExtraCharge>()) {
                                currentCharge = xCharge->charge;
                                foundExtraCharge = true;
                            }
                        }
                        break;
                    }
                }
                if (invEntry) break;
            }
        }

        // --- Step 4: Fallback enchantment search in any extraList ---
        if (!foundExtraCharge) {
            RE::InventoryEntryData* fallbackEntry = invEntry ? invEntry : liveEntry;
            if (fallbackEntry && fallbackEntry->extraLists) {
                for (auto* xList : *fallbackEntry->extraLists) {
                    if (!xList) continue;
                    if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                        if (xEnch->enchantment) enchantment = xEnch->enchantment;
                        if (xEnch->charge > 0) maxCharge = static_cast<float>(xEnch->charge);
                    }
                    if (auto* xCharge = xList->GetByType<RE::ExtraCharge>()) {
                        currentCharge = xCharge->charge;
                        foundExtraCharge = true;
                    }
                }
            }
        }

        // --- Resolve maxCharge ---
        if (maxCharge <= 0.0f) {
            if (weapon->amountofEnchantment > 0) {
                maxCharge = static_cast<float>(weapon->amountofEnchantment);
            } else if (enchantment && enchantment->data.chargeOverride > 0) {
                maxCharge = static_cast<float>(enchantment->data.chargeOverride);
            } else {
                maxCharge = 2000.0f;
            }
        }

        if (!enchantment || maxCharge <= 0.0f) return std::nullopt;

        // --- Step 5: GetEnchantmentCharge() percentage fallback ---
        // This internally walks extraLists and computes charge/maxCharge.
        // Call on liveEntry first (more up-to-date), then invEntry.
        if (!foundExtraCharge) {
            auto tryPct = [&](RE::InventoryEntryData* e) {
                if (!e) return;
                if (auto nativePercent = e->GetEnchantmentCharge()) {
                    float pct = static_cast<float>(*nativePercent);
                    // GetEnchantmentCharge always returns 0.0-100.0 (percentage)
                    currentCharge = std::clamp(pct / 100.0f, 0.0f, 1.0f) * maxCharge;
                    foundExtraCharge = true;
                }
            };
            tryPct(liveEntry);
            if (!foundExtraCharge) tryPct(invEntry);
        }

        // --- Step 6: ActorMagicCaster live combat discharge ---
        // costCharged tracks the cost deducted per most-recent cast/swing.
        // Not cumulative, but useful as a lower-bound correction during rapid fire.
        auto casterSlot = a_leftHand ? RE::Actor::SlotTypes::kLeftHand : RE::Actor::SlotTypes::kRightHand;
        auto* caster = a_actor->GetActorRuntimeData().magicCasters[casterSlot];
        if (caster && caster->currentSpell == enchantment && caster->costCharged > 0.0f) {
            float liveCharge = maxCharge - caster->costCharged;
            if (!foundExtraCharge || liveCharge < currentCharge) {
                currentCharge = std::max(0.0f, liveCharge);
                foundExtraCharge = true;
            }
        }

        // --- Step 7: Charge cache fallback ---
        // Only used when no ExtraCharge was found in liveEntry or invChanges.
        // For bows/crossbows the cache is kept fresh by SetWeaponCharge called from
        // TESPlayerBowShotEvent (same proactive deduction approach as melee).
        if (!foundExtraCharge || currentCharge < 0.0f) {
            const uint64_t cacheKey = MakeCacheKey(a_actor, a_leftHand);
            std::lock_guard lock(_cacheMutex);
            auto it = _chargeCache.find(cacheKey);
            if (it != _chargeCache.end() && it->second.weaponFormID == weapon->GetFormID()) {
                currentCharge = it->second.desiredCharge;
                foundExtraCharge = true;
            }
        }

        // --- Step 8: Final default (pristine weapon) ---
        if (!foundExtraCharge || currentCharge < 0.0f) {
            currentCharge = maxCharge;
        }

        EnchantedWeaponInfo info;
        info.weapon = weapon;
        info.entryData = invEntry ? invEntry : liveEntry;
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

    // -------------------------------------------------------------------------
    // Public: SyncBowCacheFromInventory
    // Called immediately after TESPlayerBowShotEvent fires.
    // The engine has already written the post-shot ExtraCharge into InventoryChanges;
    // we read it here and store it in _chargeCache so InspectEquippedSlot's Step 7
    // can serve the correct value without waiting for an inventory open.
    // -------------------------------------------------------------------------
    void WeaponManager::SyncBowCacheFromInventory(RE::Actor* a_actor, RE::FormID a_weaponFormID)
    {
        if (!a_actor || a_weaponFormID == 0) return;

        auto* invChanges = a_actor->GetInventoryChanges(false);
        if (!invChanges || !invChanges->entryList) return;

        for (auto* entry : *invChanges->entryList) {
            if (!entry || !entry->object) continue;
            if (entry->object->GetFormID() != a_weaponFormID) continue;
            if (!entry->extraLists) continue;

            auto* weapon = entry->object->As<RE::TESObjectWEAP>();

            for (auto* xList : *entry->extraLists) {
                if (!xList) continue;

                // Bows always occupy the right-hand (kWorn) slot
                bool isWorn = xList->HasType(RE::ExtraDataType::kWorn) ||
                              xList->HasType(RE::ExtraDataType::kWornLeft);
                if (!isWorn) continue;

                auto* xCharge = xList->GetByType<RE::ExtraCharge>();
                if (!xCharge) continue;

                float liveCharge = xCharge->charge;

                // Resolve maxCharge from ExtraEnchantment or weapon base data
                float maxCharge = 0.0f;
                if (auto* xEnch = xList->GetByType<RE::ExtraEnchantment>()) {
                    if (xEnch->charge > 0) maxCharge = static_cast<float>(xEnch->charge);
                }
                if (maxCharge <= 0.0f && weapon && weapon->amountofEnchantment > 0) {
                    maxCharge = static_cast<float>(weapon->amountofEnchantment);
                }

                const uint64_t cacheKey = MakeCacheKey(a_actor, false); // bows = right hand
                {
                    std::lock_guard lock(_cacheMutex);
                    auto& cached = _chargeCache[cacheKey];
                    cached.weaponFormID = a_weaponFormID;
                    cached.desiredCharge = liveCharge;
                    if (maxCharge > 0.0f) cached.maxCharge = maxCharge;
                }

                logger::info("[SyncBowCache] '{}' charge synced: {:.0f}/{:.0f}",
                    weapon ? weapon->GetName() : "Bow",
                    liveCharge, maxCharge);
                return;
            }
        }

        logger::warn("[SyncBowCache] Could not find worn ExtraCharge for weapon 0x{:08X}", a_weaponFormID);
    }
}
