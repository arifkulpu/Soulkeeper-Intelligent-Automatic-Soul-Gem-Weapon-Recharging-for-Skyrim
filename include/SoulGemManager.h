#pragma once

#include "PCH.h"
#include "WeaponManager.h"

namespace Soulkeeper
{
    struct AvailableSoulGem
    {
        RE::TESSoulGem* soulGem{ nullptr };
        RE::SOUL_LEVEL soulLevel{ RE::SOUL_LEVEL::kNone };
        uint32_t chargeValue{ 0 };
        int32_t count{ 0 };
        bool isBlack{ false };
    };

    class SoulGemManager
    {
    public:
        static SoulGemManager* GetSingleton();

        // Gets the charge value granted by a given soul level (Vanilla Skyrim values)
        static uint32_t GetSoulChargeValue(RE::SOUL_LEVEL a_level);

        // Finds all filled soul gems in actor inventory
        std::vector<AvailableSoulGem> GetAvailableSoulGems(RE::Actor* a_actor);

        // Selects the smartest/smallest adequate soul gem for the weapon's missing charge
        std::optional<AvailableSoulGem> FindBestSoulGemForCharge(RE::Actor* a_actor, float a_missingCharge);

        // Charges weapon and consumes soul gem from actor's inventory
        bool ChargeWeaponWithSoulGem(RE::Actor* a_actor, const EnchantedWeaponInfo& a_weapon, const AvailableSoulGem& a_gem);

    private:
        SoulGemManager() = default;
        ~SoulGemManager() = default;
    };
}
