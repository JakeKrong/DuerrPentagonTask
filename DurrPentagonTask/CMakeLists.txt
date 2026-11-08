# CMakeList.txt : CMake project for DurrPentagonTask, include source and define
# project specific logic here.
#

# Add source to this project's executable.
qt_add_executable (DurrPentagonTask "DurrPentagonTask.cpp" "DurrPentagonTask.h")
target_link_libraries (DurrPentagonTask PRIVATE Qt6::Widgets)
set_property (TARGET DurrPentagonTask PROPERTY CXX_STANDARD 20)

# Copy the Qt DLLs and plugins the app needs next to the .exe, so it runs straight from the build folder.
if (WIN32 AND TARGET Qt6::windeployqt)
  add_custom_command (TARGET DurrPentagonTask POST_BUILD
    COMMAND Qt6::windeployqt --verbose 0 --no-translations --no-compiler-runtime
            --no-opengl-sw --no-system-d3d-compiler "$<TARGET_FILE:DurrPentagonTask>"
    COMMENT "Deploying Qt runtime next to DurrPentagonTask.exe"
    VERBATIM)
elseif (WIN32)
  message (WARNING "Qt6::windeployqt not found; the Qt DLLs will not be copied next to DurrPentagonTask.exe.")
endif()

# TODO: Add tests and install targets if needed.
