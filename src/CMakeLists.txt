# CMakeLists.txt : the DurrPentagonTask application.

# Core library: the model, with no widget code, so the app and the unit tests can both link it.
qt_add_library (PentagonCore STATIC
  PentagonModel.cpp PentagonModel.h
)
# Qt6::Gui rather than Widgets: the model uses QColor and QPolygonF, but no widgets
target_link_libraries (PentagonCore PUBLIC Qt6::Gui)
# Headers are included relative to src/ (e.g. "canvas/PolygonItem.h"), from here and from anything that links this
target_include_directories (PentagonCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# The application: the window, the canvas items and the property widgets.
qt_add_executable (DurrPentagonTask
  main.cpp
  MainWindow.cpp MainWindow.h
  canvas/PolygonItem.cpp canvas/PolygonItem.h
  canvas/VertexHandle.cpp canvas/VertexHandle.h
  canvas/LabelItem.cpp canvas/LabelItem.h
  properties/PropertiesPanel.cpp properties/PropertiesPanel.h
  properties/ColorProperty.cpp properties/ColorProperty.h
  properties/PointProperty.cpp properties/PointProperty.h
)
target_link_libraries (DurrPentagonTask PRIVATE PentagonCore Qt6::Widgets)
# Start as a GUI app on Windows (no console window behind it); ignored on other platforms.
set_target_properties (DurrPentagonTask PROPERTIES WIN32_EXECUTABLE ON)

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
