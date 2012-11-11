#pragma once

#include <BWAPI.h>

BWAPI::UnitType getUnitTypeByName(const std::string& name);
BWAPI::Unit* getNearest(const std::set<BWAPI::Unit*>& units, const BWAPI::Position& p);
