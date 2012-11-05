#pragma once

#include "default-code.hpp"
#include "bwapi-precondition.hpp"



struct TerranMarinesCode : public DefaultCode
{
    static void onMatchBegin();
	static void onMatchEnd();
	static void onTick();
	static void onDrawPlan();
	static void onCheckMemoryLeaks();
};


void makeSomethingUsefulWithInfantry(UnitPrecondition *u);
