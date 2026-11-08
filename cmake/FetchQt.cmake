# FetchQt.cmake : makes Qt available to this project without a manual Qt install.
#
# On the first configure, the prebuilt Qt binaries for MSVC 2022 x64 are downloaded from Qt's
# official server (download.qt.io), checked against the SHA-1 hashes pinned below and unpacked
# into DURR_QT_CACHE_DIR. Every build preset (Debug, Release, ...) reuses that copy, so the
# download happens once per checkout. The Qt directory is then added to CMAKE_PREFIX_PATH,
# so a plain find_package(Qt6 ...) picks it up.
#
# Options (pass with -D, or set them in CMakeUserPresets.json):
#   DURR_FETCH_QT=OFF        Use a Qt you installed yourself; point CMAKE_PREFIX_PATH at it.
#   DURR_QT_CACHE_DIR=<dir>  Unpack Qt somewhere else, e.g. to share one copy between clones.
#   DURR_QT_MIRROR=<url>     Download from a specific Qt mirror instead of download.qt.io.
#
# To add a Qt module (e.g. qtsvg), add it to _durr_qt_archives together with the SHA-1 that Qt
# publishes next to the archive (<archive URL>.sha1).

option(DURR_FETCH_QT "Download a pinned Qt automatically instead of using an installed one" ON)
set(DURR_QT_CACHE_DIR "${PROJECT_SOURCE_DIR}/out/qt" CACHE PATH "Where the downloaded Qt is unpacked")
set(DURR_QT_MIRROR "https://download.qt.io/online/qtsdkrepository" CACHE STRING "Base URL of the Qt online repository")

if(NOT DURR_FETCH_QT)
  return()
endif()

# --- Pinned Qt release ---------------------------------------------------------------------
# Taken from <mirror>/windows_x86/desktop/qt6_6113/qt6_6113_msvc2022_64/Updates.xml
set(DURR_QT_VERSION "6.11.3")
set(_durr_qt_release "6.11.3-0-202610220312")
set(_durr_qt_arch "msvc2022_64")
set(_durr_qt_archive_suffix "Windows-Windows_11_24H2-MSVC2022-Windows-Windows_11_24H2-X86_64.7z")
set(_durr_qt_archives
  # module=sha1
  "qtbase=2df6e5efa3d116e47f289cd120730da4842159db"
)

if(NOT (MSVC AND CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "x64"))
  message(WARNING
    "Automatic Qt download is only set up for MSVC x64 builds "
    "(this build is ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}). "
    "Install Qt ${DURR_QT_VERSION} yourself, point CMAKE_PREFIX_PATH at it, and configure with -DDURR_FETCH_QT=OFF.")
  return()
endif()

if(MSVC_VERSION LESS 1930)
  message(WARNING "The downloaded Qt is built with Visual Studio 2022; older MSVC versions may fail to link against it.")
endif()

function(_durr_fetch_qt out_root)
  set(root "${DURR_QT_CACHE_DIR}/${DURR_QT_VERSION}/${_durr_qt_arch}")
  set(stamp "${root}/.durr-fetch-qt.stamp")
  set(stamp_content "${_durr_qt_release};${_durr_qt_archives}")
  set(${out_root} "${root}" PARENT_SCOPE)

  if(EXISTS "${stamp}")
    file(READ "${stamp}" have)
    if(have STREQUAL stamp_content)
      return()
    endif()
  endif()

  # Two configures sharing the cache (e.g. Debug and Release at once) must not unpack over each other.
  file(MAKE_DIRECTORY "${DURR_QT_CACHE_DIR}")
  file(LOCK "${DURR_QT_CACHE_DIR}/.lock" GUARD FUNCTION TIMEOUT 1800)
  if(EXISTS "${stamp}")
    file(READ "${stamp}" have)
    if(have STREQUAL stamp_content)
      return()
    endif()
  endif()

  string(REPLACE "." "" ver_id "${DURR_QT_VERSION}")
  set(base_url "${DURR_QT_MIRROR}/windows_x86/desktop/qt6_${ver_id}/qt6_${ver_id}_${_durr_qt_arch}/qt.qt6.${ver_id}.win64_${_durr_qt_arch}")
  set(staging "${root}.partial")
  file(REMOVE_RECURSE "${root}" "${staging}")

  set(archives "")
  foreach(entry IN LISTS _durr_qt_archives)
    string(REPLACE "=" ";" entry "${entry}")
    list(GET entry 0 module)
    list(GET entry 1 expected_sha1)
    set(file_name "${_durr_qt_release}${module}-${_durr_qt_archive_suffix}")
    set(archive "${DURR_QT_CACHE_DIR}/${file_name}")
    list(APPEND archives "${archive}")

    # An archive left over from an interrupted configure is reused if it is intact.
    set(actual_sha1 "")
    if(EXISTS "${archive}")
      file(SHA1 "${archive}" actual_sha1)
    endif()
    if(NOT actual_sha1 STREQUAL expected_sha1)
      message(STATUS "Downloading Qt ${DURR_QT_VERSION} ${module} (one-time, cached in ${DURR_QT_CACHE_DIR})")
      file(DOWNLOAD "${base_url}/${file_name}" "${archive}"
        TLS_VERIFY ON
        INACTIVITY_TIMEOUT 120
        STATUS status)
      list(GET status 0 status_code)
      if(NOT status_code EQUAL 0)
        file(REMOVE "${archive}")
        list(GET status 1 status_message)
        message(FATAL_ERROR
          "Downloading Qt failed: ${status_message}\n  URL: ${base_url}/${file_name}\n"
          "Check your internet connection, try another mirror with -DDURR_QT_MIRROR=<url>, "
          "or use an installed Qt with -DDURR_FETCH_QT=OFF.")
      endif()
      file(SHA1 "${archive}" actual_sha1)
      if(NOT actual_sha1 STREQUAL expected_sha1)
        file(REMOVE "${archive}")
        message(FATAL_ERROR
          "Downloaded ${file_name} has SHA-1 ${actual_sha1}, expected ${expected_sha1}. "
          "The download was corrupted or the mirror served a different file; configure again to retry.")
      endif()
    endif()

    message(STATUS "Unpacking Qt ${DURR_QT_VERSION} ${module}")
    file(ARCHIVE_EXTRACT INPUT "${archive}" DESTINATION "${staging}")
  endforeach()

  # The archives unpack either straight into the Qt prefix or into <version>/<arch>/ below it.
  set(unpacked "${staging}")
  if(NOT EXISTS "${unpacked}/lib/cmake/Qt6/Qt6Config.cmake")
    set(unpacked "${staging}/${DURR_QT_VERSION}/${_durr_qt_arch}")
  endif()
  if(NOT EXISTS "${unpacked}/lib/cmake/Qt6/Qt6Config.cmake")
    message(FATAL_ERROR "Unpacked Qt in ${staging} does not contain lib/cmake/Qt6/Qt6Config.cmake.")
  endif()

  file(RENAME "${unpacked}" "${root}")
  file(REMOVE_RECURSE "${staging}")
  file(WRITE "${stamp}" "${stamp_content}")
  file(REMOVE ${archives})
endfunction()

_durr_fetch_qt(DURR_QT_ROOT)
list(PREPEND CMAKE_PREFIX_PATH "${DURR_QT_ROOT}")
message(STATUS "Using Qt ${DURR_QT_VERSION} from ${DURR_QT_ROOT}")
