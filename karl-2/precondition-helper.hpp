#pragma once

#include "precondition.hpp"

struct PreconditionSorter
{
    bool operator () (const Precondition* p1, const Precondition* p2) const
    {
        return p1->time < p2->time;
    }
};

inline int calcMaxTime(Precondition* first)
{
	if (first == NULL)
		return 0;
    return first->time;
}

template <class ... List>
int calcMaxTime(Precondition* first, List... args)
{
    return std::max(calcMaxTime(first), calcMaxTime(args...));
}

/*
inline void setWishTimeSave(int time, int wishtime, Precondition* first)
{
	if (first == NULL)
		return;

    if (first->time == time) {
        first->wishtime = wishtime;
    } else {
        first->wishtime = time-1;
    }
}

template <class ... List>
void setWishTimeSave(int time, int wishtime, Precondition* first, List... args)
{
    setWishTimeSave(time, wishtime, first);
    setWishTimeSave(time, wishtime, args...);
}

inline void setWishTimeTest(int wishtime, Precondition* first)
{
	if (first == NULL)
		return;

    first->wishtime = wishtime;
}

template <class ... List>
void setWishTimeTest(int wishtime, Precondition* first, List... args)
{
    setWishTimeTest(wishtime, first);
    setWishTimeTest(wishtime, args...);
}

struct TestStatus
{
    enum Type { test = 0, save = 1, unknown = 2 };
};

inline TestStatus::Type getType(int time, int wishtime, Precondition* first)
{
	if (first == NULL)
		return TestStatus::save;

    if (first->wishtime == wishtime) {
        return TestStatus::test;
    } else if (first->wishtime == time-1) {
        return TestStatus::save;
    } else {
        return TestStatus::unknown;
    }
}

template <class ... List>
TestStatus::Type getType(int time, int wishtime, Precondition* first, List... args)
{
	TestStatus::Type s = getType(time, wishtime, args...);
	if (first == NULL) {
		return s;
	} else if (first->wishtime == wishtime) {
        return s;
    } else if (first->wishtime == time-1) {
        return std::max(TestStatus::save, s);
    } else {
        return TestStatus::unknown;
    }
}
*/

inline void setWishTimeSimple(int wishtime, Precondition* pre)
{
	if (pre == NULL)
		return;
	
	pre->wishtime = wishtime;
}

template <class ... List>
void setWishTimeSimple(int wishtime, Precondition* first, List... args)
{
    setWishTimeSimple(wishtime, first);
    setWishTimeSimple(wishtime, args...);
}

template <class ... List>
bool updateTimePreconditions(Precondition* self, int duration, List... args)
{
    int time = calcMaxTime(args...);
    if (time == 0)
        return true;
	int oldtime  = self->time - duration;
	int wishtime = self->wishtime - duration;
    self->time   = time + duration;

	int dt       = std::max(oldtime - time, 1);
	int subtime  = std::max(time - 10*dt, wishtime);
	setWishTimeSimple(subtime, args...);
	
	/*
    switch (getType(oldtime, wishtime, args...))
    {
        case TestStatus::test:
            setWishTimeSave(time, wishtime, args...);
            break;

        case TestStatus::save:
			setWishTimeSave(time, wishtime, args...);
            break;

        case TestStatus::unknown:
            setWishTimeTest(wishtime, args...);
            break;
    }
	*/

    return false;
}

template <class T>
void release(T*& pre)
{
	if (pre != NULL) {
		delete pre;
		pre = NULL;
	}
}
