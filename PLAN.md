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
| 3D | Qt Quick 3D, hosted in the plugin's widget via `QQuickWidget` | Qt3D is in maintenance; Quick 3D is the future-proof path for the later, bigger game |
| Plugin assets | QML compiled into the `.so` with `qt_add_resources` (`qrc:/`) | Single-file deployment, no QML import-path setup in host |
| Game logic | Plain C++ `QObject` model, exposed to QML | Unit-testable without a GPU |

## Plugin interface (sketch)

```cpp
// sdk/include/qtplugins/IExtension.h
class IExtension {
public:
    virtual ~IExtension() = default;
    virtual QString id() const = 0;          // stable, e.g. "org.example.tictactoe"
    virtual QString displayName() const = 0;
    virtual QWidget *createWidget(QWidget *parent) = 0;  // host owns returned widget via parent
};
#define IExtension_iid "org.qtplugins.IExtension/1.0"
Q_DECLARE_INTERFACE(IExtension, IExtension_iid)
```

Each plugin also ships a metadata JSON (`name`, `version`, `apiVersion`, `description`) so the host
can list plugins via `QPluginLoader::metaData()` before instantiating them.

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
- [ ] `scripts/setup-linux.sh`: apt install `build-essential cmake ninja-build qt6-base-dev
      qt6-declarative-dev qt6-quick3d-dev qml6-module-qtquick3d qml6-module-qtquick
      libgl1-mesa-dev mesa-utils xvfb` (+ any QML runtime modules found missing)
- [ ] Top-level CMake project, empty `host`/`plugins`/`tests` subdirs, `.gitignore`
- [ ] Verify a trivial Qt Quick 3D scene renders under `xvfb-run` (de-risks headless rendering early)
- Commit

### Phase 1 — SDK + minimal host
- [ ] `IExtension.h` interface + CMake INTERFACE target
- [ ] `ExtensionManager`: scan dir (`--extensions-dir` arg, default `<appdir>/extensions`),
      filter by platform suffix (`QLibrary::isLibrary`), read metadata, load with `QPluginLoader`,
      `qobject_cast<IExtension*>`, collect errors (wrong IID, Qt version mismatch, load failure)
- [ ] `MainWindow`: list of loaded extensions (menu or side list) → open selected extension's widget
      in a tab/central area; show load errors in a status area
- [ ] Keep host code small and copyable; no dependencies beyond QtWidgets
- Commit

### Phase 2 — Plugin loading tests
- [ ] A trivial test plugin (`plugins/hello`, plain QLabel) to prove the pipeline without 3D
- [ ] Qt Test for `ExtensionManager`: finds/loads valid plugin, rejects non-plugin `.so`, reports errors
- [ ] Wire into `ctest` with `QT_QPA_PLATFORM=offscreen`
- Commit

### Phase 3 — Tic-tac-toe game logic
- [ ] `GameModel` (QObject): 3×3 board, current player, `play(index)`, winner/draw detection,
      `reset()`, `Q_PROPERTY`s + signals for QML
- [ ] Simple AI opponent option (random or minimax) — optional, nice for single-player demo
- [ ] Unit tests for all win lines, draw, illegal moves, reset
- Commit

### Phase 4 — 3D board plugin
- [ ] `TicTacToePlugin` implementing `IExtension`; `createWidget()` returns a `QQuickWidget`
      loading `qrc:/tictactoe/Board.qml` with the `GameModel` exposed (initial property / context)
- [ ] `Board.qml`: `View3D` with camera, lights, board base + grid bars, 9 pickable cells
      (`View3D.pick()` on click), X/O pieces built from primitive meshes (cylinders/torus-ish via
      `#Cylinder`/custom), win highlight, reset button overlay; subtle camera orbit
- [ ] Headless smoke test: run host under `xvfb-run`, load plugin, grab a screenshot to verify
      the scene renders
- Commit

### Phase 5 — Deployment & docs
- [ ] `install()` rules: host binary + `extensions/` layout; document `RPATH` / Qt runtime expectations
- [ ] README: how to build a new plugin, ABI rules (same Qt/compiler/build type), how to integrate
      `ExtensionManager` + `IExtension.h` into the real host
- [ ] Notes for Windows (`windeployqt`, `.dll` suffix) and macOS (`macdeployqt`, `.dylib`, signing)
- Commit

### Phase 6 — macOS (M2) bring-up *(later, interactive)*
- [ ] **Ask permission** before installing Qt / CMake / Ninja on the Mac
- [ ] Build & run; verify Metal RHI backend renders the board, tune visuals/interaction
- Commit

### Phase 7 — Complex 3D game *(future)*
- Separate plugin on the same interface; revisit interface needs then (e.g. host services,
  settings, input focus, lifecycle hooks like pause/resume) and bump the IID version if changed.

## Risks / open questions

- **Host toolkit:** plan assumes the real host is Qt Widgets. If it's QML-based, add a
  `createQuickItem()`-style entry point (or a QML module URL) to the interface — cheap to do now.
- **Headless rendering:** Quick 3D needs OpenGL; on the VM this means Mesa llvmpipe under Xvfb.
  Verified in Phase 0 so it doesn't surprise us in Phase 4.
- **`QQuickWidget` vs `createWindowContainer`:** `QQuickWidget` composes cleanly with widgets but
  renders via an offscreen FBO (some cost). Fine for tic-tac-toe; revisit for the heavier game.
- **ABI coupling:** Qt plugins are not stable across compilers/Qt minor versions in practice —
  plugins must ship per platform and per host Qt build.
