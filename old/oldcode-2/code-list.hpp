#pragma once

#include "code-caller.hpp"
#include "exception-handler.hpp"
#include "log.hpp"
#include "message-dispatch.hpp"
#include "mineral-line.hpp"
#include "resources.hpp"
#include "supply.hpp"
#include "building-placer.hpp"
#include "idle-unit-container.hpp"
#include "unit-morpher.hpp"
#include "unit-builder.hpp"
#include "unit-trainer.hpp"
#include "terran-strategie.hpp"
#include "protoss-strategie.hpp"
#include "zerg-strategie.hpp"
#include "hud-code.hpp"
#include "larvas.hpp"
#include "requirements.hpp"
#include "tech-upgrades.hpp"
#include "terran-marines-code.hpp"
#include "squad.hpp"
#include "unit-micromanagement.hpp"
#include "scout.hpp"
#include "strategie-list.hpp"
#include "arena.hpp"
#include "valuing.hpp"
#include "addon-builder.hpp"
#include "building-flyer.hpp"
#include "wall-in.hpp"
#include "information-collector.hpp"
#include "dual-graph.hpp"

typedef CodeCaller<
                    ExceptionHandlerCode,
                    LogCode,
                    ValuingCode,
                    MineralLineCode,
                    ResourcesCode,
                    LarvaCode,                  // before MessageDispatchCode!!!
                    MessageDispatchCode,
                    UnitMicromanagementCode,    // before other Code that uses units
                    BuildingPlacerCode,
                    DualGraphCode,
                    InformationCode,
                    IdleUnitContainerCode,
                    RequirementsCode,
                    UnitBuilderCode,
                    UnitMorpherCode,
                    UnitTrainerCode,
                    TechCode,
                    AddonBuilderCode,
                    BuildingFlyerCode,
                    TerranMarinesCode,
                    SquadCode,
                    SupplyCode,
                    HUDCode,
                    ScoutCode,
                    WallInCode,
                    StrategieList<
                        TerranStrategieCode,
                        ProtossStrategieCode,
                        ZergStrategieCode,
                        ArenaCode
                    >
                  >
        CodeList;
