#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"

struct Production
{
	int time;		// starting time.
	int minerals;   // in 1/MineralFactor mineral per frame.
	int gas;		// in 1/MineralFactor gas per frame.
};

const int MineralFactor = 1000;

extern std::vector<Production> estimatedProduction; // sorted by time.

void useWorker(UnitPrecondition* unit);
UnitPrecondition* getWorker(const BWAPI::Race& r);

struct MineralLineCode : public DefaultCode
{
	static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onUnitDestroy(BWAPI::Unit* unit);
};
