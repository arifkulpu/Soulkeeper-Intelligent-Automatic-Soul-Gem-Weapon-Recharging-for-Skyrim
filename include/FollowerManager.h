#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class FollowerManager
    {
    public:
        static FollowerManager* GetSingleton();

        // Detects all active followers (humanoids, animals, framework followers)
        std::vector<RE::Actor*> GetActiveFollowers();

        // Checks if a specific actor is an active follower
        bool IsActiveFollower(RE::Actor* a_actor);

    private:
        FollowerManager() = default;
        ~FollowerManager() = default;

        RE::TESFaction* _dialogueFollowerFaction{ nullptr };
        RE::TESFaction* _currentFollowerFaction{ nullptr };
        bool _initializedFactions{ false };

        void InitFactions();
    };
}
