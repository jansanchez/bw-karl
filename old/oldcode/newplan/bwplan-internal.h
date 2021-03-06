#pragma once

#include "resources.h"
#include "operations.h"
#include "checkpoints.h"
#include "plan.h"
#include "plan-impl.h"

namespace {

/* for Operation::firstApplyableAt(): */
void Needs(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.getExisting(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Locks(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	if (res.get(ri) < num) {
		blocking = ri;
		result   = std::numeric_limits<TimeType>::max();
	}
}

void Consums(const Resources& res, int num, const ResourceIndex& ri, TimeType& result, ResourceIndex& blocking)
{
	result = std::max(result, res.firstMoreThan(ri, num, blocking));
}

/* for Operation::apply(): */
void Locks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.incLocked(ri, applytime, num);
}

void Unlocks(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool /*pushdecs*/)
{
	if (interval.contains(applytime))
		res.decLocked(ri, applytime, num);
}

void Consums(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool pushdecs)
{
	if (interval.contains(applytime, pushdecs))
		res.dec(ri, applytime, num);
}

void Prods(Resources& res, int num, const ResourceIndex& ri, const TimeInterval& interval, const TimeType& applytime, bool /*pushdecs*/)
{
	if (interval.contains(applytime))
		res.inc(ri, applytime, num);
}

} // end namespace

bool ResourceIndex::isLockable() const
{
	return (index_ < IndexLockedEnd);
}

ResourceIndex ResourceIndex::byName(const std::string& name)
{
	typedef std::map<std::string, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices())
				result[it.getName()] = it;
			return result;
		}();

	auto it = fast.find(name);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byUnitType(const BWAPI::UnitType& ut)
{
	typedef std::map<BWAPI::UnitType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::UnitType t = it.associatedUnitType();
				if (t != BWAPI::UnitTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(ut);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byTechType(const BWAPI::TechType& tt)
{
	typedef std::map<BWAPI::TechType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::TechType t = it.associatedTechType();
				if (t != BWAPI::TechTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(tt);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

ResourceIndex ResourceIndex::byUpgradeType(const BWAPI::UpgradeType& gt)
{
	typedef std::map<BWAPI::UpgradeType, ResourceIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllResourceIndices()) {
				BWAPI::UpgradeType t = it.associatedUpgradeType();
				if (t != BWAPI::UpgradeTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(gt);
	if (it == fast.end())
		return ResourceIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byName(const std::string& name)
{
	typedef std::map<std::string, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices())
				result[it.getName()] = it;
			return result;
		}();

	auto it = fast.find(name);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byUnitType(const BWAPI::UnitType& ut)
{
	typedef std::map<BWAPI::UnitType, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::UnitType t = it.associatedUnitType();
				if (t != BWAPI::UnitTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(ut);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byTechType(const BWAPI::TechType& tt)
{
	typedef std::map<BWAPI::TechType, OperationIndex> MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::TechType t = it.associatedTechType();
				if (t != BWAPI::TechTypes::None)
					result[t] = it;
			}
			return result;
		}();

	auto it = fast.find(tt);
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}

OperationIndex OperationIndex::byUpgradeType(const BWAPI::UpgradeType& gt, int level)
{
	typedef std::pair<BWAPI::UpgradeType, int>	ItemType;
	typedef std::map<ItemType, OperationIndex>	MapType;
	static MapType fast = []
		{
			MapType result;
			for (auto it : AllOperationIndices()) {
				BWAPI::UpgradeType t = it.associatedUpgradeType();
				if (t != BWAPI::UpgradeTypes::None) {
					int level = it.getUpgradeLevel();
					result[std::make_pair(t, level)] = it;
				}
			}
			return result;
		}();

	auto it = fast.find(std::make_pair(gt, level));
	if (it == fast.end())
		return OperationIndex::None;
	return it->second;
}