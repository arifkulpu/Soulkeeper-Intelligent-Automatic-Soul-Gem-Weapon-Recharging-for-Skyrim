#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class SoulkeeperManager : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static SoulkeeperManager* GetSingleton();

        void Initialize();
        void Update(float a_deltaSeconds);

        // BSTEventSink<MenuOpenCloseEvent>
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                              RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;

    private:
        SoulkeeperManager() = default;
        ~SoulkeeperManager() = default;

        float _checkTimer{ 0.0f };

        void ProcessFollowers();
        void ReassertAllWeaponCharges();
    };
}
