#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class MenuManager
    {
    public:
        static MenuManager* GetSingleton();

        // Register SKSE Menu Framework section item and hooks
        void Register();

        // ImGui Render Callback for SKSE Menu Framework
        static void RenderMenu();

    private:
        MenuManager() = default;
        ~MenuManager() = default;
        MenuManager(const MenuManager&) = delete;
        MenuManager& operator=(const MenuManager&) = delete;

        bool _registered{ false };
    };
}
