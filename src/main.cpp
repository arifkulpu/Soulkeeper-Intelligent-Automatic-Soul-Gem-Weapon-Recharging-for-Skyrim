#include "PCH.h"
#include "SoulkeeperManager.h"
#include "Settings.h"
#include "PassiveRechargeManager.h"

// Define SKSE Plugin Version Data for Skyrim SE (1.5.97), AE (1.6.x - 1.7.104+), and VR (1.4.15)
SKSEPluginInfo(
    .Version = REL::Version{ 1, 0, 0, 0 },
    .Name = "Soulkeeper",
    .Author = "Soulkeeper Developer",
    .SupportEmail = "",
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
    .MinimumSKSEVersion = REL::Version{ 0, 0, 0, 0 }
)

namespace
{
    void InitializeLog()
    {
#ifndef NDEBUG
        auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
        auto path = logger::log_directory();
        if (!path) {
            return;
        }

        *path /= "Soulkeeper.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

        auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
        log->set_level(spdlog::level::trace);
#else
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
#endif

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
    }

    void OnMessage(SKSE::MessagingInterface::Message* a_msg)
    {
        switch (a_msg->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            Soulkeeper::SoulkeeperManager::GetSingleton()->Initialize();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            Soulkeeper::Settings::GetSingleton()->Load();
            Soulkeeper::PassiveRechargeManager::GetSingleton()->ResetGameTimeTracker();
            break;
        default:
            break;
        }
    }

    // Hook PlayerCharacter::Update (index 0xAD / 173) via VTABLE
    struct PlayerUpdateHook
    {
        static void Install()
        {
            auto vtbl = RE::PlayerCharacter::VTABLE[0].address();
            auto* target = reinterpret_cast<uintptr_t*>(vtbl + (0xAD * sizeof(uintptr_t)));

            // 1. Read original function pointer
            uintptr_t originalFunc = *target;
            _originalUpdate = reinterpret_cast<decltype(Update)*>(originalFunc);

            // 2. Safe write our detour into the vtable
            REL::safe_write(reinterpret_cast<std::uintptr_t>(target), reinterpret_cast<std::uintptr_t>(Update));

            logger::info("PlayerCharacter::Update hook installed successfully via VTABLE.");
        }

        static void Update(RE::PlayerCharacter* a_this, float a_delta)
        {
            if (_originalUpdate) {
                _originalUpdate(a_this, a_delta);
            }
            Soulkeeper::SoulkeeperManager::GetSingleton()->Update(a_delta);
        }

        inline static decltype(Update)* _originalUpdate{ nullptr };
    };
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    InitializeLog();
    logger::info("Soulkeeper v1.0.0 initializing for Skyrim SE/AE/VR (Universal)...");

    SKSE::Init(a_skse);

    // Immediately load/create Soulkeeper.ini on plugin startup
    Soulkeeper::Settings::GetSingleton()->Load();

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(OnMessage)) {
        logger::critical("Failed to register SKSE messaging listener.");
        return false;
    }

    SKSE::AllocTrampoline(14);
    PlayerUpdateHook::Install();

    logger::info("Soulkeeper loaded successfully.");
    return true;
}
