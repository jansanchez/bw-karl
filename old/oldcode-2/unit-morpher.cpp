// ToDo:
//  *

#include "unit-morpher.hpp"
#include "precondition-helper.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "idle-unit-container.hpp"
#include "requirements.hpp"
#include "container-helper.hpp"
#include "object-counter.hpp"
#include "log.hpp"
#include <BWAPI.h>
#include <vector>
#include <algorithm>

using namespace BWAPI;

namespace
{
    struct UnitMorphPrecondition;
    std::vector<UnitMorphPrecondition*> list;
    
    struct UnitMorphPrecondition : public UnitPrecondition, public ObjectCounter<UnitMorphPrecondition>
    {
        enum StatusType { pending, tryagain, commanded, waiting, finished };

        UnitPrecondition*           baseunit;
        ResourcesPrecondition*      resources;
        SupplyPrecondition*         supply;
        RequirementsPrecondition*   requirements;
        Precondition*               extra;
        StatusType                  status;

        UnitMorphPrecondition(UnitPrecondition* u, ResourcesPrecondition* r, SupplyPrecondition* s, RequirementsPrecondition* v, 
                              const UnitType& ut, Precondition* e)
            : UnitPrecondition(ut, u->pos, u->unit, UnitPrecondition::WithoutAddon),
              baseunit(u), resources(r), supply(s), requirements(v), extra(e), status(pending)
        {
            updateTime();
        }
        
        ~UnitMorphPrecondition()
        {
            Containers::remove(list, this);

            release(baseunit);
            release(resources);
            release(supply);
            release(requirements);
            release(extra);
        }

        bool updateTime()
        {
            switch (status)
            {
                case pending:
                    if (baseunit != NULL)
                        baseunit->wishpos = wishpos;
                    if (updateTimePreconditions(this, ut.buildTime(), baseunit, resources, supply, requirements, extra)) {
                        start();
                        time = Broodwar->getFrameCount() + ut.buildTime();
                    }
                    break;
                
                case tryagain:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    start();
                    break;

                case commanded:
                    time = Broodwar->getFrameCount() + ut.buildTime();
                    if (unit->isMorphing()) {
                        freeResources();
                        status = waiting;
                    }
                    break;

                case waiting:
                    if (!unit->isMorphing()) {
                        time   = 0;
                        status = finished;
                    }
                    break;
                
                case finished:
                    break;
            }
            return (status == finished);
        }

        void start()
        {
            if (baseunit != NULL) {
                unit = baseunit->unit;
                release(baseunit);
            }
            assert(unit != NULL);
            if (!unit->morph(ut)) {
                Error err = Broodwar->getLastError();
                WARNING << "Unable to morph unit " << ut << ": " << err;
                if (   (err == Errors::Insufficient_Supply)
                    || (err == Errors::Insufficient_Minerals)
                    || (err == Errors::Insufficient_Gas))
                {
                    status = pending;
                    return;
                } else if (err == Errors::Incompatible_UnitType) {
                    rememberIdle(unit);
                    baseunit = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WhatEver);
                    status = pending;
                    return;
                } else if (err == Errors::Unit_Not_Owned) {
                    unit = NULL;
                    baseunit = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WhatEver);
                    status = pending;
                    return;
                }
                
                status = tryagain;
                return;
            }
            
            status = commanded;
        }

        void freeResources()
        {
            release(resources);
            release(supply);
            release(requirements);
            release(extra);
        }
        
        void onDrawPlan()
        {
            if (status == pending)
                return;
            
            assert(unit != NULL);
            Position pos = unit->getPosition();
            int x = pos.x(), y = pos.y();
            Broodwar->drawCircleMap(x, y, 20, Colors::Green, false);
        }
    };
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, SupplyPrecondition* supply, const BWAPI::UnitType& ut, Precondition* extra)
{
    // supply maybe NULL.
    if ((unit == NULL) || (res == NULL)) {
        release(unit);
        release(res);
        release(supply);
        release(extra);
        return NULL;
    }
    
    RequirementsPrecondition* req = getRequirements(ut);
    // req maybe NULL.
    
    UnitMorphPrecondition* result = new UnitMorphPrecondition(unit, res, supply, req, ut, extra);
    list.push_back(result);

    UnitPrecondition* first = result;
    first = registerSupplyUnit(first);
    first = registerRequirement(first);
    return first;
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, ResourcesPrecondition* res, const BWAPI::UnitType& ut, Precondition* extra)
{
    SupplyPrecondition* supply = getSupply(ut);
    return morphUnit(unit, res, supply, ut, extra);
}

UnitPrecondition* morphUnit(UnitPrecondition* unit, const BWAPI::UnitType& ut, Precondition* extra)
{
    ResourcesPrecondition* res = getResources(ut);
    return morphUnit(unit, res, ut, extra);
}

UnitPrecondition* morphUnit(const BWAPI::UnitType& ut, Precondition* extra)
{
    UnitPrecondition* unit = getIdleUnit(ut.whatBuilds().first, UnitPrecondition::WhatEver);
    return morphUnit(unit, ut, extra);
}

void morphWorker(const BWAPI::UnitType& ut)
{
    useWorker(morphUnit(ut));
}

void morphUnitEx(const BWAPI::UnitType& ut)
{
    rememberIdle(morphUnit(ut));
}

void UnitMorpherCode::onMatchEnd()
{
    list.clear();
}

void UnitMorpherCode::onTick()
{
    Containers::remove_if(list, std::mem_fun(&UnitMorphPrecondition::updateTime));
}

void UnitMorpherCode::onDrawPlan(HUDTextOutput& /*hud*/)
{
    for (auto it : list)
        it->onDrawPlan();
}

void UnitMorpherCode::onCheckMemoryLeaks()
{
    UnitMorphPrecondition::checkObjectsAlive();
}
