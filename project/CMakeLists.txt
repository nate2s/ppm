cmake_minimum_required(VERSION 3.16)
project(ppm)

add_subdirectory(library/taffy-2.73/project)

add_executable(ppm
        src/Color.cpp
        src/CommandLineArguments.cpp
        src/Font.cpp
        src/FontFactory.cpp
        src/Lattice.cpp
        src/PPMApp.cpp
        src/Renderer.cpp
        src/TaffyBridge.cpp
        src/main.cpp)

target_compile_features(ppm PRIVATE cxx_std_17)

target_link_libraries(ppm taffy)

include_directories(
        .
        src
        library/taffy-2.73/project/src/evaluator
        library/taffy-2.73/project/src/org/taffy/core/exception
        library/taffy-2.73/project/src/io
        library/taffy-2.73/project/src/graph
	library/taffy-2.73/project/src/scope
	library/taffy-2.73/project/src/taffy
	library/taffy-2.73/project/src/container
	library/taffy-2.73/project/src/third_party/decNumber
	library/taffy-2.73/project/src/maths
	library/taffy-2.73/project/src/class
	library/taffy-2.73/project/src/global
	library/taffy-2.73/project/src/graph
	library/taffy-2.73/project/src/container
        library/taffy-2.73/project/src/class/special
        library/taffy-2.73/project/src/org/taffy/help
        library/taffy-2.73/project/src/taffy/text)
