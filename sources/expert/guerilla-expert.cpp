#include "guerilla-expert.hpp"
#include "expert-registrar.hpp"
#include "plan/broodwar-boundary-items.hpp"
#include "utils/bw-helper.hpp"
#include <algorithm>

# define M_PI 3.141592653589793238462643383279502884L

REGISTER_EXPERT(GuerillaExpert)

void GuerillaExpert::beginTraversal()
{
    allUnits.clear();
}

void GuerillaExpert::visitOwnUnitBoundaryItem(OwnUnitBoundaryItem* item)
{
    if (    item->isBuilding()
         || item->isFlying()
         || !item->getPosition().isValid()
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Egg)
        )
        return;
    allUnits.push_back(item);
}

void GuerillaExpert::visitEnemyUnitBoundaryItem(EnemyUnitBoundaryItem* item)
{
    if (    item->isBuilding()
         || item->isFlying()
         || !item->getPosition().isValid()
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Larva)
         || (item->getUnitType() == BWAPI::UnitTypes::Zerg_Egg)
         || !item->isVisible()
        )
        return;
    allUnits.push_back(item);
}

// 1. Cluster units
void GuerillaExpert::endTraversal()
{
    if (currentBlackboard->getLastUpdateTime() % 100 == 0) {
        while (!allUnits.empty()) {
            std::vector<AbstractSpaceUnitBoundaryItem*> cluster = { allUnits.back() };
            allUnits.pop_back();
            for (unsigned int k=0; k<cluster.size(); ++k) {
                auto position = cluster[k]->getPosition();
                auto seperator = std::remove_if(allUnits.begin(), allUnits.end(), [&](AbstractSpaceUnitBoundaryItem* unit) {
                        bool result = (position.getDistance(unit->getPosition()) < 128.0);
                        if (result)
                            cluster.push_back(unit);
                        return result;
                    });
                allUnits.erase(seperator, allUnits.end());
            }
            analyzeSituation(cluster);
        }
    }

    allUnits.clear();
}

// 2. Classify situations
namespace
{
    template <class T>
    double valueOfUnits(std::vector<T*>& units)
    {
        std::sort(units.begin(), units.end(), [](T* lhs, T* rhs) {
                return (lhs->getHealth() > rhs->getHealth());
            });
        double value = 0.0;
        double strength = 0.0;
        for (auto it : units) {
            strength += it->getGroundDPS();
            value += it->getHealth() * strength;
        }
        return value;
    }
}

void GuerillaExpert::analyzeSituation(const std::vector<AbstractSpaceUnitBoundaryItem*>& units)
{
    std::vector<OwnUnitBoundaryItem*> ownUnits;
    std::vector<EnemyUnitBoundaryItem*> enemyUnits;
    for (auto it : units) {
        auto ou = dynamic_cast<OwnUnitBoundaryItem*>(it);
        if (ou != NULL) {
            ownUnits.push_back(ou);
        } else {
            auto eu = dynamic_cast<EnemyUnitBoundaryItem*>(it);
            if (eu != NULL)
                enemyUnits.push_back(eu);
        }
    }

    if (ownUnits.empty() || enemyUnits.empty())
        return;

    double ownPower = valueOfUnits(ownUnits);
    double enemyPower = valueOfUnits(enemyUnits);

    if (ownPower > enemyPower)
        return;

    std::cout << currentBlackboard->getLastUpdateTime() << ": Retreating " << ownUnits.size() << " units from " << enemyUnits.size() << " enemy units...\n";
    retreat(ownUnits, enemyUnits);
}

// 3. and action ...
namespace
{
    double angleOf(const BWAPI::Position& pos)
    {
        if (pos.x() > 0.0) {
            return atan((double)pos.y() / (double)pos.x());
        } else if (pos.x() < 0.0) {
            return M_PI + atan((double)pos.y() / (double)pos.x());
        } else {
            return (pos.y() > 0.0) ? 0.5 * M_PI : -0.5 * M_PI;
        }
    }
}

void GuerillaExpert::retreat(const std::vector<OwnUnitBoundaryItem*>& ownUnits, const std::vector<EnemyUnitBoundaryItem*>& enemyUnits)
{
    std::vector<double> angles(enemyUnits.size(), 0.0);
    for (auto unit : ownUnits) {
        std::transform(enemyUnits.begin(), enemyUnits.end(), angles.begin(), [unit] (EnemyUnitBoundaryItem* enemy) {
                return angleOf(enemy->getPosition() - unit->getPosition());
            });
        std::sort(angles.begin(), angles.end());
        double bestArc    = M_PI + 0.5 * (angles.front() + angles.back());
        double bestAngle  = 2.0 * M_PI + angles.front() - angles.back();
        for (unsigned int k=1, size=angles.size(); k<size; ++k) {
            double arc = angles[k] - angles[k-1];
            if (arc > bestArc) {
                bestArc   = arc;
                bestAngle = 0.5 * (angles[k] + angles[k-1]);
            }
        }
        double distance = 96.0;
        BWAPI::Position dir((int)(distance * cos(bestAngle)), (int)(distance * sin(bestAngle)));
        retreatTo(unit, unit->getPosition() + dir);
    }
    //std::cout << "enemyUnits:\n";
    //for (auto it : enemyUnits)
    //    std::cout << it << "; type: " << it->getUnitType() << "; id: " << it->getUnit()->getID() << "; pos: " << it->getPosition() << "\n";
    //interrupt();
}

void GuerillaExpert::retreatTo(OwnUnitBoundaryItem* ownUnit, BWAPI::Position pos)
{
    //std::cout << ownUnit << "; type: " << ownUnit->getUnitType() << "; id: " << ownUnit->getUnit()->getID() << "; pos: " << ownUnit->getPosition() << "\n";
    auto provider = &ownUnit->provideUnit;
    if (provider->isActiveConnection()) {
        auto planItem = dynamic_cast<AbstractSimpleUnitPlanItem*>(provider->getConnectedPort()->getOwner());
        if (planItem == NULL)
            return;
        auto moveItem = dynamic_cast<MoveToPositionPlanItem*>(planItem);
        if ((moveItem != NULL) && (moveItem->getPosition().getDistance(pos) < 32.0))
            return;
        currentBlackboard->terminate(planItem);
        provider = &planItem->provideUnit;
        // ToDo: Copy active plan item...
    }

    auto nextRequirePort = provider->getConnectedPort();
    auto newPlanItem = currentBlackboard->move(provider, pos);
    newPlanItem->provideUnit.connectTo(nextRequirePort);
}
