#pragma once

#include "precondition.hpp"
#include <BWAPI.h>

struct ResourcesPrecondition : public Precondition
{
    int minerals;
    int gas;
	
	ResourcesPrecondition(int t, int m, int g)
		: Precondition(t), minerals(m), gas(g)
	{ }
};

struct SupplyPrecondition : public Precondition
{
	BWAPI::Race 	race;
	int 			supply;
	
	SupplyPrecondition(const BWAPI::Race& r, int s)
		: race(r), supply(s)
	{ }
};

struct BuildingPositionPrecondition : public Precondition
{
	BWAPI::UnitType		ut;
	BWAPI::TilePosition	pos;

	BuildingPositionPrecondition(const BWAPI::UnitType& t, const BWAPI::TilePosition& p)
		: Precondition(0), ut(t), pos(p)
	{ }
    
    bool isExplored() const
    {
        int x = pos.x(), y = pos.y();
        for (int dx=0; dx<ut.tileWidth(); ++dx)
            for (int dy=0; dy<ut.tileHeight(); ++dy)
                if (!BWAPI::Broodwar->isExplored(x+dx, y+dy))
                    return false;
        return true;
    }
};

struct RequirementsPrecondition : public Precondition
{
	BWAPI::UnitType		ut;
	BWAPI::UpgradeType	gt;
	int level;
	
	RequirementsPrecondition(const BWAPI::UnitType& t)
		: Precondition(0), ut(t), gt(BWAPI::UpgradeTypes::None), level(0)
	{ }
	
	RequirementsPrecondition(const BWAPI::UpgradeType& t, int l)
		: Precondition(0), ut(BWAPI::UnitTypes::None), gt(t), level(l)
	{ }
};

struct UnitPrecondition : public Precondition
{
    BWAPI::UnitType     ut;
    BWAPI::Position     pos;
    BWAPI::Unit*        unit;

	UnitPrecondition()
		: Precondition(0), unit(NULL)
	{ }

    explicit UnitPrecondition(BWAPI::Unit* u)
		: Precondition(0), ut(u->getType()), pos(u->getPosition()), unit(u)
	{ }
	
	UnitPrecondition(const BWAPI::UnitType& t, const BWAPI::Position& p, BWAPI::Unit* u)
        : Precondition(0), ut(t), pos(p), unit(u)
    { }
	
	UnitPrecondition(int time, const BWAPI::UnitType& t, const BWAPI::Position& p)
        : Precondition(time), ut(t), pos(p), unit(NULL)
    { }
};
