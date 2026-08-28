#include "PassiveRechargeManager.h"
#include "Settings.h"
#include "WeaponManager.h"
#include "FollowerManager.h"
#include "NotificationManager.h"

namespace Soulkeeper
{
    PassiveRechargeManager* PassiveRechargeManager::GetSingleton()
    {
        static PassiveRechargeManager singleton;
        return std::addressof(singleton);
    }

    bool PassiveRechargeManager::RechargeActorWeapons(RE::Actor* a_actor, float a_percent)
    {
        if (!a_actor || a_actor->IsDead()) return false;

        auto weapons = WeaponManager::GetSingleton()->GetEquippedEnchantedWeapons(a_actor);
        const char* actorName = a_actor->GetDisplayFullName() ? a_actor->GetDisplayFullName() : "Player";

        if (weapons.empty()) {
            logger::info("[PassiveRecharge] {} has no enchanted weapons equipped.", actorName);
            return false;
        }

        auto settings = Settings::GetSingleton();
        float capPercent = std::clamp(settings->fPassiveRechargeCapPercent, 1.0f, 100.0f);
        bool anyRecharged = false;

        for (const auto& wpn : weapons) {
            float maxAllowedCharge = wpn.maxCharge * (capPercent / 100.0f);
            if (wpn.currentCharge < maxAllowedCharge) {
                float chargeToAdd = wpn.maxCharge * (a_percent / 100.0f);
                if (chargeToAdd < 1.0f) chargeToAdd = 1.0f;

                float targetCharge = std::min(wpn.currentCharge + chargeToAdd, maxAllowedCharge);
                logger::info("[PassiveRecharge] {} | '{}' {:.0f}/{:.0f} (Cap: {:.0f}%) -> target: {:.0f}",
                    actorName, wpn.weapon->GetName(), wpn.currentCharge, wpn.maxCharge, capPercent, targetCharge);
                if (WeaponManager::GetSingleton()->SetWeaponCharge(a_actor, wpn, targetCharge)) {
                    anyRecharged = true;
                }
            } else {
                logger::info("[PassiveRecharge] {} | '{}' at/above cap ({:.0f}/{:.0f}, Cap: {:.0f}%), skipping.",
                    actorName, wpn.weapon->GetName(), wpn.currentCharge, wpn.maxCharge, capPercent);
            }
        }

        return anyRecharged;
    }

    void PassiveRechargeManager::ResetGameTimeTracker()
    {
        if (auto* calendar = RE::Calendar::GetSingleton()) {
            _lastGameHoursPassed = calendar->GetHoursPassed();
        } else {
            _lastGameHoursPassed = -1.0f;
        }
        _accumulatedTime = 0.0f;
    }

    void PassiveRechargeManager::ProcessPassiveRecharge(float a_deltaSeconds)
    {
        auto settings = Settings::GetSingleton();
        if (!settings->bEnablePassiveRecharge) return;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player) return;

        bool inCombat = player->IsInCombat();
        if (inCombat && !settings->bRechargeInCombat) {
            return;
        }

        // 1. Check game calendar time for jumps (Sleeping, Waiting, Fast Traveling)
        auto* calendar = RE::Calendar::GetSingleton();
        if (calendar) {
            float currentHours = calendar->GetHoursPassed();
            if (_lastGameHoursPassed < 0.0f) {
                _lastGameHoursPassed = currentHours;
            } else {
                float hoursDelta = currentHours - _lastGameHoursPassed;
                _lastGameHoursPassed = currentHours;

                // If at least ~6 in-game minutes passed (0.1 hours), calculate multi-tick recharge
                if (hoursDelta >= 0.1f) {
                    float timescale = calendar->GetTimescale();
                    if (timescale <= 0.0f) timescale = 20.0f;

                    // Convert elapsed game hours into equivalent real-time seconds
                    float equivalentRealSeconds = (hoursDelta * 3600.0f) / timescale;
                    float interval = inCombat ? settings->fCombatRechargeInterval : settings->fPassiveRechargeInterval;
                    if (interval <= 0.0f) interval = 60.0f;

                    float ticks = equivalentRealSeconds / interval;
                    if (ticks >= 1.0f) {
                        float totalPercent = std::min(ticks * settings->fPassiveRechargePercent, 100.0f);
                        logger::info("[PassiveRecharge] Game time jump detected: {:.2f} game hours passed ({:.1f} ticks) -> Recharging {:.1f}%",
                            hoursDelta, ticks, totalPercent);

                        bool chargedAny = false;

                        // Recharge player equipped weapons
                        if (RechargeActorWeapons(player, totalPercent)) {
                            chargedAny = true;
                        }

                        // Recharge followers equipped weapons
                        auto followers = FollowerManager::GetSingleton()->GetActiveFollowers();
                        for (auto* follower : followers) {
                            if (RechargeActorWeapons(follower, totalPercent)) {
                                chargedAny = true;
                            }
                        }

                        if (chargedAny && settings->bNotifyPassiveRecharge) {
                            NotificationManager::GetSingleton()->NotifyPassiveRecharge(totalPercent);
                        }
                        _accumulatedTime = 0.0f;
                        return;
                    }
                }
            }
        }

        // 2. Real-time periodic passive recharge tick
        float targetInterval = inCombat ? settings->fCombatRechargeInterval : settings->fPassiveRechargeInterval;
        if (targetInterval <= 0.0f) targetInterval = 60.0f;

        _accumulatedTime += a_deltaSeconds;
        if (_accumulatedTime >= targetInterval) {
            _accumulatedTime = 0.0f;

            bool chargedAny = false;

            // Recharge player
            if (RechargeActorWeapons(player, settings->fPassiveRechargePercent)) {
                chargedAny = true;
            }

            // Recharge followers
            auto followers = FollowerManager::GetSingleton()->GetActiveFollowers();
            for (auto* follower : followers) {
                if (RechargeActorWeapons(follower, settings->fPassiveRechargePercent)) {
                    chargedAny = true;
                }
            }

            // Sadece gerçekten bir silah şarj edildiyse bildirim gönder
            if (chargedAny && settings->bNotifyPassiveRecharge) {
                NotificationManager::GetSingleton()->NotifyPassiveRecharge(settings->fPassiveRechargePercent);
            }
        }
    }
}
