OBJECTPATH   = ../out/
SOURCEPATH   = ../karl/
BWAPIPATH    = ../includes
LIBPATH      = ../lib/
EXEPATH      = ../

ASSIGNMENT-TEST-SOURCES  = hungarian-algorithm.cpp timer.cpp
ASSIGNMENT-TEST-PSOURCES = $(addprefix $(SOURCEPATH), $(ASSIGNMENT-TEST-SOURCES))

CXX          = g++ -ggdb
CXXINCLUDES  = -I../ -I$(SOURCEPATH) -I$(BWAPIPATH)
CXXFLAGS     = -Wall -Wextra -O3 -DNDEBUG --std=c++0x $(CXXINCLUDES)
CXXLIBS      = -L$(LIBPATH) -lBWAPI

$(EXEPATH)assignment-test.exe: assignment-test.cpp $(ASSIGNMENT-TEST-PSOURCES)
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) $(CXXLIBS) assignment-test.cpp log.cpp $(ASSIGNMENT-TEST-PSOURCES) -o $@

assignment: $(EXEPATH)assignment-test.exe
	$<

$(EXEPATH)mineral-speed-test.exe: mineral-speed-test.cpp
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) $(CXXLIBS) $< -o $@ -lBWAPI

mineral-speed: $(EXEPATH)mineral-speed-test.exe
	$<

$(EXEPATH)lambda-support.exe: bool-lambda-support.cpp
	$(CXX) $(CXXINCLUDES) $(CXXFLAGS) $(CXXLIBS) $< -o $@

lambda-support: $(EXEPATH)lambda-support.exe
	$<
