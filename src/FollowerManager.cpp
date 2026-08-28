#include "FollowerManager.h"

namespace Soulkeeper
{
    FollowerManager* FollowerManager::GetSingleton()
    {
        static FollowerManager singleton;
        return std::addressof(singleton);
    }

    void FollowerManager::InitFactions()
    {
        if (_initializedFactions) return;

        auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler) {
            // DialogueFollowerFaction (0x0005C84E)
            _dialogueFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x0005C84E);
            // CurrentFollowerFaction (0x0005C84E or 0x00084D1B / 0x000723A8)
            _currentFollowerFaction = RE::TESForm::LookupByID<RE::TESFaction>(0x000723A8);
            _initializedFactions = true;
        }
    }

    bool FollowerManager::IsActiveFollower(RE::Actor* a_actor)
    {
        if (!a_actor || a_actor->IsPlayerRef() || a_actor->IsDead() || a_actor->IsDisabled()) {
            return false;
        }

        // Framework / Vanilla: IsPlayerTeammate is set true for active followers
        if (a_actor->IsPlayerTeammate()) {
            return true;
        }

        // Check commanding actor (Summons / special followers)
        auto commandingActor = a_actor->GetCommandingActor();
        if (commandingActor && commandingActor->IsPlayerRef()) {
            return true;
        }

        // Faction checks
        InitFactions();
        if (_currentFollowerFaction && a_actor->IsInFaction(_currentFollowerFaction)) {
            return true;
        }

        return false;
    }

    std::vector<RE::Actor*> FollowerManager::GetActiveFollowers()
    {
        std::vector<RE::Actor*> followers;

        auto processList = RE::ProcessLists::GetSingleton();
        if (!processList) return followers;

        // Iterate high-process actors near the player
        for (auto& handle : processList->highActorHandles) {
            auto actorPtr = handle.get();
            if (actorPtr && IsActiveFollower(actorPtr.get())) {
                followers.push_back(actorPtr.get());
            }
        }

        return followers;
    }
}
