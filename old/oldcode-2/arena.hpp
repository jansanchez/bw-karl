#pragma once

#include "default-code.hpp"

extern bool isArena;

struct ArenaCode : public DefaultCode
{
    static bool isApplyable();
    static void onMatchBegin();
    static void onMatchEnd();
    static void onTick();
    static bool onAssignUnit(BWAPI::Unit* unit);
};
