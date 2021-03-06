#pragma once

#include <BWAPI.h>
#include <BWTA.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

class BaseLocationInfo : public boost::enable_shared_from_this<BaseLocationInfo>
{
	friend class InformationKeeper;
	
	public:
		BWTA::BaseLocation* get() const
		{
			return location;
		}
		
		BWAPI::TilePosition getTilePosition() const
		{
			return tilepos;
		}
		
		BWAPI::Position getPosition() const
		{
			return pos;
		}
		
		bool isStartLocation() const
		{
			return startlocation;
		}
		
		int lastSeen() const
		{
			return lastseen;
		}
		
		bool isVisible() const
		{
			return (lastseen == InformationKeeper::instance().currentFrame());
		}
		
		UnitInfoPtr currentBase() const
		{
			return currentbase;
		}
		
		PlayerInfoPtr currentUser() const
		{
			return currentuser;
		}
		
		int currentUserSince() const
		{
			return currentusersince;
		}
		
		RegionInfoPtr getRegion() const
		{
			return region;
		}
		
		double getDistance(BaseLocationInfoPtr base) const
		{
			return BWTA::getGroundDistance(tilepos, base->getTilePosition());
		}
	
	protected:
		void readEveryTurn();
		void setNewBase(UnitInfoPtr base);
		void removeBase();
	
	protected:
		BWTA::BaseLocation* location;
		
		BWAPI::TilePosition tilepos;
		BWAPI::Position pos;
		bool startlocation;
		int lastseen;
		UnitInfoPtr currentbase;
		PlayerInfoPtr currentuser;
		RegionInfoPtr region;
		int currentusersince;
		
		BaseLocationInfo(BWTA::BaseLocation* loc) : location(loc), lastseen(-1)
		{
			tilepos       = location->getTilePosition();
			pos           = location->getPosition();
			startlocation = location->isStartLocation();
			region        = InformationKeeper::instance().getInfo(location->getRegion());
		}
};
