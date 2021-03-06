#pragma once

#include "basic-port-impl.hpp"
#include <BWAPI.h>

class AbstractAction;
class ProvideUnitPort;
class RequireUnitPort;
class ProvideResourcePort;
class RequireResourcePort;

class ProvideUnitPort final : public BasicPortImpl<ProvideUnitPort, RequireUnitPort, false, false>
{
    public:
        ProvideUnitPort(AbstractItem* o, bool od = false);

        void updateData(BWAPI::UnitType ut, BWAPI::Position p);
        void updateData(RequireUnitPort* port);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        BWAPI::Unit* getUnit() const;
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return pos; }
        inline bool isOnDemand() const { return onDemand; }

    protected:
        friend class RequireUnitPort;

        BWAPI::UnitType     unitType;
        BWAPI::Position     pos;
        bool                onDemand;
};

class RequireUnitPort final : public BasicPortImpl<RequireUnitPort, ProvideUnitPort, true, false>
{
    public:
        RequireUnitPort(AbstractItem* o, BWAPI::UnitType ut);

        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        void bridge(ProvideUnitPort* port);

        inline BWAPI::Unit* getUnit() const { return (connection != NULL) ? connection->getUnit() : NULL; }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline BWAPI::Position getPosition() const { return (connection != NULL) ? connection->pos : BWAPI::Positions::Unknown; }
        inline ResourceCategorySet getCategory() const { return owner->getCategory(); }

    protected:
        BWAPI::UnitType     unitType;
};

class ResourcePort final : public AbstractPort
{
    public:
        ResourcePort(AbstractItem* o, BlackboardInformations* i, int m, int g, ResourceCategorySet c);

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;
        void disconnect() override;
        void updateEstimates() override;
        void resourcesConsumed();

        inline ResourceCategorySet getCategory() const { return category; }
        inline int getMinerals() const { return minerals; }
        inline int getGas() const { return gas; }

    protected:
        BlackboardInformations* info;
        ResourceCategorySet     category;
        int                     minerals;
        int                     gas;
};

class SupplyPort final : public AbstractPort
{
    public:
        int estimatedDuration;

        SupplyPort(AbstractItem* o, BWAPI::UnitType ut, bool checkTwoInOneEgg = false);

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;
        void disconnect() override;
        void updateEstimates() override;

        void updateUnitType(BWAPI::UnitType ut);

        inline BWAPI::Race getRace() const { return race; }
        inline int getProvidedAmount() const { return providedAmount; }
        inline int getRequiredAmount() const { return -providedAmount; }
        inline bool isConnected() const { return !isImpossibleTime(estimatedTime); }

    protected:
        BWAPI::Race race;
        int         providedAmount;
};

class ResourceBoundaryItem;

class ProvideResourcePort final : public BasicPortImpl<ProvideResourcePort, RequireResourcePort, false, true>
{
    public:
        ProvideResourcePort(ResourceBoundaryItem* o);
        void acceptVisitor(AbstractVisitor* visitor) override;

        BWAPI::Unit* getUnit() const;
        ResourceBoundaryItem* getOwner() const;
        bool isMineralField() const;
};

class RequireResourcePort final : public BasicPortImpl<RequireResourcePort, ProvideResourcePort, true, false>
{
    public:
        RequireResourcePort(AbstractItem* o, ResourceBoundaryItem* m);
        void acceptVisitor(AbstractVisitor* visitor) override;

        inline BWAPI::Unit* getUnit() const { return connection->getUnit(); }
        inline bool isMineralField() const { return (connection != NULL) && connection->isMineralField(); }
};

class RequireSpacePort final : public AbstractPort
{
    public:
        RequireSpacePort(AbstractItem* o, BlackboardInformations* i, BWAPI::UnitType ut, BWAPI::TilePosition p = BWAPI::TilePositions::Unknown);
        ~RequireSpacePort();

        bool isRequirePort() const override;
        bool isActiveConnection() const override;
        void acceptVisitor(AbstractVisitor* visitor) override;

        void updateEstimates() override;
        void disconnect() override;
        void connectTo(BWAPI::TilePosition tp);
        void setUnitType(BWAPI::UnitType ut);

        inline const BWAPI::TilePosition& getTilePosition() const { return pos; }
        inline int getWidth() const { return unitType.tileWidth(); }
        inline int getHeight() const { return unitType.tileHeight(); }
        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline bool isConnected() const { return (pos.x() < 1000); }

    protected:
        BlackboardInformations*     info;
        BWAPI::TilePosition         pos;
        BWAPI::UnitType             unitType;
};


class RequireEnemyUnitPort;
class EnemyUnitBoundaryItem;

class ProvideEnemyUnitPort final : public BasicPortImpl<ProvideEnemyUnitPort, RequireEnemyUnitPort, false, true>
{
    public:
        ProvideEnemyUnitPort(EnemyUnitBoundaryItem* o);
        void acceptVisitor(AbstractVisitor* visitor) override;

        BWAPI::Unit* getUnit() const;
        EnemyUnitBoundaryItem* getOwner() const;
        BWAPI::Position getPosition() const;
};


class RequireEnemyUnitPort final : public BasicPortImpl<RequireEnemyUnitPort, ProvideEnemyUnitPort, true, false>
{
    public:
        RequireEnemyUnitPort(AbstractItem* o, EnemyUnitBoundaryItem* m);
        void acceptVisitor(AbstractVisitor* visitor) override;

        inline BWAPI::Unit* getUnit() const { return connection->getUnit(); }
        inline BWAPI::Position getPosition() const { return connection->getPosition();}
};

class RequireUnitExistancePort;

class ProvideUnitExistancePort final : public BasicPortImpl<ProvideUnitExistancePort, RequireUnitExistancePort, false, true>
{
    public:
        ProvideUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut);
        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        inline BWAPI::UnitType getUnitType() const { return unitType; }

    protected:
        BWAPI::UnitType unitType;
};

class RequireUnitExistancePort final : public BasicPortImpl<RequireUnitExistancePort, ProvideUnitExistancePort, true, false>
{
    public:
        RequireUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut);
        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
        ProvideUnitExistancePort* connectTo(AbstractItem* provider);
        using BaseClass::connectTo;

        inline BWAPI::UnitType getUnitType() const { return unitType; }
        inline ResourceCategorySet getCategory() const { return owner->getCategory(); }

    protected:
        BWAPI::UnitType unitType;
};

class RequirePurposePort;

class ProvidePurposePort final : public BasicPortImpl<ProvidePurposePort, RequirePurposePort, false, true>
{
    public:
        ProvidePurposePort(AbstractPlanItem* o);
        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;

        inline AbstractPlanItem* getOwner() const { return static_cast<AbstractPlanItem*>(BaseClass::getOwner()); }
};

class RequirePurposePort final : public BasicPortImpl<RequirePurposePort, ProvidePurposePort, true, false>
{
    public:
        RequirePurposePort(AbstractPlanItem* o, AbstractPort* p);
        void acceptVisitor(AbstractVisitor* visitor) override;
        AbstractAction* prepareForExecution(AbstractExecutionEngine* engine) override;
        void updateEstimates() override;
        void connectTo(AbstractPlanItem* provider);
        using BaseClass::connectTo;

        inline AbstractPlanItem* getOwner() const { return static_cast<AbstractPlanItem*>(BaseClass::getOwner()); }

    protected:
        AbstractPort* providePort;
};
