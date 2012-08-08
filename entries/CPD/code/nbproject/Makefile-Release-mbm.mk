#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Release-mbm
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/ParamManager.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/Scenario.o \
	${OBJECTDIR}/src/utils.o \
	${OBJECTDIR}/src/Rectangle.o \
	${OBJECTDIR}/src/CPD2.o \
	${OBJECTDIR}/src/Timer.o \
	${OBJECTDIR}/src/SlimmerRectangle.o \
	${OBJECTDIR}/src/RectangleSplitter.o \
	${OBJECTDIR}/src/MapCpd.o \
	${OBJECTDIR}/src/ScenarioRM.o \
	${OBJECTDIR}/src/QuadTree.o \
	${OBJECTDIR}/src/AbstractCPD.o \
	${OBJECTDIR}/src/Tile.o \
	${OBJECTDIR}/src/DijkstraNode.o \
	${OBJECTDIR}/src/GenericNode.o \
	${OBJECTDIR}/src/TableCompresser1.o \
	${OBJECTDIR}/src/CompressedMoveDatabase.o \
	${OBJECTDIR}/src/QTDatabase.o \
	${OBJECTDIR}/src/ScenarioLoader.o \
	${OBJECTDIR}/src/GridMap.o \
	${OBJECTDIR}/src/AbstractGraphEdge.o \
	${OBJECTDIR}/src/MoveTable.o \
	${OBJECTDIR}/src/SubstringManager.o \
	${OBJECTDIR}/src/utils2.o \
	${OBJECTDIR}/src/TableCompresser.o \
	${OBJECTDIR}/src/ReachabilityTable.o \
	${OBJECTDIR}/src/AbstractGraph.o \
	${OBJECTDIR}/src/globals.o \
	${OBJECTDIR}/src/SlimRectangle.o \
	${OBJECTDIR}/src/Entry.o \
	${OBJECTDIR}/src/AbstractGraphNode.o \
	${OBJECTDIR}/src/GenericEdge.o \
	${OBJECTDIR}/src/CompressedTable.o \
	${OBJECTDIR}/src/GenericMap.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-pthread -D MOVEBYMOVEPATH
CXXFLAGS=-pthread -D MOVEBYMOVEPATH

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cpd-comp-mbm

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cpd-comp-mbm: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cpd-comp-mbm ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/src/ParamManager.o: src/ParamManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ParamManager.o src/ParamManager.cpp

${OBJECTDIR}/src/main.o: src/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/Scenario.o: src/Scenario.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Scenario.o src/Scenario.cpp

${OBJECTDIR}/src/utils.o: src/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils.o src/utils.cpp

${OBJECTDIR}/src/Rectangle.o: src/Rectangle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Rectangle.o src/Rectangle.cpp

${OBJECTDIR}/src/CPD2.o: src/CPD2.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/CPD2.o src/CPD2.cpp

${OBJECTDIR}/src/Timer.o: src/Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Timer.o src/Timer.cpp

${OBJECTDIR}/src/SlimmerRectangle.o: src/SlimmerRectangle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SlimmerRectangle.o src/SlimmerRectangle.cpp

${OBJECTDIR}/src/RectangleSplitter.o: src/RectangleSplitter.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/RectangleSplitter.o src/RectangleSplitter.cpp

${OBJECTDIR}/src/MapCpd.o: src/MapCpd.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MapCpd.o src/MapCpd.cpp

${OBJECTDIR}/src/ScenarioRM.o: src/ScenarioRM.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ScenarioRM.o src/ScenarioRM.cpp

${OBJECTDIR}/src/QuadTree.o: src/QuadTree.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/QuadTree.o src/QuadTree.cpp

${OBJECTDIR}/src/AbstractCPD.o: src/AbstractCPD.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/AbstractCPD.o src/AbstractCPD.cpp

${OBJECTDIR}/src/Tile.o: src/Tile.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Tile.o src/Tile.cpp

${OBJECTDIR}/src/DijkstraNode.o: src/DijkstraNode.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/DijkstraNode.o src/DijkstraNode.cpp

${OBJECTDIR}/src/GenericNode.o: src/GenericNode.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GenericNode.o src/GenericNode.cpp

${OBJECTDIR}/src/TableCompresser1.o: src/TableCompresser1.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TableCompresser1.o src/TableCompresser1.cpp

${OBJECTDIR}/src/CompressedMoveDatabase.o: src/CompressedMoveDatabase.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/CompressedMoveDatabase.o src/CompressedMoveDatabase.cpp

${OBJECTDIR}/src/QTDatabase.o: src/QTDatabase.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/QTDatabase.o src/QTDatabase.cpp

${OBJECTDIR}/src/ScenarioLoader.o: src/ScenarioLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ScenarioLoader.o src/ScenarioLoader.cpp

${OBJECTDIR}/src/GridMap.o: src/GridMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GridMap.o src/GridMap.cpp

${OBJECTDIR}/src/AbstractGraphEdge.o: src/AbstractGraphEdge.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/AbstractGraphEdge.o src/AbstractGraphEdge.cpp

${OBJECTDIR}/src/MoveTable.o: src/MoveTable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/MoveTable.o src/MoveTable.cpp

${OBJECTDIR}/src/SubstringManager.o: src/SubstringManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SubstringManager.o src/SubstringManager.cpp

${OBJECTDIR}/src/utils2.o: src/utils2.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/utils2.o src/utils2.cpp

${OBJECTDIR}/src/TableCompresser.o: src/TableCompresser.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/TableCompresser.o src/TableCompresser.cpp

${OBJECTDIR}/src/ReachabilityTable.o: src/ReachabilityTable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/ReachabilityTable.o src/ReachabilityTable.cpp

${OBJECTDIR}/src/AbstractGraph.o: src/AbstractGraph.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/AbstractGraph.o src/AbstractGraph.cpp

${OBJECTDIR}/src/globals.o: src/globals.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/globals.o src/globals.cpp

${OBJECTDIR}/src/SlimRectangle.o: src/SlimRectangle.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/SlimRectangle.o src/SlimRectangle.cpp

${OBJECTDIR}/src/Entry.o: src/Entry.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/Entry.o src/Entry.cpp

${OBJECTDIR}/src/AbstractGraphNode.o: src/AbstractGraphNode.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/AbstractGraphNode.o src/AbstractGraphNode.cpp

${OBJECTDIR}/src/GenericEdge.o: src/GenericEdge.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GenericEdge.o src/GenericEdge.cpp

${OBJECTDIR}/src/CompressedTable.o: src/CompressedTable.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/CompressedTable.o src/CompressedTable.cpp

${OBJECTDIR}/src/GenericMap.o: src/GenericMap.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} $@.d
	$(COMPILE.cc) -O3 -MMD -MP -MF $@.d -o ${OBJECTDIR}/src/GenericMap.o src/GenericMap.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/cpd-comp-mbm

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
