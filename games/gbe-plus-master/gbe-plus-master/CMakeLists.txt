cmake_minimum_required(VERSION 2.8.11)

project(gbe_plus)

if (NOT MSVC)
	add_definitions(-O3 -funroll-loops -Wno-error -Wno-narrowing -std=c++17)
endif()

if (MSVC AND WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeTests)

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIRS})
	
	if (WIN32 AND MSVC)
		SET(SDL2MAIN_LIBRARY CACHE STRING "SDL2main library")
	else()
		set(SDL2MAIN_LIBRARY "")
	endif()
else()
    message(STATUS "SDL2 not found.")
endif()

option(LINK_CABLE "Enable link cable emulation (requires SDL2_net)" ON)

if (LINK_CABLE)
    find_package(SDL2NET REQUIRED)
    if (SDL2NET_FOUND)
        include_directories(${SDL2NET_INCLUDE_DIR})
        add_definitions(-DGBE_NETPLAY)
    else()
        message(STATUS "SDL2_net not found.")
    endif()
endif()

option(ADVANCED_DEBUG "Enable advanced debugging features (may affect performance)" OFF)

if (ADVANCED_DEBUG)
    add_definitions(-DGBE_DEBUG)
endif()

option(USE_OGL "Enable OpenGL for drawing operations (requires OpenGL)" ON)

option(FAST_FETCH "Enables fast instruction fetching on the GBA without memory checks. Offers a small speedup, on by default. Required to be off for Campho Advance emulation." ON)

if (FAST_FETCH)
    add_definitions(-DGBE_FAST_FETCH)
endif()

if (USE_OGL)
    find_package(OpenGL REQUIRED)
    if (OPENGL_FOUND)
        include_directories(${OPENGL_INCLUDE_DIR})
        add_definitions(-DGBE_OGL)
    else()
        message(STATUS "OpenGL not found.")
    endif()
endif()

if (WIN32 AND USE_OGL)
    find_package(GLEW REQUIRED)
    if (GLEW_FOUND)
        include_directories(${GLEW_INCLUDE_DIRS})
    	add_definitions(-DGBE_GLEW)
    else()
        message(STATUS "GLEW not found.")
    endif()
endif()

option(QT_GUI "Enable the Qt GUI" ON)

if(QT_GUI)
    find_package(Qt5OpenGL REQUIRED)
    find_package(Qt5Widgets REQUIRED)
    find_package(Qt5Gui REQUIRED)
    set(GBE_QT_LIBS Qt5::Gui Qt5::Widgets Qt5::OpenGL)
endif()

add_subdirectory(src)

SET(USER_HOME $ENV{HOME} CACHE STRING "Target User Home")
SET(USER $ENV{USER} CACHE STRING "Target User")

MESSAGE("-- Target User Home -> ${USER_HOME}")
MESSAGE("-- Target User -> ${USER}")
