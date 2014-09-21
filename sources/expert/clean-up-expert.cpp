#include "clean-up-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-plan-items.hpp"

REGISTER_EXPERT(CleanUpExpert)

void CleanUpExpert::visitAbstractPlanItem(AbstractPlanItem* item)
{
    if (item->getStatus() != AbstractPlanItem::Failed)
        return;

    currentBlackboard->removeItem(item);
}

void CleanUpExpert::visitMoveToPositionPlanItem(MoveToPositionPlanItem* item)
{
    if (item->isActive())
        return;
    if (item->requireUnit.isConnected())
        return;

    currentBlackboard->removeItem(item);
}

void CleanUpExpert::visitAttackUnitPlanItem(AttackUnitPlanItem* item)
{
    if (item->isActive())
        return;
    if (item->requireUnit.isConnected() && item->enemyUnit.isConnected())
        return;

    currentBlackboard->removeItem(item);
}

void CleanUpExpert::visitAttackPositionPlanItem(AttackPositionPlanItem* item)
{
    if (item->isActive())
        return;
    if (item->requireUnit.isConnected())
        return;

    currentBlackboard->removeItem(item);
}