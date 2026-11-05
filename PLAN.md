# Implementation Plan

Goal: a minimal, copyable Qt6 host that loads plugins from a directory, plus a tic-tac-toe plugin
with a 3D board — architected so a more complex 3D game can be added later as another plugin.

## Key decisions

| Topic | Decision | Why |
|---|---|---|
| Qt version | 6.4.2 (Ubuntu 24.04 apt); Mac via Qt online installer / `aqtinstall` at 6.4.x–6.5 LTS | Matches brief; binary distributions only |
| Build | CMake + Ninja, `qt_standard_project_setup()` | Qt6's first-class build system |
| Plugin mechanism | `QPluginLoader` + `Q_DECLARE_INTERFACE` / `Q_PLUGIN_METADATA` | Standard Qt plugin system; version/build-key checks for free; metadata readable without loading the library |
| Host UI toolkit | Qt Widgets (`QMainWindow`) | Most likely shape of an existing app; plugin returns a `QWidget*` |
| 3D | Qt Quick 3D in a `QQuickView`, embedded with `QWidget::createWindowContainer` | Qt3D is in maintenance; Quick 3D is the future-proof path for the later, bigger game |
| Plugin assets | QML compiled into the `.so` with `qt_add_resources` (`qrc:/`) | Single-file deployment, no QML import-path setup in host |
| Game logic | Plain C++ `QObject` model, exposed to QML | Unit-testable without a GPU |

## Plugin interface

```cpp
// sdk/include/qtplugins/IExtension.h
class IExtension {
public:
    virtual ~IExtension() = default;
    virtual QWidget *createWidget(QWidget *parent) = 0;  // host owns returned widget via parent
};
#define IExtension_iid "org.qtplugins.IExtension/1"
Q_DECLARE_INTERFACE(IExtension, IExtension_iid)
```

Descriptive info lives only in each plugin's metadata JSON (`id`, `name`, `version`,
`description`, `apiVersion`), so the host lists and validates plugins via
`QPluginLoader::metaData()` before loading them — a library with the wrong IID or API version is
never initialised.

## Layout

```
CMakeLists.txt
sdk/include/qtplugins/IExtension.h      header-only interface (INTERFACE target QtPlugins::Sdk)
host/                                   main.cpp, ExtensionManager.{h,cpp}, MainWindow.{h,cpp}
plugins/tictactoe/                      TicTacToePlugin.{h,cpp}, GameModel.{h,cpp}, qml/*.qml, tictactoe.json
tests/                                  Qt Test: game model, extension loading
scripts/                                setup-linux.sh, smoke test / screenshot helper
```

Build output: all plugins go to `build/extensions/` so the host can be pointed straight at it.

## Phases

### Phase 0 — Toolchain & skeleton
- [x] `scripts/setup-linux.sh`: apt install `build-essential cmake ninja-build qt6-base-dev
      qt6-declarative-dev qt6-quick3d-dev qml6-module-qtquick3d qml6-module-qtquick
      libgl1-mesa-dev mesa-utils xvfb` (+ any QML runtime modules found missing)
- [x] Top-level CMake project, empty `host`/`plugins`/`tests` subdirs, `.gitignore`
- [x] Verify a trivial Qt Quick 3D scene renders under `xvfb-run` (de-risks headless rendering early)
- Commit

### Phase 1 — SDK + minimal host
- [x] `IExtension.h` interface + CMake INTERFACE target
- [x] `ExtensionManager`: scan dir (`--extensions-dir` arg, default `<appdir>/extensions`),
      filter by platform suffix (`QLibrary::isLibrary`), read metadata, load with `QPluginLoader`,
      `qobject_cast<IExtension*>`, collect errors (wrong IID, Qt version mismatch, load failure)
- [x] `MainWindow`: list of loaded extensions (menu or side list) → open selected extension's widget
      in a tab/central area; show load errors in a status area
- [x] Keep host code small and copyable; no dependencies beyond QtWidgets
- Commit

### Phase 2 — Plugin loading tests
- [x] A trivial test plugin (`plugins/hello`, plain QLabel) to prove the pipeline without 3D
- [x] Qt Test for `ExtensionManager`: finds/loads valid plugin, rejects non-plugin `.so`, reports errors
- [x] Wire into `ctest` with `QT_QPA_PLATFORM=offscreen`
- Commit

### Phase 3 — Tic-tac-toe game logic
- [x] `GameModel` (QObject): 3×3 board, current player, `play(index)`, winner/draw detection,
      `reset()`, `Q_PROPERTY`s + signals for QML
- [x] Simple AI opponent option (random or minimax) — optional, nice for single-player demo
- [x] Unit tests for all win lines, draw, illegal moves, reset
- Commit

### Phase 4 — 3D board plugin
- [x] `TicTacToeExtension` implementing `IExtension`; `createWidget()` returns a window container
      around a `QQuickView` loading `qrc:/qtplugins/tictactoe/Board.qml`, `GameModel` exposed as a
      per-view context property
- [x] `Board.qml`: `View3D` with camera, lights + shadows, board base + grid bars, cell picking
      (`View3D.pick()` on the board surface), X from crossed bars / O as a segmented torus,
      drop-in animation, pulsing glow on the winning line, hover highlight, status/score overlay,
      New game + opponent toggle; gentle camera sway
- [x] Headless smoke test (`tests/smoke_tictactoe`, runs under `xvfb-run`): loads the plugin via
      `ExtensionManager`, checks the scene renders, clicks a cell, plays to a win, fails on QML
      warnings, saves screenshots to `build/tests/smoke-*.png`
- Commit

### Phase 5 — Deployment & docs
- [x] `install()` rules: host binary + `extensions/` layout; document `RPATH` / Qt runtime expectations
- [x] README: how to build a new plugin, ABI rules (same Qt/compiler/build type), how to integrate
      `ExtensionManager` + `IExtension.h` into the real host
- [x] Notes for Windows (`windeployqt`, `.dll` suffix) and macOS (`macdeployqt`, `.dylib`, signing)
- Commit

### Phase 6 — macOS (M2) bring-up *(later, interactive)*
- [ ] **Ask permission** before installing Qt / CMake / Ninja on the Mac
      (setup steps, including aqt timeout workaround: `docs/macos-setup.md`)
- [ ] Build & run; verify Metal RHI backend renders the board, tune visuals/interaction
- Commit

### Iteration 2 — "Mustard and Mayonnaise" plugin (`plugins/mustardmayo/`)
From `Plugin Iteration 2 Ideas.md`. Same architecture as tic-tac-toe: rules in a GPU-free C++
library with unit tests, Qt Quick 3D scenes in QML compiled into the plugin.

- [x] Core model (`mustardmayo_core`): `TypingSession` (type the shown C++, every keystroke adds
      fatigue → blur; unreadable at full fatigue), `DrivingSession` (lane position between the
      yellow and white lines, road curvature drift, random vision-blur episodes that disturb
      steering, mustard/mayo warnings, progress only while in lane), `MustardMayoGame` (phase
      state machine: work → drive → destination → home → asleep, or → burger → eat → work)
- [x] Unit tests for all three
- [x] Blur post-effect: custom `ShaderEffect` with a `qt_add_shaders`-compiled shader (no extra
      QML module dependency; works on Qt 6.4)
- [x] Scenes: office at night (monitor showing the code as a live texture, keyboard, out-of-focus
      circuit cards and wires as a separately blurred layer), night drive (curved road, dashed
      yellow / solid white lines, headlights, dashboard + steering wheel), road-sign fork,
      house, burger joint, bedtime card
- [x] Smoke test driving every phase through real key input + screenshots
- [x] Host: give an opened extension keyboard focus
- Commit after the model + tests, and again after the scenes + smoke test

### Phase 7 — Complex 3D game *(future)*
- Separate plugin on the same interface; revisit interface needs then (e.g. host services,
  settings, input focus, lifecycle hooks like pause/resume) and bump the IID version if changed.

## Risks / open questions

- **Host toolkit:** plan assumes the real host is Qt Widgets. If it's QML-based, add a
  `createQuickItem()`-style entry point (or a QML module URL) to the interface — cheap to do now.
- **Headless rendering:** Quick 3D needs OpenGL; on the VM this means Mesa llvmpipe under Xvfb.
  Verified in Phase 0 so it doesn't surprise us in Phase 4.
- **Window container vs `QQuickWidget` (decided):** in Qt < 6.7 `QQuickWidget` only renders via
  OpenGL (no Metal on macOS) and adds an offscreen copy, so the plugin uses `QQuickView` +
  `createWindowContainer`, which uses the native RHI backend. Trade-off: a native child window
  can't be overlapped by sibling widgets and has some focus/stacking quirks.
- **ABI coupling:** Qt plugins are not stable across compilers/Qt minor versions in practice —
  plugins must ship per platform and per host Qt build.
