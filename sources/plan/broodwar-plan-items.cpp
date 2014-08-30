#include "broodwar-plan-items.hpp"
#include "broodwar-boundary-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"

AbstractSimpleUnitPlanItem::AbstractSimpleUnitPlanItem(BWAPI::UnitType ut, bool od)
    : requireUnit(this, ut), provideUnit(this, od)
{ }

void AbstractSimpleUnitPlanItem::removeFinished(AbstractAction* /*action*/)
{
    requireUnit.bridge(&provideUnit);
}

void AbstractSimpleUnitPlanItem::addRequirements(BWAPI::UnitType ut)
{
    BWAPI::UnitType builder = ut.whatBuilds().first;
    for (auto it : ut.requiredUnits())
        if (it.first != builder)
            new RequireUnitExistancePort(this, it.first);
}

void AbstractSimpleUnitPlanItem::bridgeUnitExistancePortsTo(AbstractItem* item)
{
    std::set<ProvideUnitExistancePort*> providePorts;
    for (auto it : ports) {
        auto ptr = dynamic_cast<ProvideUnitExistancePort*>(it);
        if (ptr != NULL)
            providePorts.insert(ptr);
    }
    for (auto it : providePorts)
        it->getConnectedPort()->connectTo(item);
}

void AbstractSimpleUnitPlanItem::removeRequireExistancePorts()
{
    std::set<RequireUnitExistancePort*> requirePorts;
    for (auto it : ports) {
        auto ptr = dynamic_cast<RequireUnitExistancePort*>(it);
        if (ptr != NULL)
            requirePorts.insert(ptr);
    }
    for (auto it : requirePorts)
        delete it;
}


GatherMineralsPlanItem::GatherMineralsPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(provider->getUnitType(), true), requireMineralField(this, m)
{
    provideUnit.updateData(provider->getUnitType(), BWAPI::Position(m->getTilePosition()));
    if (provider != NULL)
        requireUnit.connectTo(provider);
}

void GatherMineralsPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGatherMineralPlanItem(this);
}

AbstractAction* GatherMineralsPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new CollectMineralsAction(unit, requireMineralField.getUnit());
}


MorphUnitPlanItem::MorphUnitPlanItem(BWAPI::UnitType type, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(type.whatBuilds().first),
      requireResources(this, type.mineralPrice(), type.gasPrice()),
      supply(this, type, true),
      unitType(type)
{
    addRequirements(unitType);
    provideUnit.updateData(unitType, (provider != NULL) ? provider->getPosition() : BWAPI::Positions::Unknown);
    supply.estimatedDuration = provideUnit.estimatedDuration = unitType.buildTime();
    requireUnit.connectTo(provider);
}

void MorphUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMorphUnitPlanItem(this);
}

AbstractAction* MorphUnitPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new MorphUnitAction(unit, unitType);
}

void MorphUnitPlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    setExecuting();
    removePort(&requireResources);
    if (supply.isRequirePort())
        removePort(&supply);
    removeRequireExistancePorts();
}

void MorphUnitPlanItem::removeFinished(AbstractAction* action)
{
    // ToDo: I do not know why requireUnit can not be connected, I think it is a bug...
    if (requireUnit.isConnected()) {
        bridgeUnitExistancePortsTo(requireUnit.getConnectedPort()->getOwner());
        AbstractSimpleUnitPlanItem::removeFinished(action);
    }
}


MoveToPositionPlanItem::MoveToPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), position(p)
{
    provideUnit.updateData(provider->getUnitType(), position);
    requireUnit.connectTo(provider);
}

void MoveToPositionPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitMoveToPositionPlanItem(this);
}

void MoveToPositionPlanItem::updateEstimates(Time current)
{
    provideUnit.estimatedDuration = (int)(position.getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
    AbstractSimpleUnitPlanItem::updateEstimates(current);
}

AbstractAction* MoveToPositionPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new MoveToPositionAction(unit, position);
}


AttackPositionPlanItem::AttackPositionPlanItem(ProvideUnitPort* provider, BWAPI::Position p)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), position(p)
{
    provideUnit.updateData(provider->getUnitType(), position);
    requireUnit.connectTo(provider);
}

void AttackPositionPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitAttackPositionPlanItem(this);
}

void AttackPositionPlanItem::updateEstimates(Time current)
{
    provideUnit.estimatedDuration = (int)(position.getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
    AbstractSimpleUnitPlanItem::updateEstimates(current);
}

AbstractAction* AttackPositionPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new AttackPositionAction(unit, position);
}


AttackUnitPlanItem::AttackUnitPlanItem(ProvideUnitPort* provider, EnemyUnitBoundaryItem* enemy)
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), enemyUnit(this,enemy)
{
    provideUnit.updateData(provider->getUnitType(), enemy->getPosition());
    requireUnit.connectTo(provider);
}

void AttackUnitPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitAttackUnitPlanItem(this);
}

void AttackUnitPlanItem::updateEstimates(Time current)
{
    provideUnit.estimatedDuration = (int)(enemyUnit.getPosition().getDistance(requireUnit.getPosition()) / provideUnit.getUnitType().topSpeed());
    AbstractSimpleUnitPlanItem::updateEstimates(current);
}

AbstractAction* AttackUnitPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new AttackUnitAction(unit, enemyUnit.getUnit());
}




BuildPlanItem::BuildPlanItem(Array2d<FieldInformations>* f, BWAPI::UnitType ut, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first),
      requireResources(this, ut.mineralPrice(), ut.gasPrice()),
      requireSpace(this, f, ut, p),
      supply(this, ut),
      unitType(ut)
{
    addRequirements(unitType);
    provideUnit.updateData(unitType, BWAPI::Position(p));
    supply.estimatedDuration = provideUnit.estimatedDuration = unitType.buildTime();
}

void BuildPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitBuildPlanItem(this);
}

AbstractAction* BuildPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new ZergBuildAction(unit, unitType, requireSpace.getTilePosition());
}

void BuildPlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    setExecuting();
    removePort(&requireResources);
    removePort(&requireSpace);
    removeRequireExistancePorts();
}

void BuildPlanItem::removeFinished(AbstractAction* action)
{
    assert(requireUnit.isConnected());
    bridgeUnitExistancePortsTo(requireUnit.getConnectedPort()->getOwner());
    AbstractSimpleUnitPlanItem::removeFinished(action);
}




GiveUpPlanItem::GiveUpPlanItem()
    : AbstractPlanItem()
{ }

void GiveUpPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGiveUpPlanItem(this);
}

AbstractAction* GiveUpPlanItem::buildAction()
{
    return new GiveUpAction();
}

void GiveUpPlanItem::removeFinished(AbstractAction* /*action*/)
{ }
