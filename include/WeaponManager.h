#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    struct EnchantedWeaponInfo
    {
        RE::TESObjectWEAP* weapon{ nullptr };
        RE::InventoryEntryData* entryData{ nullptr };
        RE::EnchantmentItem* enchantment{ nullptr };
        bool isLeftHand{ false };
        float currentCharge{ 0.0f };
        float maxCharge{ 0.0f };
        float chargePercent{ 100.0f };

        [[nodiscard]] bool IsValid() const {
            return weapon != nullptr && maxCharge > 0.0f;
        }

        [[nodiscard]] float GetMissingCharge() const {
            return std::max(0.0f, maxCharge - currentCharge);
        }
    };

    class WeaponManager
    {
    public:
        static WeaponManager* GetSingleton();

        // Retrieves enchanted equipped weapons for any actor (Right Hand & Left Hand)
        std::vector<EnchantedWeaponInfo> GetEquippedEnchantedWeapons(RE::Actor* a_actor);

        // Sets weapon charge in inventory extra lists and notifies game engine
        bool SetWeaponCharge(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weaponInfo, float a_newCharge);

        // Adds charge to weapon up to maxCharge
        bool AddWeaponCharge(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weaponInfo, float a_chargeAmount);

        // Re-applies cached charge values for player and followers (called periodically)
        void RestoreCachedCharges(RE::Actor* a_actor);

        // Clear cache entry when an actor drops/unequips a weapon
        void ClearCache(RE::Actor* a_actor);

    private:
        WeaponManager() = default;
        ~WeaponManager() = default;

        std::optional<EnchantedWeaponInfo> InspectEquippedSlot(RE::Actor* a_actor, bool a_leftHand);

        // Cache key: actor FormID (32-bit) packed with hand flag in bit 32
        static uint64_t MakeCacheKey(RE::Actor* a_actor, bool a_isLeftHand) {
            return (static_cast<uint64_t>(a_actor->GetFormID()) << 1) | (a_isLeftHand ? 1 : 0);
        }

        // Stores desired charge values per actor+hand slot
        // value = {weaponFormID, desiredCharge, maxCharge}
        struct CachedCharge {
            RE::FormID weaponFormID{ 0 };
            float desiredCharge{ 0.0f };
            float maxCharge{ 0.0f };
        };

        std::unordered_map<uint64_t, CachedCharge> _chargeCache;
        std::mutex _cacheMutex;
    };
}
