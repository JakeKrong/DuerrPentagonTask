#Unit tests (Qt Test), run with ctest.

find_package (Qt6 REQUIRED COMPONENTS Test)

#Model tests: no UI at all
qt_add_executable (tst_pentagonmodel tst_pentagonmodel.cpp)
target_link_libraries (tst_pentagonmodel PRIVATE PentagonCore Qt6::Test)

add_test (NAME tst_pentagonmodel COMMAND tst_pentagonmodel)
#windeployqt only runs for the app, so put Qt's own bin folder on PATH while ctest runs the test
set_tests_properties (tst_pentagonmodel PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")

#Editor tests: the real main window, driven with mouse and keyboard events
qt_add_executable (tst_editor tst_editor.cpp)
target_link_libraries (tst_editor PRIVATE PentagonUi Qt6::Test)

add_test (NAME tst_editor COMMAND tst_editor)
#Run it with Qt's off-screen platform, so no windows appear (Qt's own plugin folder has it; windeployqt doesn't copy it)
set (_editor_test_env
  "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
  "QT_QPA_PLATFORM=set:offscreen"
  "QT_QPA_PLATFORM_PLUGIN_PATH=set:$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>")
if (WIN32)
  #The off-screen platform doesn't use the Windows font system, so point it at the font folder; without this,
  #text is drawn as boxes and measures wrongly
  file (TO_CMAKE_PATH "$ENV{WINDIR}/Fonts" _windows_fonts)
  list (APPEND _editor_test_env "QT_QPA_FONTDIR=set:${_windows_fonts}")
endif()
set_tests_properties (tst_editor PROPERTIES ENVIRONMENT_MODIFICATION "${_editor_test_env}")
