#include "utils/debug-clog.h"
#include "utils/debug-1.h"

#define  NO_ASSOCIATIONS
#include "newplan/bwplan.h"
#include "newplan/nocheckpoints.h"
#include "newplan/stream-output.h"
#include "newplan/parameter-reader.h"

#include <iostream>

int main(int argc, const char* argv[])
{
	
	BWParameterReader reader;
	std::string savefilename;
	reader.general.add_options()  ("save,s", po::value<std::string>(&savefilename), "Save build to file name.");
	try {
		reader.run(argc, argv);
	} catch (std::exception& e) {
		std::cerr << "Error occurred while parsing parameter: " << e.what();
		return 1;
	}
	if (reader.showhelp) {
		std::cerr << "Use: PlanDesigner [Options...] [Operations...]\n" << reader;
		return 1;
	}

	std::cout << "Parsing arguments...\n";
	BWPlan plan = reader.getStartPlan();
	std::cout << "\n";

	std::cout << "Planed Operations:\n";
	for (auto it : plan.scheduledOperations())
		std::cout << "planed(" << it.scheduledTime() << "): " << it.getName() << "\n";
	std::cout << "\n";

	std::cout << "Planed Resources:\n";
	for (auto it : plan) {
		std::cout << "planed(" << it.time() << "): \t" << outResources(it.getResources()) << "\n";
		auto alt = plan.at(it.time());
		if (it.getResources() != alt.getResources())
			std::cout << "directly gives: " << outResources(alt.getResources()) << "\n";
	}
	std::cout << "\n";
	
	/*
	std::cout << "Planed Resources (pushdecs):\n";
	for (auto it = plan.begin(true); !it.beyond(); ++it) {
		std::cout << "planed(" << it.time() << "): \t" << outResources(it.getResources()) << "\n";
	}
	std::cout << "\n";
	*/
	
	/*
	std::cout << "Corrections:\n";
	for (auto it : plan.getCorrections())
		std::cout << "correction: " << it.interval << ", value = " << it.value << "\n";
	std::cout << "\n";
	*/
	
	std::cout << "Plan finished after " << outTime(plan.endTime()) << " frames.\n";
	
	if (savefilename != "")
		plan.saveToFile(savefilename.c_str());
	
	return 0;
}
