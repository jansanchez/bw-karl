#pragma once

#include "worker-manager.h"
#include "scout-manager.h"

class UnitDistributer
{
	public:
		static UnitDistributer& instance()
		{
			return Singleton<UnitDistributer>::instance();
		}

		void tick() const
		{
			auto list = InformationKeeper::instance().self()->idleUnits();
			for (auto it : list) {
				BWAPI::UnitType ut = it->getType();
				if (ut.isWorker()) {
					LOG3 << "WorkerManager should use Worker...";
					WorkerManager::instance().useIdleWorker(it);
				} else if (ut.isRefinery()) {
					if (it->get()->isBeingConstructed())
						continue;
					LOG2 << "WorkerManager should use Extractor...";
					WorkerManager::instance().useIdleExtractor(it);
				} else if (ut == BWAPI::UnitTypes::Zerg_Overlord) {
					//LOG1 << "ScoutManager should use Overlord...";
					//ScoutManager::instance().useScout(it);
				}
			}
		}
};
