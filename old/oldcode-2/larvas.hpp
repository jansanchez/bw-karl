#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* getLarva();
void registerHatchery(BWAPI::Unit* u);
UnitPrecondition* registerHatchery(UnitPrecondition* hatch);
void registerLarva(BWAPI::Unit* u);

int nextFreeLarvaTime();

struct LarvaCode : public DefaultCode
{
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static bool onAssignUnit(BWAPI::Unit* unit);
    static void onDrawPlan(HUDTextOutput& hud);
    static void onCheckMemoryLeaks(); 
};
