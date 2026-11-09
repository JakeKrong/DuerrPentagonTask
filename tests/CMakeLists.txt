#Unit tests (Qt Test), run with ctest.

find_package (Qt6 REQUIRED COMPONENTS Test)

qt_add_executable (tst_pentagonmodel tst_pentagonmodel.cpp)
target_link_libraries (tst_pentagonmodel PRIVATE PentagonCore Qt6::Test)

add_test (NAME tst_pentagonmodel COMMAND tst_pentagonmodel)
#windeployqt only runs for the app, so put Qt's own bin folder on PATH while ctest runs the test
set_tests_properties (tst_pentagonmodel PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
