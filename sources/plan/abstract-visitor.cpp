#include "abstract-visitor.hpp"
#include "broodwar-ports.hpp"
#include "broodwar-plan-items.hpp"

void BasicVisitor::visitAbstractPort(AbstractPort* /*port*/)
{ }

void BasicVisitor::visitProvideUnitPort(ProvideUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitRequireUnitPort(RequireUnitPort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitResourcePort(ResourcePort* port)
{
    visitAbstractPort(port);
}

void BasicVisitor::visitAbstractBoundaryItem(AbstractBoundaryItem* /*item*/)
{ }

void BasicVisitor::visitOwnUnitPlanItem(OwnUnitPlanItem* item)
{
    visitAbstractBoundaryItem(item);
}

void BasicVisitor::visitAbstractPlanItem(AbstractPlanItem* /*item*/)
{ }

void BasicVisitor::visitGatherMineralPlanItem(GatherMineralsPlanItem* item)
{
    visitAbstractPlanItem(item);
}

void BasicVisitor::visitBuildPlanItem(BuildPlanItem* item)
{
    visitAbstractPlanItem(item);
}
