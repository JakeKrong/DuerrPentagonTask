# DurrPentagonTask

A Qt Widgets editor for a five-sided polygon and its text label. The shape is shown on a canvas
(Qt's graphics view framework) next to a properties list, and editing either one updates the other.

## Requirements

- Windows 10 or 11, x64
- Visual Studio 2022 or newer with the **Desktop development with C++** workload, which provides
  the MSVC compiler, CMake (3.22 or newer) and Ninja
- Git
- An internet connection the first time you configure

Qt does not need to be installed. On the first configure, CMake downloads Qt 6.11.3 (prebuilt for
MSVC x64) from `download.qt.io`, checks it against a pinned SHA-1 and caches it in `out/qt/`. Later
configures reuse that copy. See [cmake/FetchQt.cmake](cmake/FetchQt.cmake) for the details.

## Build and run

1. Open **x64 Native Tools Command Prompt for VS** from the Start menu. It has to be the x64 one:
   the downloaded Qt is 64-bit, and the plain "Developer Command Prompt" targets 32-bit by default.

2. Clone and build:

   ```bat
   git clone https://github.com/JakeKrong/DurrPentagonTask.git
   cd DurrPentagonTask
   cmake --preset x64-release
   cmake --build out/build/x64-release
   ```

3. Run:

   ```bat
   out\build\x64-release\DurrPentagonTask.exe
   ```

The build copies the Qt DLLs next to the .exe (using `windeployqt`), so the program runs from the
build folder as is. For a debug build, use the `x64-debug` preset and `out/build/x64-debug` instead.

### Without presets

```bat
cmake -S . -B build
cmake --build build --config Release
build\Release\DurrPentagonTask.exe
```

This uses CMake's default Visual Studio generator, which puts the program in a folder per
configuration (`Release`, `Debug`).

### Using a Qt you installed yourself

Any Qt 6.5 or newer with the Widgets module works. Turn off the download and point CMake at your Qt:

```bat
cmake -S . -B build -DDURR_FETCH_QT=OFF -DCMAKE_PREFIX_PATH=C:/Qt/<version>/msvc2022_64
cmake --build build --config Release
```

The automatic download only covers MSVC x64. On other compilers or platforms, install Qt and pass
`CMAKE_PREFIX_PATH` the same way.

## Running the tests

The unit tests are built along with the app. After building, run them with CTest:

```bat
ctest --test-dir out/build/x64-release --output-on-failure
```

For a build made without presets, add the configuration: `ctest --test-dir build -C Release`.

## Project layout

```
cmake/FetchQt.cmake      downloads and caches Qt on the first configure
src/
  PentagonModel.*        the data (5 points, fill colour, label text); built as the PentagonCore library
  main.cpp               entry point
  MainWindow.*           main window: owns the model, canvas in the centre, properties docked on the right
  canvas/
    PolygonItem.*        the filled polygon; drag it to move the whole shape
    VertexHandle.*       draggable dot on each corner
    LabelItem.*          the text label, editable in place
  properties/
    PropertiesPanel.*    the properties list, connected to the model
    ColorProperty.*      colour editor (swatch + hex)
    PointProperty.*      point editor (X and Y)
tests/
  tst_pentagonmodel.cpp  unit tests for the model (Qt Test)
```
