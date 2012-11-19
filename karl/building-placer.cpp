// ToDo:
//  * Do not build into mineral line.

#include "building-placer.hpp"
#include "array2d.hpp"
#include "valuing.hpp"
#include "unit-observer.hpp"
#include "object-counter.hpp"
#include "container-helper.hpp"
#include "log.hpp"
#include <BWTA.h>
#include <limits>
#include <sstream>
#include <algorithm>

using namespace BWAPI;
#define THIS_DEBUG LOG

int mapWidth = 0;
int mapHeight = 0;
Array2d<TileInformation> tileInformations;

// Implementation is split to the following files:
#include "building-placer-power.hpp"
#include "building-placer-tileinfo.hpp"
#include "building-placer-internal.hpp"
#include "arrangement-impl.hpp"
#include "arrangement-generator.hpp"
#include "arrangement-conditions.hpp"
#include "arrangement-selector.hpp"
#include "bool-lambda-support.hpp"

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, const BWAPI::TilePosition& pos)
{
    if (pos == TilePositions::None)
        return NULL;

    return new BuildingPositionInternal(ut, pos);
}

Arrangement* defaultArrangement(const BWAPI::UnitType& type)
{
    using namespace ArrangementGenerator;
    using namespace ArrangementCondition;
    using namespace ArrangementSelector;
    
    BWTA::BaseLocation* base = BWTA::getStartLocation(BWAPI::Broodwar->self());
    //BWTA::Region* region = base->getRegion();
    BWAPI::TilePosition center = base->getTilePosition();

    if (type.requiresPsi()) {
        return createArrangement(
                    Spiral(center, 25),
                    CheckUnitTilesWithAddonWithSpace(type, 1, IsBuildable() && !IsBuilding() && !IsReserved()) && HasPower(type),
                    TakeFirst()
                );
    }
    
    if (type.requiresCreep()) {
        return createArrangement(
                    Spiral(center, 25),
                    CheckUnitTilesWithAddonWithSpace(type, 1, IsBuildable() && TileHasCreep() && !IsBuilding() && !IsReserved()),
                    TakeFirst()
                );
    }
    
    return createArrangement(
                    Spiral(center, 25),
                    CheckUnitTilesWithAddonWithSpace(type, 1, IsBuildable() && !IsBuilding() && !IsReserved()),
                    TakeFirst()
                );
}

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut)
{
    /*TilePosition base = BWTA::getStartLocation(Broodwar->self())->getTilePosition();
    TilePosition pos  = getBuildLocationNear(base, ut);*/
    return new BuildingPositionInternal(ut, defaultArrangement(ut), 0);
}

BuildingPositionPrecondition* getBuildingPosition(const BWAPI::UnitType& ut, std::set<BuildingPositionPrecondition*>& places)
{
    for (auto it : places)
        if (it->ut == ut)
    {
        places.erase(it);
        return it;
    }
    return getBuildingPosition(ut);
}

BuildingPositionPrecondition* getExpoPosition(const BWAPI::UnitType& ut, BWTA::BaseLocation* location)
{
    if (location == NULL)
        return NULL;

    return getBuildingPosition(ut, location->getTilePosition());
}

BuildingPositionPrecondition* getNextExpo(const BWAPI::UnitType& ut)
{
    BWTA::BaseLocation* home = BWTA::getStartLocation(Broodwar->self());
    BWTA::BaseLocation* bestlocation = NULL;
    ctype               bestvalue = std::numeric_limits<ctype>::min();

    for (auto it : BWTA::getBaseLocations())
        if (!it->isIsland())
            if (Broodwar->canBuildHere(NULL, it->getTilePosition(), ut, false))
    {
        ctype newvalue = valueExpo(it, home);
        if (newvalue > bestvalue) {
            bestlocation = it;
            bestvalue    = newvalue;
        }
    }

    return getExpoPosition(ut, bestlocation);
}

UnitPrecondition* registerRangeBuilding(UnitPrecondition* pre)
{
    if (pre->ut != UnitTypes::Protoss_Pylon)
        return pre;

    return new RangeBuildingObserver(pre);
}

namespace
{
    void setBuildingInfo(BWAPI::Unit* unit, bool value)
    {
        UnitType type = unit->getType();
        if (!type.isBuilding())
            return;
        TilePosition pos = unit->getTilePosition();
        
        //LOG << "setBuildingInfo(" << type << ", " << pos << ", " << (value ? "true" : "false") << ")";

        int upperX = std::min(pos.x() + type.tileWidth(), mapWidth);
        int upperY = std::min(pos.y() + type.tileHeight(), mapHeight);
        if (type.canBuildAddon())
            if (unit->getPlayer() == Broodwar->self())
                if (unit->getAddon() == NULL)
                    upperX = std::min(upperX + 2, mapWidth);
        
        for(int x=pos.x(); x<upperX; ++x)
            for(int y=pos.y(); y<upperY; ++y)
        {
            TileInformation& info = tileInformations[x][y];
            if (value) {
                info.building = unit;
            } else if (info.building == unit) {
                info.building = NULL;
            }
        }
    }
}

void BuildingPlacerCode::onMatchBegin()
{
    mapWidth  = Broodwar->mapWidth();
    mapHeight = Broodwar->mapHeight();
    tileInformations.resize(mapWidth, mapHeight);

    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
    {
        TileInformation& info = tileInformations[x][y];

        info.buildable = Broodwar->isBuildable(x, y);
        if (Broodwar->hasCreep(x, y))
            info.creeptime = 0;
        info.pre       = NULL;
        info.building  = NULL;
        for (int k=0; k<4; ++k)
            for (int l=0; l<4; ++l)
                info.subtiles[k][l] = Broodwar->isWalkable(4*x+k, 4*y+l);
    }
    
    for (auto it : Broodwar->getAllUnits())
        setBuildingInfo(it, true);

    /*
    for (auto it : BWTA::getBaseLocations())
    {
        reserveTiles(it->getTilePosition(), UnitTypes::Zerg_Hatchery, true);
    }
    */
}

void BuildingPlacerCode::onMatchEnd()
{
    rangebuildings.clear();
    planedbuildings.clear();
}

void BuildingPlacerCode::onTick()
{
    Containers::remove_if(rangebuildings, std::mem_fun(&RangeBuildingObserver::update));
    for (auto it : planedbuildings)
        it->update();
}

void BuildingPlacerCode::onUnitCreate(BWAPI::Unit* unit)
{
    setBuildingInfo(unit, true);
}

void BuildingPlacerCode::onUnitDestroy(BWAPI::Unit* unit)
{
    setBuildingInfo(unit, false);
}

void BuildingPlacerCode::onUnitMorph(BWAPI::Unit* unit)
{
    setBuildingInfo(unit, true);
}

void BuildingPlacerCode::onUnitShow(BWAPI::Unit* unit)
{
    setBuildingInfo(unit, true);
}

void BuildingPlacerCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    /*
    for (int x=0; x<mapWidth; ++x)
        for (int y=0; y<mapHeight; ++y)
    {
        TileInformation& info = tileInformations[x][y];
        if (info.energytime < Precondition::Max) {
            Broodwar->drawTextMap(32*x+16, 32*y+16, "\x08%d", info.energytime);
        }
    }
    */
}

void BuildingPlacerCode::onCheckMemoryLeaks()
{
    RangeBuildingObserver::checkObjectsAlive();
    BuildingPositionInternal::checkObjectsAlive();
}
