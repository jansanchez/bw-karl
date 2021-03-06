#include "broodwar-ports.hpp"
#include "abstract-visitor.hpp"
#include "broodwar-boundary-items.hpp"
#include "blackboard-informations.hpp"

ProvideUnitPort::ProvideUnitPort(AbstractItem* o, bool od)
    : BaseClass(o), unitType(BWAPI::UnitTypes::Unknown), pos(BWAPI::Positions::Unknown), onDemand(od)
{ }

void ProvideUnitPort::updateData(BWAPI::UnitType ut, BWAPI::Position p)
{
    unitType    = ut;
    pos         = p;
}

void ProvideUnitPort::updateData(RequireUnitPort* port)
{
    unitType    = port->getUnitType();
    pos         = port->getPosition();
}

void ProvideUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideUnitPort(this);
}

AbstractAction* ProvideUnitPort::prepareForExecution(AbstractExecutionEngine* engine)
{
    auto planItem = dynamic_cast<AbstractPlanItem*>(owner);
    if (planItem == NULL)
        return NULL;
    if (onDemand)
        planItem->setTerminated(engine);
    return planItem->getAction();
}

BWAPI::Unit* ProvideUnitPort::getUnit() const
{
    return owner->getUnit();
}

RequireUnitPort::RequireUnitPort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{ }

void RequireUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireUnitPort(this);
}

void RequireUnitPort::bridge(ProvideUnitPort* port)
{
    if (connection != NULL)
        connection->connectTo(port->connection);
}

AbstractAction* RequireUnitPort::prepareForExecution(AbstractExecutionEngine* engine)
{
    return (connection != NULL) ? connection->prepareForExecution(engine) : NULL;
}

ResourcePort::ResourcePort(AbstractItem* o, BlackboardInformations* i, int m, int g, ResourceCategorySet c)
    : AbstractPort(o), info(i), category(c), minerals(m), gas(g)
{ }

bool ResourcePort::isRequirePort() const
{
    return true;
}

bool ResourcePort::isActiveConnection() const
{
    return false;
}

void ResourcePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResourcePort(this);
}

void ResourcePort::disconnect()
{
    estimatedTime = INFINITE_TIME;
}

void ResourcePort::updateEstimates()
{
    // do nothing, ResourceExpert updates estimates!!!
}

void ResourcePort::resourcesConsumed()
{
    owner->removePort(this);
    int sum = minerals + gas;
    for (auto& it : info->resourceCategories)
        it.amount -= it.ratio * sum;
}

SupplyPort::SupplyPort(AbstractItem* o, BWAPI::UnitType ut, bool checkTwoInOneEgg)
    : AbstractPort(o), estimatedDuration(0), race(ut.getRace()), providedAmount(ut.supplyProvided() - ut.supplyRequired())
{
    if (checkTwoInOneEgg && ut.isTwoUnitsInOneEgg())
        providedAmount *= 2;
    if (providedAmount == 0)
        owner->removePort(this);
}

void SupplyPort::updateUnitType(BWAPI::UnitType ut)
{
    int newAmount = ut.supplyProvided() - ut.supplyRequired();
    if (newAmount != providedAmount) {
        if (providedAmount == 0)
            owner->ports.push_back(this);
        providedAmount = newAmount;
        if (providedAmount == 0)
            owner->removePort(this);
    }
}

bool SupplyPort::isRequirePort() const
{
    return (providedAmount < 0);
}

bool SupplyPort::isActiveConnection() const
{
    return false;
}

void SupplyPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitSupplyPort(this);
}

void SupplyPort::disconnect()
{
    estimatedTime = INFINITE_TIME;
}

void SupplyPort::updateEstimates()
{
    if (isRequirePort()) {
        // do nothing, require ports are updated by SupplyExpert!!!
    } else {
        // Remark this method is only called if owner is of type AbstractPlanItem!!!
        estimatedTime = static_cast<AbstractPlanItem*>(owner)->estimatedStartTime + estimatedDuration;
    }
}

ProvideResourcePort::ProvideResourcePort(ResourceBoundaryItem* o)
    : BaseClass(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideResourcePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideResourcePort(this);
}

BWAPI::Unit* ProvideResourcePort::getUnit() const
{
    return getOwner()->getUnit();
}

ResourceBoundaryItem* ProvideResourcePort::getOwner() const
{
    return static_cast<ResourceBoundaryItem*>(owner);
}

bool ProvideResourcePort::isMineralField() const
{
    return getOwner()->getUnitType().isMineralField();
}

RequireResourcePort::RequireResourcePort(AbstractItem* o, ResourceBoundaryItem* m)
    : BaseClass(o)
{
    connectTo(new ProvideResourcePort(m));
}

void RequireResourcePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireResourcePort(this);
}

RequireSpacePort::RequireSpacePort(AbstractItem* o, BlackboardInformations* i, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractPort(o), info(i), pos(BWAPI::TilePositions::Unknown), unitType(ut)
{
    connectTo(p);
}

RequireSpacePort::~RequireSpacePort()
{
    disconnect();
}

bool RequireSpacePort::isRequirePort() const
{
    return true;
}

bool RequireSpacePort::isActiveConnection() const
{
    return isActive();
}

void RequireSpacePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireSpacePort(this);
}

void RequireSpacePort::updateEstimates()
{
    estimatedTime = (isConnected()) ? ACTIVE_TIME : INFINITE_TIME;
}

void RequireSpacePort::disconnect()
{
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+getWidth(); ++x)
            for (int y=pos.y(); y<pos.y()+getHeight(); ++y)
                info->fields[x][y].blocker = NULL;
        pos = BWAPI::TilePositions::Unknown;
    }
}

void RequireSpacePort::setUnitType(BWAPI::UnitType ut)
{
    BWAPI::TilePosition oldpos = pos;
    disconnect();
    unitType = ut;
    connectTo(oldpos);
}

void RequireSpacePort::connectTo(BWAPI::TilePosition tp)
{
    if (pos == tp)
        return;

    if (isConnected())
        disconnect();
    pos = tp;
    if (isConnected()) {
        for (int x=pos.x(); x<pos.x()+getWidth(); ++x)
            for (int y=pos.y(); y<pos.y()+getHeight(); ++y)
        {
            auto& field = info->fields[x][y];
            if (field.blocker != NULL)
                field.blocker->disconnect();
            field.blocker = this;
        }
    }
}


// EnemyUnit
ProvideEnemyUnitPort::ProvideEnemyUnitPort(EnemyUnitBoundaryItem* o)
    : BaseClass(o)
{
    estimatedTime = ACTIVE_TIME;
}

void ProvideEnemyUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideEnemyUnitPort(this);
}

BWAPI::Unit* ProvideEnemyUnitPort::getUnit() const
{
    return getOwner()->getUnit();
}

BWAPI::Position ProvideEnemyUnitPort::getPosition() const
{
    return getOwner()->getPosition();
}

EnemyUnitBoundaryItem* ProvideEnemyUnitPort::getOwner() const
{
    return static_cast<EnemyUnitBoundaryItem*>(owner);
}

RequireEnemyUnitPort::RequireEnemyUnitPort(AbstractItem* o, EnemyUnitBoundaryItem* enemy)
    : BaseClass(o)
{
    connectTo(new ProvideEnemyUnitPort(enemy));
}

void RequireEnemyUnitPort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireEnemyUnitPort(this);
}

ProvideUnitExistancePort::ProvideUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{
    if (owner->isPlanItem()) {
        estimatedDuration = ut.buildTime();
    } else if (owner->isBoundaryItem()) {
        estimatedTime = ACTIVE_TIME;
    }
}

void ProvideUnitExistancePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvideUnitExistancePort(this);
}

AbstractAction* ProvideUnitExistancePort::prepareForExecution(AbstractExecutionEngine* /*engine*/)
{
    auto planitem = dynamic_cast<AbstractPlanItem*>(owner);
    return (planitem != NULL) ? planitem->getAction() : NULL;
}

RequireUnitExistancePort::RequireUnitExistancePort(AbstractItem* o, BWAPI::UnitType ut)
    : BaseClass(o), unitType(ut)
{ }

void RequireUnitExistancePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequireUnitExistancePort(this);
}

ProvideUnitExistancePort* RequireUnitExistancePort::connectTo(AbstractItem* provider)
{
    if (isConnected() && (getConnectedPort()->getOwner() == provider))
        return getConnectedPort();
    auto result = new ProvideUnitExistancePort(provider, unitType);
    BaseClass::connectTo(result);
    return result;
}

AbstractAction* RequireUnitExistancePort::prepareForExecution(AbstractExecutionEngine* engine)
{
    return (connection != NULL) ? connection->prepareForExecution(engine) : NULL;
}


ProvidePurposePort::ProvidePurposePort(AbstractPlanItem* o)
    : BaseClass(o)
{
    estimatedDuration = 0;
}

void ProvidePurposePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitProvidePurposePort(this);
}

AbstractAction* ProvidePurposePort::prepareForExecution(AbstractExecutionEngine* /*engine*/)
{
    return NULL;
}


RequirePurposePort::RequirePurposePort(AbstractPlanItem* o, AbstractPort* p)
    : BaseClass(o), providePort(p)
{ }

void RequirePurposePort::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitRequirePurposePort(this);
}

AbstractAction* RequirePurposePort::prepareForExecution(AbstractExecutionEngine* /*engine*/)
{
    return NULL;
}

void RequirePurposePort::updateEstimates()
{
    if (!isConnected() || !owner->isPortRegistered(providePort)) {
        estimatedTime = INFINITE_TIME;
        return;
    }
    estimatedDuration = getOwner()->estimatedStartTime - providePort->estimatedTime;
    estimatedTime     = getConnectedPort()->getOwner()->estimatedStartTime + estimatedDuration;
}

void RequirePurposePort::connectTo(AbstractPlanItem* provider)
{
    if (isConnected() && (getConnectedPort()->getOwner() == provider))
        return;
    BaseClass::connectTo(new ProvidePurposePort(provider));
}
