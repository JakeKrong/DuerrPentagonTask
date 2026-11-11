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

## Using the editor

The canvas (the white area) shows the pentagon and its label. Every change, made in either place,
shows up on the canvas and in the properties list straight away.

On the canvas:

- **Drag a white corner handle** to move that point.
- **Drag inside the polygon** to move the whole shape.
- **Double-click the label** to edit its text. Enter starts a new line; press Escape or click
  elsewhere to finish. An empty label shows a grey "Double-click to add text" hint.
- The shape can't leave the canvas: points and drags stop at its edge.

In the properties list:

| Properties:
| Fill colour : Click the swatch to pick a colour, or type `#RRGGBB` in the field next to it |
| Area : Type a new area and press Enter: the polygon is scaled about the centre of its bounding box, up to the size that fits on the canvas |
| Point 1 to 5 : X and Y in canvas coordinates (origin in the middle, y pointing down). Type a value and press Enter, or use the arrows or mouse wheel |
| Text : The label's text; several lines are allowed |

**Reset polygon** (the button under the points) puts the polygon back to its starting shape, size,
position and colour. The label text is kept.

## How it works

All the data lives in one place, `PentagonModel`: the 5 points, the fill colour and the label
text. The area isn't stored; it's always calculated from the points.

```
                 ┌──────────────────────────┐
                 │      PentagonModel       │  5 points, fill colour, text
                 │   (QObject, no UI code)  │  setters + "changed" signals
                 └──────────────────────────┘
              set ▲    │ signals    signals │    ▲ set
                  │    ▼                    ▼    │
     ┌────────────────────────┐    ┌────────────────────────┐
     │  Canvas (scene + view) │    │    Properties panel    │
     │  polygon, handles,     │    │  colour, area,         │
     │  label                 │    │  5 points, text        │
     └────────────────────────┘    └────────────────────────┘
```

The canvas and the panel never talk to each other. Every edit goes through a model setter, the model
emits a signal, and both sides update from it. That's what keeps them consistent. Updates can't
bounce back and forth forever because:

- the model's setters ignore values that haven't changed, so they only emit on a real change;
- widgets updated from the model don't report it as a user edit. The editor widgets have separate
  `...Edited` signals, and spin boxes are updated under a `QSignalBlocker`. This matters because a
  spin box rounds to 2 decimals, and feeding that rounded value back would nudge the shape;
- text fields are only replaced when their text differs from the model's, so the cursor doesn't
  jump while you type.

Other choices worth knowing about:

- **Points are in scene coordinates.** The polygon item always stays at (0,0). Dragging it moves the
  model's points rather than the item, so the point values in the panel change with the drag.
- **The canvas is `PentagonModel::Bounds`** (800 × 600 units), which is also the scene rect. The model
  keeps every point inside it, the panel's spin boxes use the same limits, and the view scales so
  the whole canvas is always visible.
- **Area** uses the shoelace formula. Setting it scales every point about the centre of the
  bounding box by √(new area / old area).
- The code is built as two libraries plus a tiny app: `PentagonCore` (the model, no widget code)
  and `PentagonUi` (the window, canvas and panel). The app is just `main.cpp`. This lets the tests
  use the model on its own, or create the real window.

## Known limitations

- **Self-intersecting shapes:** if you drag a corner across the opposite side, the polygon is still
  filled solid (winding fill rule), but the area shown is smaller than what you see. The shoelace
  formula subtracts the part that winds the other way.
- The fill colour is opaque; there's no transparency setting.
- There's no undo.
- With multi-line label text, each line is left-aligned within the label.
- The automatic Qt download only works for MSVC x64 builds (see above for using your own Qt).

## Running the tests

There are two test programs (Qt Test), built along with the app:

- **`tst_pentagonmodel`** tests the model on its own: the starting shape, the area formula
  (including a self-intersecting case), each setter and its signals, the "no signal when nothing
  changed" rule, and keeping points on the canvas.
- **`tst_editor`** creates the real main window and drives it with mouse and keyboard events, the
  way a user would. It drags the polygon and its handles, edits the label on the canvas, and types
  into each property. After each step it checks that the canvas, the properties panel and the model
  all agree. It runs with Qt's off-screen platform, so no windows appear.

After building, run them with CTest:

```bat
ctest --test-dir out/build/x64-release --output-on-failure
```

For a build made without presets, add the configuration: `ctest --test-dir build -C Release`.
To see each individual test case, add `-V`.

## Project layout

```
cmake/FetchQt.cmake      downloads and caches Qt on the first configure
src/
  PentagonModel.*        the data (5 points, fill colour, label text); the PentagonCore library
  main.cpp               entry point; the only file in the app itself
  MainWindow.*           main window: owns the model, canvas in the centre, properties docked on the right
                         (this and everything below is the PentagonUi library)
  canvas/
    CanvasView.*         the view: fits the whole canvas in the window
    PolygonItem.*        the filled polygon; drag it to move the whole shape
    VertexHandle.*       draggable dot on each corner
    LabelItem.*          the text label, editable in place
  properties/
    PropertiesPanel.*    the properties list, connected to the model
    ColorProperty.*      colour editor (swatch + hex)
    PointProperty.*      point editor (X and Y)
tests/
  tst_pentagonmodel.cpp  unit tests for the model
  tst_editor.cpp         tests of the whole window: canvas, panel and model stay consistent
```
