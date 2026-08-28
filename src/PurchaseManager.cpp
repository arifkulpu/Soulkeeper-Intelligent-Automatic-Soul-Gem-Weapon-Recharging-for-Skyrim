#include "PurchaseManager.h"
#include "Settings.h"
#include "SoulGemManager.h"
#include "NotificationManager.h"

namespace Soulkeeper
{
    PurchaseManager* PurchaseManager::GetSingleton()
    {
        static PurchaseManager singleton;
        return std::addressof(singleton);
    }

    uint32_t PurchaseManager::CalculateSoulGemPrice(RE::SOUL_LEVEL a_level)
    {
        switch (a_level) {
        case RE::SOUL_LEVEL::kPetty:   return 50;
        case RE::SOUL_LEVEL::kLesser:  return 100;
        case RE::SOUL_LEVEL::kCommon:  return 150;
        case RE::SOUL_LEVEL::kGreater: return 300;
        case RE::SOUL_LEVEL::kGrand:   return 500;
        default:                       return 0;
        }
    }

    int32_t PurchaseManager::GetFollowerGold(RE::Actor* a_follower)
    {
        if (!a_follower) return 0;
        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
        if (!goldForm) return 0;

        auto inv = a_follower->GetInventory();
        auto it = inv.find(goldForm);
        if (it != inv.end()) {
            return it->second.first;
        }
        return 0;
    }

    RE::TESSoulGem* PurchaseManager::GetStandardSoulGemForm(RE::SOUL_LEVEL a_level)
    {
        // Filled standard soul gems FormIDs
        switch (a_level) {
        case RE::SOUL_LEVEL::kPetty:   return RE::TESForm::LookupByID<RE::TESSoulGem>(0x0002E4E3); // SoulGemPettyFilled
        case RE::SOUL_LEVEL::kLesser:  return RE::TESForm::LookupByID<RE::TESSoulGem>(0x0002E4E5); // SoulGemLesserFilled
        case RE::SOUL_LEVEL::kCommon:  return RE::TESForm::LookupByID<RE::TESSoulGem>(0x0002E4F3); // SoulGemCommonFilled
        case RE::SOUL_LEVEL::kGreater: return RE::TESForm::LookupByID<RE::TESSoulGem>(0x0002E4FB); // SoulGemGreaterFilled
        case RE::SOUL_LEVEL::kGrand:   return RE::TESForm::LookupByID<RE::TESSoulGem>(0x0002E4FF); // SoulGemGrandFilled
        default:                       return nullptr;
        }
    }

    bool PurchaseManager::IsSoulGemVendor(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsDead() || a_actor->IsHostileToActor(RE::PlayerCharacter::GetSingleton())) {
            return false;
        }

        auto* base = a_actor->GetActorBase();
        if (!base) return false;

        // Check if actor belongs to any merchant/vendor faction
        for (const auto& fRank : base->factions) {
            auto* faction = fRank.faction;
            if (!faction) continue;

            // 1. Is this faction a Vendor faction?
            if (faction->IsVendor()) {
                if (auto* sellList = faction->vendorData.vendorSellBuyList) {
                    if (sellList->ContainsOnlyType(RE::FormType::SoulGem) ||
                        sellList->ContainsOnlyType(RE::FormType::Misc) ||
                        sellList->forms.size() > 0) {
                        return true;
                    }
                } else {
                    return true;
                }
            }

            // 2. Check known wizard / merchant / court mage / apothecary factions by name
            if (const char* name = faction->GetName()) {
                std::string fName(name);
                std::transform(fName.begin(), fName.end(), fName.begin(), ::tolower);
                if (fName.find("vendor") != std::string::npos ||
                    fName.find("merchant") != std::string::npos ||
                    fName.find("court") != std::string::npos ||
                    fName.find("mage") != std::string::npos ||
                    fName.find("apothecary") != std::string::npos ||
                    fName.find("college") != std::string::npos) {
                    return true;
                }
            }
        }

        return false;
    }

    bool PurchaseManager::CanPurchaseAtCurrentLocation(RE::Actor* a_follower)
    {
        if (!a_follower) return false;

        // Follower must not be in combat
        if (a_follower->IsInCombat()) return false;

        // 1. Check if follower is in a town / city / settlement / interior shop
        bool inCivilizedArea = false;
        auto* cell = a_follower->GetParentCell();
        if (cell) {
            // Interior cells (shops, mages quarters, taverns, Jarl palaces)
            if (cell->IsInteriorCell()) {
                inCivilizedArea = true;
            }
            // Check location keywords (LocTypeTown, LocTypeCity, LocTypeSettlement, LocTypeDwelling)
            if (auto* loc = a_follower->GetCurrentLocation()) {
                for (auto* k : loc->GetKeywords()) {
                    if (!k) continue;
                    std::string_view tag = k->GetFormEditorID();
                    if (tag.find("Town") != std::string::npos ||
                        tag.find("City") != std::string::npos ||
                        tag.find("Settlement") != std::string::npos ||
                        tag.find("Dwelling") != std::string::npos ||
                        tag.find("College") != std::string::npos ||
                        tag.find("Store") != std::string::npos ||
                        tag.find("Inn") != std::string::npos) {
                        inCivilizedArea = true;
                        break;
                    }
                }
            }
        }

        if (!inCivilizedArea) {
            return false;
        }

        // 2. Scan nearby loaded NPCs (within ~3500 units / ~50 meters) for a valid merchant/vendor
        bool vendorNearby = false;
        const auto followerPos = a_follower->GetPosition();
        constexpr float maxDistanceSq = 3500.0f * 3500.0f; // ~50m radius

        if (auto* processLists = RE::ProcessLists::GetSingleton()) {
            processLists->ForEachHighActor([&](RE::Actor* actor) -> RE::BSContainer::ForEachResult {
                if (!actor || actor == a_follower || actor->IsPlayerRef()) {
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                float distSq = followerPos.GetSquaredDistance(actor->GetPosition());
                if (distSq <= maxDistanceSq) {
                    if (IsSoulGemVendor(actor)) {
                        vendorNearby = true;
                        logger::info("[PurchaseManager] Found nearby vendor '{}' for follower {}.",
                            actor->GetDisplayFullName() ? actor->GetDisplayFullName() : "NPC",
                            a_follower->GetDisplayFullName() ? a_follower->GetDisplayFullName() : "Follower");
                        return RE::BSContainer::ForEachResult::kStop;
                    }
                }
                return RE::BSContainer::ForEachResult::kContinue;
            });
        }

        return vendorNearby;
    }

    bool PurchaseManager::TryFollowerPurchaseSoulGem(RE::Actor* a_follower, const EnchantedWeaponInfo& a_weapon)
    {
        if (!a_follower || !a_weapon.IsValid()) return false;

        auto settings = Settings::GetSingleton();
        if (!settings->bEnableFollowerPurchase) return false;

        // Followers can only purchase when in a town/settlement/shop with a vendor nearby!
        if (!CanPurchaseAtCurrentLocation(a_follower)) {
            logger::info("[PurchaseManager] Follower {} cannot purchase: not in town/settlement or no vendor nearby.",
                a_follower->GetDisplayFullName() ? a_follower->GetDisplayFullName() : "Follower");
            return false;
        }

        int32_t currentGold = GetFollowerGold(a_follower);
        if (currentGold <= static_cast<int32_t>(settings->uMinGoldRemaining)) {
            NotificationManager::GetSingleton()->NotifyFollowerCannotAfford(a_follower);
            return false;
        }

        // Determine needed soul level based on missing charge
        float missingCharge = a_weapon.GetMissingCharge();
        std::array<RE::SOUL_LEVEL, 5> levels = {
            RE::SOUL_LEVEL::kPetty,
            RE::SOUL_LEVEL::kLesser,
            RE::SOUL_LEVEL::kCommon,
            RE::SOUL_LEVEL::kGreater,
            RE::SOUL_LEVEL::kGrand
        };

        RE::SOUL_LEVEL targetLevel = RE::SOUL_LEVEL::kPetty;
        for (auto lvl : levels) {
            if (static_cast<float>(SoulGemManager::GetSoulChargeValue(lvl)) >= missingCharge) {
                targetLevel = lvl;
                break;
            }
            targetLevel = lvl;
        }

        if (targetLevel == RE::SOUL_LEVEL::kGrand && !settings->bAllowGrandSoulGems) {
            targetLevel = RE::SOUL_LEVEL::kGreater;
        }

        // Check price affordability, downgrading if necessary
        RE::TESSoulGem* gemToBuy = nullptr;
        uint32_t cost = 0;

        int startIdx = static_cast<int>(targetLevel) - 1;
        for (int i = startIdx; i >= 0; --i) {
            auto lvl = levels[i];
            uint32_t price = CalculateSoulGemPrice(lvl);
            if (price <= settings->uMaxPurchasePrice &&
                (currentGold - static_cast<int32_t>(price)) >= static_cast<int32_t>(settings->uMinGoldRemaining)) {
                gemToBuy = GetStandardSoulGemForm(lvl);
                cost = price;
                break;
            }
        }

        if (!gemToBuy || cost == 0) {
            NotificationManager::GetSingleton()->NotifyFollowerCannotAfford(a_follower);
            return false;
        }

        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
        if (!goldForm) return false;

        // Deduct follower gold strictly
        a_follower->RemoveItem(goldForm, cost, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
        // Add purchased soul gem
        a_follower->AddObjectToContainer(gemToBuy, nullptr, 1, nullptr);

        logger::info("Follower {} purchased {} for {} gold (remaining: {}).",
            a_follower->GetDisplayFullName() ? a_follower->GetDisplayFullName() : "Follower",
            gemToBuy->GetName(),
            cost,
            currentGold - cost
        );

        NotificationManager::GetSingleton()->NotifyFollowerPurchased(
            a_follower,
            cost,
            gemToBuy->GetName() ? gemToBuy->GetName() : "Soul Gem"
        );

        // Immediately charge weapon with the newly acquired soul gem
        AvailableSoulGem purchasedGem;
        purchasedGem.soulGem = gemToBuy;
        purchasedGem.soulLevel = gemToBuy->GetContainedSoul();
        purchasedGem.chargeValue = SoulGemManager::GetSoulChargeValue(purchasedGem.soulLevel);
        purchasedGem.count = 1;
        purchasedGem.isBlack = false;

        return SoulGemManager::GetSingleton()->ChargeWeaponWithSoulGem(a_follower, a_weapon, purchasedGem);
    }

    void PurchaseManager::TryFollowerRestockStockGems(RE::Actor* a_follower)
    {
        if (!a_follower || a_follower->IsDead()) return;

        auto settings = Settings::GetSingleton();
        if (!settings->bEnableFollowerPurchase || !settings->bEnableFollowerStocking) return;

        if (!CanPurchaseAtCurrentLocation(a_follower)) return;

        auto goldForm = RE::TESForm::LookupByID<RE::TESBoundObject>(0x0000000F);
        if (!goldForm) return;

        int32_t currentGold = GetFollowerGold(a_follower);

        // Count current filled soul gems in follower inventory
        auto existingGems = SoulGemManager::GetSingleton()->GetAvailableSoulGems(a_follower);
        std::map<RE::SOUL_LEVEL, uint32_t> currentCounts;
        for (const auto& g : existingGems) {
            if (!g.isBlack) {
                currentCounts[g.soulLevel] += g.count;
            }
        }

        struct StockTarget {
            RE::SOUL_LEVEL level;
            uint32_t targetCount;
        };

        std::array<StockTarget, 5> targets = {
            StockTarget{ RE::SOUL_LEVEL::kPetty,   settings->uStockPettyCount },
            StockTarget{ RE::SOUL_LEVEL::kLesser,  settings->uStockLesserCount },
            StockTarget{ RE::SOUL_LEVEL::kCommon,  settings->uStockCommonCount },
            StockTarget{ RE::SOUL_LEVEL::kGreater, settings->uStockGreaterCount },
            StockTarget{ RE::SOUL_LEVEL::kGrand,   settings->bAllowGrandSoulGems ? settings->uStockGrandCount : 0 }
        };

        uint32_t totalBought = 0;
        uint32_t totalSpent = 0;

        for (const auto& tgt : targets) {
            if (tgt.targetCount == 0) continue;
            uint32_t countHave = currentCounts[tgt.level];
            if (countHave >= tgt.targetCount) continue;

            uint32_t needed = tgt.targetCount - countHave;
            uint32_t unitPrice = CalculateSoulGemPrice(tgt.level);
            if (unitPrice == 0 || unitPrice > settings->uMaxPurchasePrice) continue;

            auto* gemForm = GetStandardSoulGemForm(tgt.level);
            if (!gemForm) continue;

            for (uint32_t i = 0; i < needed; ++i) {
                if ((currentGold - static_cast<int32_t>(unitPrice)) < static_cast<int32_t>(settings->uMinGoldRemaining)) {
                    break; // Cannot afford more
                }

                currentGold -= unitPrice;
                totalSpent += unitPrice;
                totalBought++;

                a_follower->RemoveItem(goldForm, unitPrice, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
                a_follower->AddObjectToContainer(gemForm, nullptr, 1, nullptr);
            }
        }

        if (totalBought > 0) {
            logger::info("Follower {} restocked {} soul gems for {} gold (remaining: {}).",
                a_follower->GetDisplayFullName() ? a_follower->GetDisplayFullName() : "Follower",
                totalBought,
                totalSpent,
                currentGold
            );

            if (settings->bNotifyFollowerPurchase) {
                char msg[128];
                snprintf(msg, sizeof(msg), "%s restocked %u Soul Gems for %u Gold.",
                    a_follower->GetDisplayFullName() ? a_follower->GetDisplayFullName() : "Follower",
                    totalBought,
                    totalSpent);
                RE::DebugNotification(msg);
            }
        }
    }
}

