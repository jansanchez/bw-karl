#pragma once

#include "abstract-action.hpp"
#include "utils/time.hpp"
#include "utils/bw-helper.hpp"

class FrameEvent : public AbstractEvent
{
    public:
        Time    currentTime;
        int     currentMinerals;
        int     currentGas;
        int     collectedMinerals;
        int     collectedGas;

        FrameEvent(Time t, int m, int g, int cm, int cg);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class UnitUpdateEvent : public AbstractEvent
{
    public:
        BWAPI::Unit*    unit;

        UnitUpdateEvent(BWAPI::Unit* u);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class SimpleUnitUpdateEvent : public UnitUpdateEvent
{
    public:
        BWAPI::Position pos;
        int             health;
        BWAction::Type  currentAction;

        SimpleUnitUpdateEvent(BWAPI::Unit* u, BWAPI::Position p, int h, BWAction::Type a);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class CompleteUnitUpdateEvent : public SimpleUnitUpdateEvent
{
    public:
        BWAPI::UnitType     unitType;
        BWAPI::Player*      owner;
        BWAPI::TilePosition tilePos;
        BWAPI::Unit*        hatchery; // only for larva!

        CompleteUnitUpdateEvent(BWAPI::Unit* u, BWAPI::UnitType t, int h, BWAction::Type a, const BWAPI::TilePosition& tp, const BWAPI::Position& p, BWAPI::Player* o, BWAPI::Unit* hatch = NULL);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class ResourceUpdateEvent : public UnitUpdateEvent
{
    public:
        int resources;

        ResourceUpdateEvent(BWAPI::Unit* u, int res);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class BroodwarEvent : public AbstractEvent
{
    public:
        BWAPI::Event    event;

        BroodwarEvent(const BWAPI::Event& e);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class FieldSeenEvent : public AbstractEvent
{
    public:
        BWAPI::TilePosition tilePos;
        bool creep;

        FieldSeenEvent(const BWAPI::TilePosition& tp, bool c);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};

class AbstractActionEvent : public AbstractEvent
{
    public:
        AbstractAction* sender;

        AbstractActionEvent(AbstractAction* a);
};

class ResourcesConsumedEvent : public AbstractActionEvent
{
    public:
        ResourcesConsumedEvent(AbstractAction* a);
        void acceptVisitor(AbstractEventVisitor* visitor) override;
};
