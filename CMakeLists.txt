# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.22)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project ("DurrPentagonTask")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Fetch QT automatically on the first configure (refer cmake/FetchQt.cmake)
include ("cmake/FetchQt.cmake")
find_package (Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup ()

# Include sub-projects.
add_subdirectory ("src")

# Unit tests, run with ctest (enable_testing has to be called from this top-level file)
enable_testing ()
add_subdirectory ("tests")
