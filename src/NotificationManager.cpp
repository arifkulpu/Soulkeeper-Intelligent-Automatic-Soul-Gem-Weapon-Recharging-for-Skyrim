#include "NotificationManager.h"
#include "Settings.h"

namespace Soulkeeper
{
    NotificationManager* NotificationManager::GetSingleton()
    {
        static NotificationManager singleton;
        return std::addressof(singleton);
    }

    void NotificationManager::Notify(std::string_view a_message)
    {
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableNotifications) {
            logger::info("[Notify] Notifications disabled, skipping: {}", a_message);
            return;
        }

        if (a_message.empty()) {
            logger::warn("[Notify] Empty message, skipping");
            return;
        }

        // RE::DebugNotification expects a null-terminated C-string
        std::string msg(a_message);
        logger::info("[Notify] Sending notification: {}", msg);
        RE::DebugNotification(msg.c_str());
    }

    bool NotificationManager::CanNotifyFollower(RE::FormID a_formID)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        
        float currentHours = 0.0f;
        if (auto* calendar = RE::Calendar::GetSingleton()) {
            currentHours = calendar->GetHoursPassed();
        }

        auto it = _cooldowns.find(a_formID);
        if (it != _cooldowns.end()) {
            auto settings = Settings::GetSingleton();
            // Skyrim calendar: 1 real hour = 20 in-game hours (timescale 20).
            // 3 real minutes = 0.05 real hours = 1.0 game hour.
            // Formula: gameHoursElapsed < (cooldownInSeconds / 180.0f)
            float hoursElapsed = currentHours - it->second;
            if (hoursElapsed < (settings->fNotificationCooldown / 180.0f)) {
                return false;
            }
        }
        return true;
    }

    void NotificationManager::UpdateCooldown(RE::FormID a_formID)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (auto* calendar = RE::Calendar::GetSingleton()) {
            _cooldowns[a_formID] = calendar->GetHoursPassed();
        }
    }

    void NotificationManager::NotifyFollowerNoSoulGems(RE::Actor* a_follower)
    {
        if (!a_follower) return;
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableNotifications || !settings->bNotifyNoSoulGems) return;

        if (!CanNotifyFollower(a_follower->GetFormID())) {
            return;
        }

        const char* name = a_follower->GetDisplayFullName();
        std::string msg = std::format("{} has no Soul Gems to charge weapon.", name ? name : "Follower");
        Notify(msg);
        UpdateCooldown(a_follower->GetFormID());
    }

    void NotificationManager::NotifyWeaponCharged(RE::Actor* a_actor, std::string_view a_weaponName)
    {
        if (!a_actor) return;
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableNotifications) return;

        if (a_actor->IsPlayerRef()) {
            if (!settings->bNotifyPlayerCharge) return;
            std::string msg = a_weaponName.empty() ?
                "Your weapon has been automatically charged." :
                std::format("Your {} has been automatically charged.", a_weaponName);
            Notify(msg);
        } else {
            if (!settings->bNotifyFollowerCharge) return;
            const char* name = a_actor->GetDisplayFullName();
            std::string msg = a_weaponName.empty() ?
                std::format("{} has automatically charged their weapon.", name ? name : "Follower") :
                std::format("{} has charged {}.", name ? name : "Follower", a_weaponName);
            Notify(msg);
        }
    }

    void NotificationManager::NotifyPassiveRecharge(float a_percent)
    {
        auto settings = Settings::GetSingleton();
        if (!settings->bEnableNotifications || !settings->bNotifyPassiveRecharge) return;

        std::string msg = std::format("Passive recharge restored {:.0f}% weapon charge.", a_percent);
        Notify(msg);
    }

    void NotificationManager::ClearCooldowns()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _cooldowns.clear();
    }
}
