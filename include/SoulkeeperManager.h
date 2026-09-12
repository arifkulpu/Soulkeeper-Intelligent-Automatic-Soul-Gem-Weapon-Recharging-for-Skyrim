#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class SoulkeeperManager : 
        public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
        public RE::BSTEventSink<RE::TESHitEvent>,
        public RE::BSTEventSink<RE::TESPlayerBowShotEvent>
    {
    public:
        static SoulkeeperManager* GetSingleton();

        void Initialize();
        void Update(float a_deltaSeconds);

        // BSTEventSink<MenuOpenCloseEvent>
        RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                              RE::BSTEventSource<RE::MenuOpenCloseEvent>* a_source) override;

        // BSTEventSink<TESHitEvent>
        RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event,
                                              RE::BSTEventSource<RE::TESHitEvent>* a_source) override;

        // BSTEventSink<TESPlayerBowShotEvent>
        RE::BSEventNotifyControl ProcessEvent(const RE::TESPlayerBowShotEvent* a_event,
                                              RE::BSTEventSource<RE::TESPlayerBowShotEvent>* a_source) override;

    private:
        SoulkeeperManager() = default;
        ~SoulkeeperManager() = default;

        float _checkTimer{ 0.0f };

        void ProcessFollowers();
        void ReassertAllWeaponCharges();
    };
}
