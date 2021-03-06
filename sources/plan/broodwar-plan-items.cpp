#include "broodwar-plan-items.hpp"
#include "broodwar-boundary-items.hpp"
#include "abstract-visitor.hpp"
#include "engine/broodwar-actions.hpp"
#include "utils/log.hpp"
#include "utils/assert-throw.hpp"
#include "utils/bw-helper.hpp"

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


GatherResourcesPlanItem::GatherResourcesPlanItem(ResourceBoundaryItem* m, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(provider->getUnitType(), true), requireResource(this, m)
{
    provideUnit.updateData(provider->getUnitType(), BWAPI::Position(m->getTilePosition()));
    if (provider != NULL)
        requireUnit.connectTo(provider);
}

void GatherResourcesPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitGatherResourcesPlanItem(this);
}

AbstractAction* GatherResourcesPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new CollectResourcesAction(unit, requireResource.getUnit());
}

bool GatherResourcesPlanItem::isGatherMinerals() const
{
    return requireResource.isMineralField();
}


MorphUnitPlanItem::MorphUnitPlanItem(BlackboardInformations* i, BWAPI::UnitType type, ResourceCategorySet c, ProvideUnitPort* provider)
    : AbstractSimpleUnitPlanItem(type.whatBuilds().first),
      requireResources(this, i, type.mineralPrice(), type.gasPrice(), c),
      supply(this, type, true),
      unitType(type)
{
    addRequirements(unitType);
    provideUnit.updateData(unitType, (provider != NULL) ? provider->getPosition() : BWAPI::Positions::Unknown);
    if ((unitType == BWAPI::UnitTypes::Zerg_Lair) || (unitType == BWAPI::UnitTypes::Zerg_Hive)) {
        removePort(&supply);
    } else {
        supply.estimatedDuration = provideUnit.estimatedDuration = unitType.buildTime();
    }
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
    requireResources.resourcesConsumed();
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
    : AbstractSimpleUnitPlanItem(provider->getUnitType()), useSmartTurnAround(true), position(p)
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
    return new MoveToPositionAction(unit, position, useSmartTurnAround);
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




BuildPlanItem::BuildPlanItem(BlackboardInformations* i, BWAPI::UnitType ut, ResourceCategorySet c, BWAPI::TilePosition p)
    : AbstractSimpleUnitPlanItem(ut.whatBuilds().first),
      requireResources(this, i, ut.mineralPrice(), ut.gasPrice(), c),
      requireSpace(this, i, ut, p),
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
    requireResources.resourcesConsumed();
    removePort(&requireSpace);
    removeRequireExistancePorts();
}

void BuildPlanItem::removeFinished(AbstractAction* action)
{
    assert(requireUnit.isConnected());
    bridgeUnitExistancePortsTo(requireUnit.getConnectedPort()->getOwner());
    AbstractSimpleUnitPlanItem::removeFinished(action);
}


ResearchTechPlanItem::ResearchTechPlanItem(BlackboardInformations* i, BWAPI::TechType t, ResourceCategorySet c)
    : AbstractSimpleUnitPlanItem(t.whatResearches()),
      requireResources(this, i, t.mineralPrice(), t.gasPrice(), c),
      tech(t)
{
    provideUnit.updateData(t.whatResearches(), BWAPI::Positions::Unknown);
}

void ResearchTechPlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitResearchTechPlanItem(this);
}

AbstractAction* ResearchTechPlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new ResearchTechAction(unit, tech);
}

void ResearchTechPlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    setExecuting();
    requireResources.resourcesConsumed();
}


UpgradePlanItem::UpgradePlanItem(BlackboardInformations* i, BWAPI::UpgradeType u, ResourceCategorySet c, int l)
    : AbstractSimpleUnitPlanItem(u.whatUpgrades()),
      requireResources(this, i, u.mineralPrice(l), u.gasPrice(l), c),
      upgrade(u),
      level(l)
{
    auto requiredUnitType = upgrade.whatsRequired(level);
    if (requiredUnitType != BWAPI::UnitTypes::None)
        new RequireUnitExistancePort(this, requiredUnitType);
    provideUnit.updateData(u.whatUpgrades(), BWAPI::Positions::Unknown);
}

void UpgradePlanItem::acceptVisitor(AbstractVisitor* visitor)
{
    visitor->visitUpgradePlanItem(this);
}

AbstractAction* UpgradePlanItem::buildAction()
{
    unit = requireUnit.getUnit();
    if (unit == NULL)
        return NULL;
    return new UpgradeAction(unit, upgrade);
}

void UpgradePlanItem::visitResourcesConsumedEvent(ResourcesConsumedEvent* /*event*/)
{
    setExecuting();
    requireResources.resourcesConsumed();
    removeRequireExistancePorts();
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
