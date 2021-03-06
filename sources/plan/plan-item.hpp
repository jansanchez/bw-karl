#pragma once

#include "blackboard-informations.hpp"
#include "engine/event-visitor.hpp"

#include <vector>
#include <set>

class AbstractItem;
class AbstractAction;
class AbstractExecutionEngine;
class AbstractVisitor;
class Blackboard;
class AbstractExpert;

class AbstractPort : public AssertBase
{
    public:
        Time estimatedTime;

        AbstractPort(AbstractItem* o);

        inline AbstractItem* getOwner() const { return owner; }
        inline bool isImpossible() const { return isImpossibleTime(estimatedTime); }
        inline bool operator < (const AbstractPort& rhs) const { return estimatedTime < rhs.estimatedTime; }

        bool isActive() const;

        virtual ~AbstractPort();
        virtual bool isActiveConnection() const = 0;
        virtual bool isRequirePort() const = 0;
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;
        virtual void disconnect() = 0;
        virtual void updateEstimates() = 0;
        virtual AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);

    protected:
        AbstractItem*   owner;
};

class AbstractItem : public BasicEventVisitor, public AssertBase
{
    public:
        std::vector<AbstractPort*>  ports;

        AbstractItem(BWAPI::Unit* u = NULL);
        virtual ~AbstractItem();
        virtual void acceptVisitor(AbstractVisitor* visitor) = 0;

        bool isPortRegistered(AbstractPort* port);
        void removePort(AbstractPort* port);
        void update(AbstractEvent* event);

        bool isBoundaryItem() const;
        bool isPlanItem() const;

        inline BWAPI::Unit* getUnit() const { return unit; }
        void setUnit(BWAPI::Unit* u);

        ResourceCategorySet getCategory() const;

    protected:
        BWAPI::Unit* unit;
};

class AbstractBoundaryItem : public AbstractItem
{
    public:
        AbstractBoundaryItem(BWAPI::Unit* u);

        void takeConnectionsFrom(AbstractBoundaryItem* other);
};

class AbstractPlanItem : public AbstractItem
{
    public:
        enum Status { Planned, Active, Executing, Terminated, Finished, Failed };

        Time estimatedStartTime;
        AbstractExpert* creator;

        AbstractPlanItem();

        inline bool isPlanned() const { return (status == Planned); }
        inline bool isActive() const { return (status == Active) || (status == Executing) || (status == Terminated); }
        inline bool isFailed() const { return (status == Failed); }
        inline bool isImpossible() const { return isImpossibleTime(estimatedStartTime); }
        inline bool operator < (const AbstractPlanItem& rhs) const { return estimatedStartTime < rhs.estimatedStartTime; }
        inline Status getStatus() const { return status; }
        inline AbstractAction* getAction() const { return action; }

        void setActive();
        void setExecuting();
        void setTerminated(AbstractExecutionEngine* engine);
        void setFinished();
        void setErrorState(AbstractAction* action);
        void setPlanned();
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine);
        void addPurpose(AbstractPort* port, AbstractPlanItem* item);

        virtual void updateEstimates(Time current);
        virtual AbstractAction* buildAction() = 0;
        virtual void removeFinished(AbstractAction* action) = 0;

    protected:
        Status          status;
        AbstractAction* action;
};

class AbstractExpert : public AssertBase
{
    public:
        virtual ~AbstractExpert() = default;
        virtual void prepare() = 0;
        virtual bool tick(Blackboard* blackboard) = 0; // returns false if it should be removed.
        virtual void matchEnd(Blackboard* blackboard) = 0;
};

class ProvideUnitPort;

class ResourceBoundaryItem;
class EnemyUnitBoundaryItem;

class BuildPlanItem;
class MorphUnitPlanItem;
class GatherResourcesPlanItem;
class MoveToPositionPlanItem;
class AttackUnitPlanItem;
class AttackPositionPlanItem;
class GiveUpPlanItem;
class ResearchTechPlanItem;
class UpgradePlanItem;

class Blackboard : public BasicEventVisitor
{
    public:
        Blackboard(AbstractExecutionEngine* e);
        virtual ~Blackboard();

        static bool sendFrameEvent(AbstractExecutionEngine* engine);

        inline const std::vector<AbstractPlanItem*>& getItems() const { return items; }
        inline const std::map<BWAPI::Unit*, AbstractBoundaryItem*>& getBoundaries() const { return unitBoundaries; }
        inline BlackboardInformations* getInformations() { return &informations; }
        inline Time getLastUpdateTime() const { return informations.lastUpdateTime; }
        inline BWAPI::Player* self() const { return informations.self; }
        inline BWAPI::Player* neutral() const { return informations.neutral; }
        inline Time getActionHorizon() const { return informations.lastUpdateTime + 10; }

        void addItem(AbstractPlanItem* item);
        void removeItem(AbstractPlanItem* item);
        void terminate(AbstractPlanItem* item);

        void addExpert(AbstractExpert* expert);
        void removeExpert(AbstractExpert* expert);
        void setActiveExpert(AbstractExpert* expert);

        void prepare();
        void recalculateEstimatedTimes();
        void tick();
        void matchEnd();

        void visitActionEvent(ActionEvent* event) override;
        void visitFrameEvent(FrameEvent* event) override;
        void visitBroodwarEvent(BroodwarEvent* event) override;
        void visitUnitUpdateEvent(UnitUpdateEvent* event) override;
        void visitCompleteUnitUpdateEvent(CompleteUnitUpdateEvent* event) override;
        void visitFieldSeenEvent(FieldSeenEvent* event) override;
        void visitAbstractActionEvent(AbstractActionEvent* event) override;

        // for creation of plan items:
        AbstractPlanItem* create(BWAPI::UnitType ut, ResourceCategorySet c = ResourceCategorySet::all());
        BuildPlanItem* build(BWAPI::UnitType ut, ResourceCategorySet c = ResourceCategorySet::all());
        MorphUnitPlanItem* morph(BWAPI::UnitType ut, ResourceCategorySet c = ResourceCategorySet::all());
        GatherResourcesPlanItem* gather(ProvideUnitPort* provider, ResourceBoundaryItem* m);
        MoveToPositionPlanItem* move(ProvideUnitPort* provider, BWAPI::Position p);
        MoveToPositionPlanItem* move(ProvideUnitPort* provider, BWAPI::TilePosition tp);
        AttackUnitPlanItem* attack(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy);
        AttackPositionPlanItem* attack(ProvideUnitPort* provider, BWAPI::Position p);
        ResearchTechPlanItem* research(BWAPI::TechType tech, ResourceCategorySet c = ResourceCategorySet::all());
        UpgradePlanItem* upgrade(BWAPI::UpgradeType upgrade, ResourceCategorySet c = ResourceCategorySet::all());
        GiveUpPlanItem* giveUp();

        // for test propose only:
        bool includesItem(AbstractPlanItem* item) const;
        AbstractBoundaryItem* lookupUnit(BWAPI::Unit* unit) const;

    protected:
        AbstractExecutionEngine*                        engine;
        std::vector<AbstractPlanItem*>                  items;
        std::vector<AbstractExpert*>                    experts;
        BlackboardInformations                          informations;
        std::map<BWAPI::Unit*, AbstractBoundaryItem*>   unitBoundaries;
        std::map<AbstractAction*, AbstractPlanItem*>    actionMap;
        AbstractExpert*                                 activeExpert;
};
