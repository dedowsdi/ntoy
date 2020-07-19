cmake_minimum_required(VERSION 3.8)

project(ntoy VERSION 0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(OpenGL REQUIRED)
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osgViewer
    osgText
    osgAnimation
    osgParticle
    osgFX
    osgShadow
    osgWidget
    osgGA
    osgDB
    osgUtil
    )

include_directories(
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    util/ntoy/include
    )

set(SRC
    src/OsgFactory.cpp
    src/OsgQuery.cpp
    src/StringUtil.cpp
    src/main.cpp
    src/Resource.cpp
    src/NodeToy.cpp
    )

add_executable(ntoy ${SRC})

target_link_libraries(ntoy
    PRIVATE
    ${OPENSCENEGRAPH_LIBRARIES}
    OpenGL::GL
    )

target_include_directories(ntoy
    PRIVATE
    include
    ../../include
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    )

target_compile_options(ntoy PRIVATE $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-gdwarf -g3>)
target_compile_options(ntoy PRIVATE $<$<CONFIG:Debug>:-DDEBUG>)

install(TARGETS ntoy
    RUNTIME DESTINATION bin
    )
