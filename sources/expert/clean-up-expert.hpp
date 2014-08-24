#pragma once

#include "basic-expert.hpp"

class CleanUpExpert : public BasicPortExpert
{
    public:
        void visitAbstractPlanItem(AbstractPlanItem* item) override;
        void visitMoveToPositionPlanItem(MoveToPositionPlanItem* item) override;
        void visitAttackUnitPlanItem(AttackUnitPlanItem* item) override;
        void visitAttackPositionPlanItem(AttackPositionPlanItem* item) override;
};
