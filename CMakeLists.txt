# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("DurrPentagonTask")

# Fetch QT automatically on the first configure (refer cmake/FetchQt.cmake)
include ("cmake/FetchQt.cmake")
find_package (Qt6 6.5 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup ()

# Include sub-projects.
add_subdirectory ("DurrPentagonTask")
