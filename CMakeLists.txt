CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(dynamind-analysis)


FIND_PATH(DYNAMINDDEV DynaMindDevCmake.txt
            PATHS   $ENV{DYNAMIND_DIR}/include
            PATH_SUFFIXES dynamindcore/CMake)

IF(DYNAMIND_INCLUDE_DIR)
        MESSAGE(STATUS "Compiled from toolbox")
        MESSAGE(STATUS "DYNAMIND_INCLUDE_DIR ${DYNAMIND_INCLUDE_DIR}")
        SET (DYNAMIND_CORE_LIBRARY dynamindcore)
        SET (DYNAMIND_TOOLBOX_LIBRARY dynamindtoolbox)
        MESSAGE(STATUS "${DYNAMIND_CORE_LIBRARY}")
ELSE()
        MESSAGE(STATUS "Compiled from stand alone")
        FIND_PATH(DYNAMINDDEV DynaMindDevCmake.txt
        PATHS $ENV{DYNAMIND_DIR}/include
        PATH_SUFFIXES dynamindcore/CMake)

        IF(DYNAMINDDEV)
                INCLUDE(${DYNAMINDDEV}/DynaMindDevCmake.txt)
        ELSE()
                MESSAGE(FATAL_ERROR "Could not find dynamind. Maybe not installed ?")
        ENDIF()

ENDIF()




IF(CMAKE_BUILD_TYPE STREQUAL Debug)
	MESSAGE(STATUS "Build Type = Debug")
	ADD_DEFINITIONS(-DDEBUG)
ELSE()
	MESSAGE(STATUS "Build Type = Release")
ENDIF()
IF(DYNAMIND_TOOLBOX_BUILD_DIR)
    SET(LIBRARY_OUTPUT_PATH ${DYNAMIND_TOOLBOX_BUILD_DIR}/Modules)
    SET(EXECUTABLE_OUTPUT_PATH ${DYNAMIND_TOOLBOX_BUILD_DIR}/Modules)
    SET(SCRIPT_PATH ${DYNAMIND_TOOLBOX_BUILD_DIR}/PythonModules )
ELSE()
    SET(LIBRARY_OUTPUT_PATH ${dynamind-basicmodules_BINARY_DIR})
    SET(EXECUTABLE_OUTPUT_PATH ${dynamind-basicmodules_BINARY_DIR})
    SET(SCRIPT_PATH ${dynamind-basicmodules_BINARY_DIR}/PythonModules )
ENDIF()

SET(CMAKE_MODULE_PATH "${dynamind-analysis_SOURCE_DIR}/CMake/Packages")

FIND_PACKAGE(Qt4 COMPONENTS QtCore QtGui QtXml QtOpenGL REQUIRED)
FIND_PACKAGE(Boost COMPONENTS system thread REQUIRED)
FIND_PACKAGE(ICU REQUIRED)

SET(QT_USE_QTWEBKIT 1)
SET(QT_USE_QTXML 1)
SET(QT_USE_QTOPENGL 1)
SET(QT_USE_QTSQL 1)
SET(QT_USE_QTSVG 1)
SET(QT_USE_QTNETWORK 1)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(src/)
INCLUDE_DIRECTORIES( ${DYNAMIND_EXTENSIONS_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${ICU_INCLUDE_DIRS})

FILE(GLOB CPP_GUI_VIEWER_FILES *.cpp src/*.cpp src/*.h)
FILE(GLOB UI_VIEWER_FILES *.ui src/*.ui)

QT4_WRAP_UI(CPP_GUI_VIEWER_FILES ${UI_VIEWER_FILES})

INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES( ${DYNAMIND_INCLUDE_DIR})
INCLUDE_DIRECTORIES( ${DYNAMIND_EXTENSIONS_INCLUDE_DIR})

FIND_PACKAGE(GDAL)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

ADD_LIBRARY( dynamind-analysis SHARED ${CPP_GUI_VIEWER_FILES} ${QTGL_CPPS})

SET_TARGET_PROPERTIES(dynamind-analysis PROPERTIES AUTOMOC TRUE)

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
	SET_TARGET_PROPERTIES(dynamind-analysis PROPERTIES COMPILE_FLAGS "-frounding-math -fPIC")
ENDIF()



IF (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
       TARGET_LINK_LIBRARIES(dynamind-analysis ${ICU_LIBRARIES} ${DYNAMIND_EXTENSIONS_LIBRARY} ${DYNAMIND_TOOLBOX_LIBRARY}  ${DYNAMIND_CORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES}  ${GDAL_LIBRARY} -lmapnik )
ELSE()
       TARGET_LINK_LIBRARIES(dynamind-analysis ${ICU_LIBRARIES} ${DYNAMIND_EXTENSIONS_LIBRARY} ${DYNAMIND_TOOLBOX_LIBRARY}  ${DYNAMIND_CORE_LIBRARY} ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES} ${QGLVIEWER_LIBRARIES} ${Boost_LIBRARIES}  ${GDAL_LIBRARY} -lmapnik  /usr/local/Cellar/icu4c/52.1/lib/libicuuc.dylib )
ENDIF()


ADD_SUBDIRECTORY(scripts)

INSTALL(TARGETS dynamind-analysis
        DESTINATION "lib"
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

