#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class Settings
    {
    public:
        static Settings* GetSingleton();

        void Load();
        void Save();

        // General
        bool bEnableMod{ true };
        bool bEnablePlayerAutoCharge{ true };
        bool bEnableFollowerAutoCharge{ true };
        bool bEnablePassiveRecharge{ true };

        // Threshold & Timers
        float fChargeThreshold{ 20.0f };             // Percentage (0 - 100) below which auto-charge triggers
        float fAutoChargeTargetPercent{ 50.0f };      // Max percentage (0 - 100) to fill weapons with soul gems
        float fCheckIntervalSeconds{ 30.0f };        // Check cadence
        float fPassiveRechargeInterval{ 60.0f };     // Seconds between passive ticks
        float fPassiveRechargePercent{ 1.0f };       // Percent restored per tick
        float fPassiveRechargeCapPercent{ 20.0f };   // Max percentage (0 - 100) passive recharge can fill weapons
        bool bRechargeInCombat{ false };
        float fCombatRechargeInterval{ 180.0f };     // Slower recharge interval during combat

        // Soul Gem Selection
        bool bPreferSmallestSoulGem{ true };
        bool bAllowGrandSoulGems{ true };
        bool bAllowBlackSoulGems{ false };        // Protected by default to not waste black souls

        // Follower Purchase & Stocking
        bool bEnableFollowerPurchase{ true };
        bool bEnableFollowerStocking{ true };        // Auto restock soul gems in town
        bool bUseFollowerGoldOnly{ true };
        uint32_t uMaxPurchasePrice{ 500 };
        uint32_t uMinGoldRemaining{ 50 };

        // Desired stock quantities per tier
        uint32_t uStockPettyCount{ 5 };              // Desired count of Petty Soul Gems in follower inventory
        uint32_t uStockLesserCount{ 3 };             // Desired count of Lesser Soul Gems
        uint32_t uStockCommonCount{ 2 };             // Desired count of Common Soul Gems
        uint32_t uStockGreaterCount{ 0 };            // Desired count of Greater Soul Gems
        uint32_t uStockGrandCount{ 0 };              // Desired count of Grand Soul Gems

        // Notifications
        bool bEnableNotifications{ true };
        bool bNotifyPlayerCharge{ true };
        bool bNotifyFollowerCharge{ true };
        bool bNotifyNoSoulGems{ true };
        bool bNotifyFollowerPurchase{ true };
        bool bNotifyFailedPurchase{ true };
        bool bNotifyPassiveRecharge{ false };
        float fNotificationCooldown{ 180.0f };     // 3 minutes reminder cooldown per follower

    private:
        Settings() = default;
        ~Settings() = default;
        Settings(const Settings&) = delete;
        Settings& operator=(const Settings&) = delete;
    };
}
