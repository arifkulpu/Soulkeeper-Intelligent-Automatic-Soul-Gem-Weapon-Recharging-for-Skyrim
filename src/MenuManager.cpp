#include "MenuManager.h"
#include "Settings.h"
#include "PlayerManager.h"
#include "FollowerManager.h"
#include "WeaponManager.h"
#include "SoulGemManager.h"
#include "PurchaseManager.h"
#include "PassiveRechargeManager.h"
#include "NotificationManager.h"

#include "SKSEMenuFramework.h"

namespace Soulkeeper
{
    MenuManager* MenuManager::GetSingleton()
    {
        static MenuManager singleton;
        return std::addressof(singleton);
    }

    void MenuManager::Register()
    {
        if (_registered) return;

        if (!SKSEMenuFramework::IsInstalled()) {
            logger::info("SKSEMenuFramework is not installed, skipping UI registration.");
            return;
        }

        SKSEMenuFramework::SetSection("Soulkeeper");
        SKSEMenuFramework::AddSectionItem("Control Panel & Settings", RenderMenu);

        _registered = true;
        logger::info("Registered Soulkeeper with SKSE Menu Framework.");
    }

    void MenuManager::RenderMenu()
    {
        auto settings = Settings::GetSingleton();

        ImGuiMCP::Text("Soulkeeper - Smart Weapon Charging System");
        ImGuiMCP::TextDisabled("Runtime: Skyrim AE 1.6.1170 | SKSE64");
        ImGuiMCP::Separator();

        if (ImGuiMCP::BeginTabBar("SoulkeeperTabs", 0)) {

            // --- TAB 1: General & Settings ---
            if (ImGuiMCP::BeginTabItem("Settings", nullptr, 0)) {
                bool settingsChanged = false;

                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Master Switches");
                if (ImGuiMCP::Checkbox("Enable Soulkeeper Mod", &settings->bEnableMod)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Enable Player Auto Charge", &settings->bEnablePlayerAutoCharge)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Enable Follower Auto Charge", &settings->bEnableFollowerAutoCharge)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Enable Passive Weapon Recharge", &settings->bEnablePassiveRecharge)) settingsChanged = true;

                ImGuiMCP::Separator();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Thresholds & Timers");
                if (ImGuiMCP::SliderFloat("Charge Trigger Threshold (%)", &settings->fChargeThreshold, 1.0f, 99.0f, "%.0f%%", 0)) settingsChanged = true;
                if (ImGuiMCP::SliderFloat("Soul Gem Recharge Target Cap (%)", &settings->fAutoChargeTargetPercent, 1.0f, 100.0f, "%.0f%%", 0)) settingsChanged = true;
                if (ImGuiMCP::SliderFloat("Auto-Charge Check Interval (sec)", &settings->fCheckIntervalSeconds, 5.0f, 120.0f, "%.0f s", 0)) settingsChanged = true;

                ImGuiMCP::Separator();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Soul Gem Selection Rules");
                if (ImGuiMCP::Checkbox("Prefer Smallest Adequate Soul Gem", &settings->bPreferSmallestSoulGem)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Allow Grand Soul Gems", &settings->bAllowGrandSoulGems)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Allow Black Soul Gems", &settings->bAllowBlackSoulGems)) settingsChanged = true;

                ImGuiMCP::Separator();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Follower Soul Gem Purchasing & Town Stocking");
                if (ImGuiMCP::Checkbox("Allow Followers to Buy Soul Gems", &settings->bEnableFollowerPurchase)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Auto-Restock Stock Quantities in Town", &settings->bEnableFollowerStocking)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Strictly Follower Gold Only", &settings->bUseFollowerGoldOnly)) settingsChanged = true;

                int maxPrice = static_cast<int>(settings->uMaxPurchasePrice);
                if (ImGuiMCP::SliderInt("Max Gold Follower Can Spend", &maxPrice, 50, 2000, "%d Gold", 0)) {
                    settings->uMaxPurchasePrice = static_cast<uint32_t>(maxPrice);
                    settingsChanged = true;
                }

                int minGold = static_cast<int>(settings->uMinGoldRemaining);
                if (ImGuiMCP::SliderInt("Min Gold Retained by Follower", &minGold, 0, 500, "%d Gold", 0)) {
                    settings->uMinGoldRemaining = static_cast<uint32_t>(minGold);
                    settingsChanged = true;
                }

                if (settings->bEnableFollowerStocking) {
                    ImGuiMCP::Spacing();
                    ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Follower Target Stock Inventory (Per Tier):");

                    int stockPetty = static_cast<int>(settings->uStockPettyCount);
                    if (ImGuiMCP::SliderInt("Petty Soul Gems Stock (50g / 250chg)", &stockPetty, 0, 20, "%d gems", 0)) {
                        settings->uStockPettyCount = static_cast<uint32_t>(stockPetty);
                        settingsChanged = true;
                    }

                    int stockLesser = static_cast<int>(settings->uStockLesserCount);
                    if (ImGuiMCP::SliderInt("Lesser Soul Gems Stock (100g / 500chg)", &stockLesser, 0, 20, "%d gems", 0)) {
                        settings->uStockLesserCount = static_cast<uint32_t>(stockLesser);
                        settingsChanged = true;
                    }

                    int stockCommon = static_cast<int>(settings->uStockCommonCount);
                    if (ImGuiMCP::SliderInt("Common Soul Gems Stock (150g / 1000chg)", &stockCommon, 0, 20, "%d gems", 0)) {
                        settings->uStockCommonCount = static_cast<uint32_t>(stockCommon);
                        settingsChanged = true;
                    }

                    int stockGreater = static_cast<int>(settings->uStockGreaterCount);
                    if (ImGuiMCP::SliderInt("Greater Soul Gems Stock (300g / 2000chg)", &stockGreater, 0, 20, "%d gems", 0)) {
                        settings->uStockGreaterCount = static_cast<uint32_t>(stockGreater);
                        settingsChanged = true;
                    }

                    int stockGrand = static_cast<int>(settings->uStockGrandCount);
                    if (ImGuiMCP::SliderInt("Grand Soul Gems Stock (500g / 3000chg)", &stockGrand, 0, 20, "%d gems", 0)) {
                        settings->uStockGrandCount = static_cast<uint32_t>(stockGrand);
                        settingsChanged = true;
                    }
                }

                ImGuiMCP::Separator();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Passive Over-Time Recharge");
                if (ImGuiMCP::SliderFloat("Passive Recharge Rate (% / tick)", &settings->fPassiveRechargePercent, 0.1f, 10.0f, "%.1f%%", 0)) settingsChanged = true;
                if (ImGuiMCP::SliderFloat("Passive Recharge Cap Limit (%)", &settings->fPassiveRechargeCapPercent, 1.0f, 100.0f, "%.0f%%", 0)) settingsChanged = true;
                if (ImGuiMCP::SliderFloat("Passive Recharge Interval (sec)", &settings->fPassiveRechargeInterval, 10.0f, 300.0f, "%.0f s", 0)) settingsChanged = true;
                if (ImGuiMCP::Checkbox("Allow Passive Recharge During Combat", &settings->bRechargeInCombat)) settingsChanged = true;
                if (settings->bRechargeInCombat) {
                    if (ImGuiMCP::SliderFloat("Combat Recharge Interval (sec)", &settings->fCombatRechargeInterval, 30.0f, 600.0f, "%.0f s", 0)) settingsChanged = true;
                }

                ImGuiMCP::Separator();
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Notification Preferences");
                if (ImGuiMCP::Checkbox("Enable HUD Notifications", &settings->bEnableNotifications)) settingsChanged = true;
                if (settings->bEnableNotifications) {
                    if (ImGuiMCP::Checkbox("Notify Player Weapon Charged", &settings->bNotifyPlayerCharge)) settingsChanged = true;
                    if (ImGuiMCP::Checkbox("Notify Follower Weapon Charged", &settings->bNotifyFollowerCharge)) settingsChanged = true;
                    if (ImGuiMCP::Checkbox("Notify Follower Lacks Soul Gems", &settings->bNotifyNoSoulGems)) settingsChanged = true;
                    if (ImGuiMCP::Checkbox("Notify Follower Soul Gem Purchase", &settings->bNotifyFollowerPurchase)) settingsChanged = true;
                    if (ImGuiMCP::Checkbox("Notify Follower Insufficient Gold", &settings->bNotifyFailedPurchase)) settingsChanged = true;
                    if (ImGuiMCP::Checkbox("Notify Passive Recharge Ticks", &settings->bNotifyPassiveRecharge)) settingsChanged = true;
                    if (ImGuiMCP::SliderFloat("Follower Notification Cooldown (sec)", &settings->fNotificationCooldown, 10.0f, 900.0f, "%.0f s", 0)) settingsChanged = true;
                }

                if (settingsChanged) {
                    settings->Save();
                }

                ImGuiMCP::EndTabItem();
            }

            // --- TAB 2: Live Status & Quick Actions ---
            if (ImGuiMCP::BeginTabItem("Live Status & Controls", nullptr, 0)) {
                auto player = RE::PlayerCharacter::GetSingleton();

                // Live real-time inspection when tab is viewed
                std::vector<EnchantedWeaponInfo> playerWeapons;
                if (player) {
                    playerWeapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(player);
                }

                auto followers = FollowerManager::GetSingleton()->GetActiveFollowers();
                std::unordered_map<RE::Actor*, std::vector<EnchantedWeaponInfo>> followerWeapons;
                for (auto* fol : followers) {
                    if (fol) {
                        followerWeapons[fol] = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(fol);
                    }
                }

                // Player Section
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Player Status:");
                if (player) {
                    if (playerWeapons.empty()) {
                        ImGuiMCP::TextDisabled("  No enchanted weapon currently equipped.");
                    } else {
                        for (const auto& wpn : playerWeapons) {
                            float pct = wpn.chargePercent / 100.0f;

                            // Color: green > 60%, yellow > 25%, red otherwise
                            ImGuiMCP::ImVec4 barColor =
                                (pct > 0.60f) ? ImGuiMCP::ImVec4(0.15f, 0.85f, 0.3f, 1.0f) :
                                (pct > 0.25f) ? ImGuiMCP::ImVec4(0.95f, 0.75f, 0.1f, 1.0f) :
                                                ImGuiMCP::ImVec4(0.9f,  0.2f,  0.2f, 1.0f);

                            // Weapon name + hand label
                            ImGuiMCP::Text("  [%s] %s", wpn.isLeftHand ? "L" : "R", wpn.weapon->GetName());

                            // Progress bar with colored fill
                            ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_PlotHistogram, barColor);
                            char barLabel[64];
                            snprintf(barLabel, sizeof(barLabel), "  %.1f%%  ", wpn.chargePercent);
                            ImGuiMCP::ProgressBar(pct, ImGuiMCP::ImVec2(240.0f, 30.0f), barLabel);
                            ImGuiMCP::PopStyleColor(1);

                            // Charge value text below bar
                            ImGuiMCP::Text("    Charge: %.0f / %.0f", wpn.currentCharge, wpn.maxCharge);
                            ImGuiMCP::Spacing();
                        }
                    }

                    if (ImGuiMCP::Button("Force Charge Player Weapons", ImGuiMCP::ImVec2(0, 0))) {
                        PlayerManager::GetSingleton()->ProcessPlayer();
                    }
                } else {
                    ImGuiMCP::TextDisabled("  Player not loaded.");
                }

                ImGuiMCP::Separator();

                // Followers Section
                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Active Followers Status:");
                if (followers.empty()) {
                    ImGuiMCP::TextDisabled("  No active followers detected nearby.");
                } else {
                    for (auto* follower : followers) {
                        if (!follower) continue;
                        const char* name = follower->GetDisplayFullName() ? follower->GetDisplayFullName() : "Follower";
                        int32_t gold = PurchaseManager::GetSingleton()->GetFollowerGold(follower);

                        ImGuiMCP::Text("• %s  (Gold: %d)", name, gold);
                        auto it = followerWeapons.find(follower);
                        if (it == followerWeapons.end() || it->second.empty()) {
                            ImGuiMCP::TextDisabled("    No enchanted weapon equipped.");
                        } else {
                            for (const auto& wpn : it->second) {
                                float pct = wpn.chargePercent / 100.0f;

                                ImGuiMCP::ImVec4 barColor =
                                    (pct > 0.60f) ? ImGuiMCP::ImVec4(0.15f, 0.85f, 0.3f, 1.0f) :
                                    (pct > 0.25f) ? ImGuiMCP::ImVec4(0.95f, 0.75f, 0.1f, 1.0f) :
                                                    ImGuiMCP::ImVec4(0.9f,  0.2f,  0.2f, 1.0f);

                                ImGuiMCP::TextColored(ImGuiMCP::ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "    [%s] %s", wpn.isLeftHand ? "L" : "R", wpn.weapon->GetName());

                                ImGuiMCP::PushStyleColor(ImGuiMCP::ImGuiCol_PlotHistogram, barColor);
                                char barLabel[64];
                                snprintf(barLabel, sizeof(barLabel), "  %.1f%%  ", wpn.chargePercent);
                                ImGuiMCP::ProgressBar(pct, ImGuiMCP::ImVec2(240.0f, 28.0f), barLabel);
                                ImGuiMCP::PopStyleColor(1);

                                ImGuiMCP::Text("    Charge: %.0f / %.0f", wpn.currentCharge, wpn.maxCharge);
                                ImGuiMCP::Spacing();
                            }
                        }
                    }
                }

                ImGuiMCP::Separator();

                if (ImGuiMCP::Button("Instant Recharge All Weapons (Debug)", ImGuiMCP::ImVec2(0, 0))) {
                    if (player) {
                        for (const auto& w : WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(player)) {
                            WeaponManager::GetSingleton()->SetWeaponCharge(player, w, w.maxCharge);
                        }
                    }
                    for (auto* fol : followers) {
                        for (const auto& w : WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(fol)) {
                            WeaponManager::GetSingleton()->SetWeaponCharge(fol, w, w.maxCharge);
                        }
                    }
                    NotificationManager::GetSingleton()->Notify("All weapons instantly recharged (Debug).");
                }

                ImGuiMCP::SameLine(0, -1);
                if (ImGuiMCP::Button("Reload Soulkeeper.ini", ImGuiMCP::ImVec2(0, 0))) {
                    settings->Load();
                    NotificationManager::GetSingleton()->Notify("Soulkeeper configuration reloaded.");
                }

                ImGuiMCP::EndTabItem();
            }

            ImGuiMCP::EndTabBar();
        }
    }
}
