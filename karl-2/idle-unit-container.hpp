#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

UnitPrecondition* getIdleUnit(const BWAPI::UnitType& ut);
void rememberIdle(UnitPrecondition* unit);

struct IdleUnitContainerCode : public DefaultCode
{
	static void onMatchEnd();
	static void onTick();
	static void onUnitCreate(BWAPI::Unit* unit);
	static void onUnitDestroy(BWAPI::Unit* unit);
};
