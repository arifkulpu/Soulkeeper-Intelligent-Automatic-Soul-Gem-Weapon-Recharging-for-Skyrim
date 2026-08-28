#pragma once

#include "PCH.h"
#include "WeaponManager.h"

namespace Soulkeeper
{
    class PurchaseManager
    {
    public:
        static PurchaseManager* GetSingleton();

        // Attempts to purchase a soul gem using follower's own inventory gold
        bool TryFollowerPurchaseSoulGem(RE::Actor* a_follower, const EnchantedWeaponInfo& a_weapon);

        // Calculates gold cost for a given soul level
        uint32_t CalculateSoulGemPrice(RE::SOUL_LEVEL a_level);

        // Finds gold in follower inventory
        int32_t GetFollowerGold(RE::Actor* a_follower);

        // Checks if follower is in a town/settlement/interior and has a soul gem merchant nearby
        bool CanPurchaseAtCurrentLocation(RE::Actor* a_follower);

        // Restocks soul gems up to configured inventory limits when in town near a vendor
        void TryFollowerRestockStockGems(RE::Actor* a_follower);

    private:
        PurchaseManager() = default;
        ~PurchaseManager() = default;

        RE::TESSoulGem* GetStandardSoulGemForm(RE::SOUL_LEVEL a_level);
        bool IsSoulGemVendor(RE::Actor* a_actor);
    };
}
