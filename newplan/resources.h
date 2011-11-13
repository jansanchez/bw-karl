#pragma once

#include "timetype.h"
#include "resourceindex.h"

#include <array>
#include <limits>
#include <cassert>

class Resources
{
	public:	
		Resources() : time(0)
		{ 
			for (auto& it : amount)
				it = 0;
			for (auto& it : locked)
				it = 0;
		}
	
		bool valid() const
		{
			for (int it : amount)
				if (it < 0)
					return false;
			for (int it : locked)
				if (it < 0)
					return false;
			return true;
		}
	 
	public:
		int get(const ResourceIndex& ri) const
		{
			assert(ri.valid());
			return amount[ri.getIndex()] / ri.getScaling();
		}
		
		int getLocked(const ResourceIndex& ri) const
		{
			assert(ri.valid());
			return locked[ri.getIndex()] / ri.getScaling();
		}
		
		int getExisting(const ResourceIndex& ri) const
		{
			return get(ri) + getLocked(ri);
		}
		
		int getInternalValue(const ResourceIndex& ri) const
		{
			assert(ri.valid());
			return amount[ri.getIndex()];
		}
		
		void set(const ResourceIndex& ri, int value)
		{
			assert(ri.valid());
			amount[ri.getIndex()] = value * ri.getScaling();
		}
		
		void setLocked(const ResourceIndex& ri, int value)
		{
			assert(ri.valid());
			locked[ri.getIndex()] = value * ri.getScaling();
		}
		
		// Auto-generated functions see bwplan.cpp
		void inc(const ResourceIndex& ri, int optime, int value);
		
		void incInternal(const ResourceIndex& ri, int value)
		{
			assert(ri.valid());
			amount[ri.getIndex()] += value;
		}
		
		void dec(const ResourceIndex& ri, int optime, int value)
		{
			inc(ri, optime, -value);
		}
		
		void incLocked(const ResourceIndex& ri, int optime, int value)
		{
			assert(ri.valid());
			dec(ri, optime, value);
			locked[ri.getIndex()] += value * ri.getScaling();
		}
		
		void decLocked(const ResourceIndex& ri, int optime, int value)
		{
			incLocked(ri, optime, -value);
		}
		
		// Auto-generated functions see bwplan.cpp
		int getGrowth(const ResourceIndex& ri) const;
		
		int firstMoreThan(const ResourceIndex& ri, int value, ResourceIndex& blocking) const
		{
			assert(ri.valid());
			int current = amount[ri.getIndex()];
			int growth  = getGrowth(ri);
			value *= ri.getScaling();
			if (current >= value) {
				return 0;
			} else if (growth > 0) {
				return (value - current + growth - 1) / growth;
			} else {
				blocking = ri;
				return std::numeric_limits<int>::max();
			}
		}
    
	public:
		// Auto-generated functions see bwplan.cpp
		void advance(int dt);
		 
		void setTime(int newtime)
		{
		   time = newtime;
		}
		 
		int getTime() const
		{
		   return time;
		}

	protected:
		typedef std::array<int, ResourceIndex::IndexLast>       AmountType;
		typedef std::array<int, ResourceIndex::IndexLockedLast> LockedType;
		TimeType        time;
		AmountType		amount;
		LockedType		locked;
};
