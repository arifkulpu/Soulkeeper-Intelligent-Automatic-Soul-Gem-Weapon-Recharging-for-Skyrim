#pragma once

#include "PCH.h"

namespace Soulkeeper
{
    class PlayerManager
    {
    public:
        static PlayerManager* GetSingleton();

        // Process player's equipped weapon charge check
        void ProcessPlayer();

    private:
        PlayerManager() = default;
        ~PlayerManager() = default;
    };
}
