#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class PassiveRechargeManager
    {
    public:
        static PassiveRechargeManager* GetSingleton();

        // Process passive recharge for player and followers based on delta & game hours
        void ProcessPassiveRecharge(float a_deltaSeconds);

        // Reset the calendar hour tracker (e.g. after loading a game)
        void ResetGameTimeTracker();

    private:
        PassiveRechargeManager() = default;
        ~PassiveRechargeManager() = default;

        float _accumulatedTime{ 0.0f };
        float _lastGameHoursPassed{ -1.0f };

        bool RechargeActorWeapons(RE::Actor* a_actor, float a_percent);
    };
}
