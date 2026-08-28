#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class NotificationManager
    {
    public:
        static NotificationManager* GetSingleton();

        void Notify(std::string_view a_message);
        void NotifyFollowerNoSoulGems(RE::Actor* a_follower);
        void NotifyFollowerCannotAfford(RE::Actor* a_follower);
        void NotifyFollowerPurchased(RE::Actor* a_follower, uint32_t a_goldSpent, std::string_view a_gemName);
        void NotifyWeaponCharged(RE::Actor* a_actor, std::string_view a_weaponName);
        void NotifyPassiveRecharge(float a_percent);

        void ClearCooldowns();

    private:
        NotificationManager() = default;
        ~NotificationManager() = default;

        bool CanNotifyFollower(RE::FormID a_formID);
        void UpdateCooldown(RE::FormID a_formID);

        std::unordered_map<RE::FormID, float> _cooldowns;
        std::mutex _mutex;
    };
}
