#include "Settings.h"
#include <SimpleIni.h>

namespace Soulkeeper
{
    Settings* Settings::GetSingleton()
    {
        static Settings singleton;
        return std::addressof(singleton);
    }

    void Settings::Load()
    {
        constexpr auto path = L"Data/SKSE/Plugins/Soulkeeper.ini";

        std::filesystem::path iniPath(path);
        std::error_code ec;
        if (iniPath.has_parent_path()) {
            std::filesystem::create_directories(iniPath.parent_path(), ec);
        }

        CSimpleIniA ini;
        ini.SetUnicode();

        SI_Error rc = ini.LoadFile(path);
        if (rc < 0) {
            logger::info("Soulkeeper.ini not found, creating default INI file at Data/SKSE/Plugins/Soulkeeper.ini.");
            Save();
            return;
        }

        // General
        bEnableMod = ini.GetBoolValue("General", "bEnableMod", bEnableMod);
        bEnablePlayerAutoCharge = ini.GetBoolValue("General", "bEnablePlayerAutoCharge", bEnablePlayerAutoCharge);
        bEnableFollowerAutoCharge = ini.GetBoolValue("General", "bEnableFollowerAutoCharge", bEnableFollowerAutoCharge);
        bEnablePassiveRecharge = ini.GetBoolValue("General", "bEnablePassiveRecharge", bEnablePassiveRecharge);

        // Threshold & Timers
        fChargeThreshold = static_cast<float>(ini.GetDoubleValue("ChargeThreshold", "fChargeThreshold", fChargeThreshold));
        fAutoChargeTargetPercent = static_cast<float>(ini.GetDoubleValue("ChargeThreshold", "fAutoChargeTargetPercent", fAutoChargeTargetPercent));
        fCheckIntervalSeconds = static_cast<float>(ini.GetDoubleValue("Timers", "fCheckIntervalSeconds", fCheckIntervalSeconds));
        fPassiveRechargeInterval = static_cast<float>(ini.GetDoubleValue("PassiveRecharge", "fPassiveRechargeInterval", fPassiveRechargeInterval));
        fPassiveRechargePercent = static_cast<float>(ini.GetDoubleValue("PassiveRecharge", "fPassiveRechargePercent", fPassiveRechargePercent));
        fPassiveRechargeCapPercent = static_cast<float>(ini.GetDoubleValue("PassiveRecharge", "fPassiveRechargeCapPercent", fPassiveRechargeCapPercent));
        bRechargeInCombat = ini.GetBoolValue("PassiveRecharge", "bRechargeInCombat", bRechargeInCombat);
        fCombatRechargeInterval = static_cast<float>(ini.GetDoubleValue("PassiveRecharge", "fCombatRechargeInterval", fCombatRechargeInterval));

        // Soul Gem Selection
        bPreferSmallestSoulGem = ini.GetBoolValue("SoulGems", "bPreferSmallestSoulGem", bPreferSmallestSoulGem);
        bAllowGrandSoulGems = ini.GetBoolValue("SoulGems", "bAllowGrandSoulGems", bAllowGrandSoulGems);
        bAllowBlackSoulGems = ini.GetBoolValue("SoulGems", "bAllowBlackSoulGems", bAllowBlackSoulGems);

        // Follower Purchase & Stocking
        bEnableFollowerPurchase = ini.GetBoolValue("FollowerPurchase", "bEnableFollowerPurchase", bEnableFollowerPurchase);
        bEnableFollowerStocking = ini.GetBoolValue("FollowerPurchase", "bEnableFollowerStocking", bEnableFollowerStocking);
        bUseFollowerGoldOnly = ini.GetBoolValue("FollowerPurchase", "bUseFollowerGoldOnly", bUseFollowerGoldOnly);
        uMaxPurchasePrice = static_cast<uint32_t>(ini.GetLongValue("FollowerPurchase", "uMaxPurchasePrice", uMaxPurchasePrice));
        uMinGoldRemaining = static_cast<uint32_t>(ini.GetLongValue("FollowerPurchase", "uMinGoldRemaining", uMinGoldRemaining));

        uStockPettyCount = static_cast<uint32_t>(ini.GetLongValue("FollowerStock", "uStockPettyCount", uStockPettyCount));
        uStockLesserCount = static_cast<uint32_t>(ini.GetLongValue("FollowerStock", "uStockLesserCount", uStockLesserCount));
        uStockCommonCount = static_cast<uint32_t>(ini.GetLongValue("FollowerStock", "uStockCommonCount", uStockCommonCount));
        uStockGreaterCount = static_cast<uint32_t>(ini.GetLongValue("FollowerStock", "uStockGreaterCount", uStockGreaterCount));
        uStockGrandCount = static_cast<uint32_t>(ini.GetLongValue("FollowerStock", "uStockGrandCount", uStockGrandCount));

        // Soul Gem Charge Rates
        uChargePetty = static_cast<uint32_t>(ini.GetLongValue("SoulGemChargeRates", "uChargePetty", uChargePetty));
        uChargeLesser = static_cast<uint32_t>(ini.GetLongValue("SoulGemChargeRates", "uChargeLesser", uChargeLesser));
        uChargeCommon = static_cast<uint32_t>(ini.GetLongValue("SoulGemChargeRates", "uChargeCommon", uChargeCommon));
        uChargeGreater = static_cast<uint32_t>(ini.GetLongValue("SoulGemChargeRates", "uChargeGreater", uChargeGreater));
        uChargeGrand = static_cast<uint32_t>(ini.GetLongValue("SoulGemChargeRates", "uChargeGrand", uChargeGrand));

        // Soul Gem Purchase Prices
        uPricePetty = static_cast<uint32_t>(ini.GetLongValue("SoulGemPrices", "uPricePetty", uPricePetty));
        uPriceLesser = static_cast<uint32_t>(ini.GetLongValue("SoulGemPrices", "uPriceLesser", uPriceLesser));
        uPriceCommon = static_cast<uint32_t>(ini.GetLongValue("SoulGemPrices", "uPriceCommon", uPriceCommon));
        uPriceGreater = static_cast<uint32_t>(ini.GetLongValue("SoulGemPrices", "uPriceGreater", uPriceGreater));
        uPriceGrand = static_cast<uint32_t>(ini.GetLongValue("SoulGemPrices", "uPriceGrand", uPriceGrand));

        // Notifications
        bEnableNotifications = ini.GetBoolValue("Notifications", "bEnableNotifications", bEnableNotifications);
        bNotifyPlayerCharge = ini.GetBoolValue("Notifications", "bNotifyPlayerCharge", bNotifyPlayerCharge);
        bNotifyFollowerCharge = ini.GetBoolValue("Notifications", "bNotifyFollowerCharge", bNotifyFollowerCharge);
        bNotifyNoSoulGems = ini.GetBoolValue("Notifications", "bNotifyNoSoulGems", bNotifyNoSoulGems);
        bNotifyFollowerPurchase = ini.GetBoolValue("Notifications", "bNotifyFollowerPurchase", bNotifyFollowerPurchase);
        bNotifyFailedPurchase = ini.GetBoolValue("Notifications", "bNotifyFailedPurchase", bNotifyFailedPurchase);
        bNotifyPassiveRecharge = ini.GetBoolValue("Notifications", "bNotifyPassiveRecharge", bNotifyPassiveRecharge);
        fNotificationCooldown = static_cast<float>(ini.GetDoubleValue("Notifications", "fNotificationCooldown", fNotificationCooldown));

        Save();
        logger::info("Soulkeeper settings loaded successfully.");
    }

    void Settings::Save()
    {
        constexpr auto path = L"Data/SKSE/Plugins/Soulkeeper.ini";

        std::filesystem::path iniPath(path);
        std::error_code ec;
        if (iniPath.has_parent_path()) {
            std::filesystem::create_directories(iniPath.parent_path(), ec);
        }

        CSimpleIniA ini;
        ini.SetUnicode();

        ini.SetBoolValue("General", "bEnableMod", bEnableMod, "; Enable or disable the entire Soulkeeper system");
        ini.SetBoolValue("General", "bEnablePlayerAutoCharge", bEnablePlayerAutoCharge, "; Enable automatic weapon charging for the player");
        ini.SetBoolValue("General", "bEnableFollowerAutoCharge", bEnableFollowerAutoCharge, "; Enable automatic weapon charging for followers");
        ini.SetBoolValue("General", "bEnablePassiveRecharge", bEnablePassiveRecharge, "; Enable passive over-time weapon recharge");

        ini.SetDoubleValue("ChargeThreshold", "fChargeThreshold", fChargeThreshold, "; Weapon charge percentage below which auto-charge triggers (e.g. 20.0 = 20%)");
        ini.SetDoubleValue("ChargeThreshold", "fAutoChargeTargetPercent", fAutoChargeTargetPercent, "; Maximum percentage soul gems will recharge a weapon up to (e.g. 50.0 = 50%)");
        ini.SetDoubleValue("Timers", "fCheckIntervalSeconds", fCheckIntervalSeconds, "; Frequency in seconds between weapon charge checks");

        ini.SetDoubleValue("PassiveRecharge", "fPassiveRechargeInterval", fPassiveRechargeInterval, "; Interval in seconds between passive recharge ticks");
        ini.SetDoubleValue("PassiveRecharge", "fPassiveRechargePercent", fPassiveRechargePercent, "; Percentage of max charge restored per tick");
        ini.SetDoubleValue("PassiveRecharge", "fPassiveRechargeCapPercent", fPassiveRechargeCapPercent, "; Maximum percentage passive recharge can fill a weapon up to (e.g. 20.0 = 20%)");
        ini.SetBoolValue("PassiveRecharge", "bRechargeInCombat", bRechargeInCombat, "; Allow passive recharge while in combat");
        ini.SetDoubleValue("PassiveRecharge", "fCombatRechargeInterval", fCombatRechargeInterval, "; Slower tick interval during combat");

        ini.SetBoolValue("SoulGems", "bPreferSmallestSoulGem", bPreferSmallestSoulGem, "; Always use smallest adequate soul gem first");
        ini.SetBoolValue("SoulGems", "bAllowGrandSoulGems", bAllowGrandSoulGems, "; Allow automatic usage of Grand Soul Gems");
        ini.SetBoolValue("SoulGems", "bAllowBlackSoulGems", bAllowBlackSoulGems, "; Allow automatic usage of Black Soul Gems");

        ini.SetBoolValue("FollowerPurchase", "bEnableFollowerPurchase", bEnableFollowerPurchase, "; Allow followers to buy soul gems with their own gold");
        ini.SetBoolValue("FollowerPurchase", "bEnableFollowerStocking", bEnableFollowerStocking, "; Automatically restock desired soul gems when in town near a vendor");
        ini.SetBoolValue("FollowerPurchase", "bUseFollowerGoldOnly", bUseFollowerGoldOnly, "; Strictly use follower inventory gold only");
        ini.SetLongValue("FollowerPurchase", "uMaxPurchasePrice", uMaxPurchasePrice, "; Maximum gold a follower is willing to spend per soul gem");
        ini.SetLongValue("FollowerPurchase", "uMinGoldRemaining", uMinGoldRemaining, "; Minimum gold a follower must keep after purchase");

        ini.SetLongValue("FollowerStock", "uStockPettyCount", uStockPettyCount, "; Target quantity of Petty Soul Gems in follower inventory");
        ini.SetLongValue("FollowerStock", "uStockLesserCount", uStockLesserCount, "; Target quantity of Lesser Soul Gems in follower inventory");
        ini.SetLongValue("FollowerStock", "uStockCommonCount", uStockCommonCount, "; Target quantity of Common Soul Gems in follower inventory");
        ini.SetLongValue("FollowerStock", "uStockGreaterCount", uStockGreaterCount, "; Target quantity of Greater Soul Gems in follower inventory");
        ini.SetLongValue("FollowerStock", "uStockGrandCount", uStockGrandCount, "; Target quantity of Grand Soul Gems in follower inventory");

        ini.SetLongValue("SoulGemChargeRates", "uChargePetty", uChargePetty, "; Amount of weapon charge restored by a Petty Soul Gem");
        ini.SetLongValue("SoulGemChargeRates", "uChargeLesser", uChargeLesser, "; Amount of weapon charge restored by a Lesser Soul Gem");
        ini.SetLongValue("SoulGemChargeRates", "uChargeCommon", uChargeCommon, "; Amount of weapon charge restored by a Common Soul Gem");
        ini.SetLongValue("SoulGemChargeRates", "uChargeGreater", uChargeGreater, "; Amount of weapon charge restored by a Greater Soul Gem");
        ini.SetLongValue("SoulGemChargeRates", "uChargeGrand", uChargeGrand, "; Amount of weapon charge restored by a Grand / Black Soul Gem");

        ini.SetLongValue("SoulGemPrices", "uPricePetty", uPricePetty, "; Follower purchase cost in Gold for a Petty Soul Gem");
        ini.SetLongValue("SoulGemPrices", "uPriceLesser", uPriceLesser, "; Follower purchase cost in Gold for a Lesser Soul Gem");
        ini.SetLongValue("SoulGemPrices", "uPriceCommon", uPriceCommon, "; Follower purchase cost in Gold for a Common Soul Gem");
        ini.SetLongValue("SoulGemPrices", "uPriceGreater", uPriceGreater, "; Follower purchase cost in Gold for a Greater Soul Gem");
        ini.SetLongValue("SoulGemPrices", "uPriceGrand", uPriceGrand, "; Follower purchase cost in Gold for a Grand Soul Gem");

        ini.SetBoolValue("Notifications", "bEnableNotifications", bEnableNotifications, "; Enable top-left HUD notification messages");
        ini.SetBoolValue("Notifications", "bNotifyPlayerCharge", bNotifyPlayerCharge, "; Show notification when player weapon is charged");
        ini.SetBoolValue("Notifications", "bNotifyFollowerCharge", bNotifyFollowerCharge, "; Show notification when follower weapon is charged");
        ini.SetBoolValue("Notifications", "bNotifyNoSoulGems", bNotifyNoSoulGems, "; Show notification when follower lacks soul gems");
        ini.SetBoolValue("Notifications", "bNotifyFollowerPurchase", bNotifyFollowerPurchase, "; Show notification when follower buys a soul gem");
        ini.SetBoolValue("Notifications", "bNotifyFailedPurchase", bNotifyFailedPurchase, "; Show notification when follower cannot afford a soul gem");
        ini.SetBoolValue("Notifications", "bNotifyPassiveRecharge", bNotifyPassiveRecharge, "; Show notification on passive recharge ticks");
        ini.SetDoubleValue("Notifications", "fNotificationCooldown", fNotificationCooldown, "; Cooldown in seconds between repeat follower notifications");

        (void)ini.SaveFile(path);
    }
}
