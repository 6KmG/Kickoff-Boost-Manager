#include "pch.h"
#include "BoostOnKickoff.h"

BAKKESMOD_PLUGIN(BoostOnKickoff, "Boost on Kickoff Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

// A plugin-level variable to hold the desired boost amount (as a percentage)
float kickoffBoostValue = 33.33f;
bool boostSetterEnabled = false; // Variable to toggle the boost functionality

void BoostOnKickoff::onLoad()
{
    _globalCvarManager = cvarManager;
    LOG("Boost On Kickoff plugin loaded!");
    DEBUGLOG("BoostOnKickoff debug mode enabled");

    // Register CVars
    cvarManager->registerCvar("cool_distance", "100.0", "Boost amount (percentage)", true, true, 0.0f, true, 100.0f, true);
    cvarManager->registerCvar("boost_setter_enabled", "1", "Enable or disable the boost setter", true, true);

    LOG("CVars registered!");

    // Hook into the kickoff event
    gameWrapper->HookEventWithCaller<ServerWrapper>(
        "Function GameEvent_Soccar_TA.Countdown.BeginState",
        [this](ServerWrapper server, void* caller, std::string newState) {
            if (boostSetterEnabled) {
                cvarManager->log("Kickoff event triggered, scheduling boost update...");
                gameWrapper->SetTimeout([this](GameWrapper* gw) {
                    setBoostForAll(kickoffBoostValue);
                    cvarManager->log("Boost set to " + std::to_string(kickoffBoostValue) + "% for all players");
                    }, 0.125);
            }
        });
}

void BoostOnKickoff::onUnload()
{
    LOG("Boost On Kickoff plugin unloaded!");
}

void BoostOnKickoff::RenderSettings() {
    ImGui::TextUnformatted("Boost On Kickoff Plugin Settings");

    // Checkbox to enable/disable boost setter functionality
    CVarWrapper boostSetterCvar = cvarManager->getCvar("boost_setter_enabled");
    if (boostSetterCvar) {
        boostSetterEnabled = boostSetterCvar.getBoolValue();
    }
    if (ImGui::Checkbox("Enable Boost Setter", &boostSetterEnabled)) {
        if (boostSetterCvar) {
            boostSetterCvar.setValue(boostSetterEnabled);
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Toggle the boost setter on kickoff");
    }

    // Slider for boost amount
    CVarWrapper distanceCvar = cvarManager->getCvar("cool_distance");
    if (distanceCvar) {
        kickoffBoostValue = distanceCvar.getFloatValue();
    }
    if (ImGui::SliderFloat("Boost Amount (%)", &kickoffBoostValue, 0.0f, 100.0f)) {
        if (distanceCvar) {
            distanceCvar.setValue(kickoffBoostValue);
            LOG(std::to_string(kickoffBoostValue));
        }
    }
    if (ImGui::IsItemHovered()) {
        std::string hoverText = "Set the boost amount to " + std::to_string(kickoffBoostValue) + "%";
        ImGui::SetTooltip(hoverText.c_str());
    }
}

void BoostOnKickoff::setBoostForAll(float boostAmount) {
    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) {
        cvarManager->log("No game state available!");
        return;
    }

    // Get all cars in the current game state
    auto cars = server.GetCars();
    for (CarWrapper car : cars) {
        if (car.IsNull())
            continue;

        auto boostComponent = car.GetBoostComponent();
        if (!boostComponent.IsNull()) {
            // Set boost amount; converting percentage to a decimal
            boostComponent.SetBoostAmount(boostAmount / 100.0f);
        }
    }
    cvarManager->log("Set boost to " + std::to_string(boostAmount) + "% for all players.");
}
