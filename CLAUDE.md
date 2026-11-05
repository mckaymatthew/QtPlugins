# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

Deployable Qt6 plugins (`.so` on Linux, `.dll` on Windows, `.dylib` on macOS) that load into an
existing host application at runtime. The repo contains:

- **sdk/** – the plugin interface header(s). This is the contract between host and plugins; keep it
  tiny, header-only, and dependency-light (QtCore + QtWidgets only).
- **host/** – a *minimal* example host app that loads extensions from a directory. Its code is meant
  to be copied into the real host application later, so keep it small, self-contained, and free of
  example-only cleverness.
- **plugins/tictactoe/** – example plugin: tic-tac-toe with a 3D-rendered board (Qt Quick 3D).
- **plugins/mustardmayo/** – "Mustard and Mayonnaise" (spec: `Plugin Iteration 2 Ideas.md`). It
  has typing, driving and destination phases, and a custom blur `ShaderEffect` built with
  `qt_add_shaders`. A `Texture { sourceItem: ... }` doesn't reliably re-render after a property-only
  change in Qt 6.4, so drive transient effects through 3D materials instead.
- Later: a more complex 3D-accelerated game built on the same plugin architecture.

See `Project.MD` for the original brief and `PLAN.md` for the implementation plan and status.

## Constraints

- **Qt version:** target Qt 6.4.2 (Ubuntu 24.04 apt ships exactly this). Newer 6.x is OK on other
  platforms (e.g. 6.5 LTS), but don't jump to a much newer series and don't use APIs added after 6.4
  without a version guard. Never build Qt from source — use binary distributions only.
- **3D:** use Qt Quick 3D (not Qt3D, which is in maintenance mode).
- **Build system:** CMake (>= 3.22) + Ninja. No qmake.
- **Plugin ABI:** host and plugins must be built with the same Qt version, compiler, and build type
  (Debug/Release). Bump `IExtension` IID version string whenever the interface changes.
- **Plugin self-containment:** a plugin must bundle its own QML/assets (Qt resources compiled into
  the shared library) so deployment is a single file dropped into the extensions directory.

## Environments

- **Linux (current):** headless Ubuntu 24.04 VM. OK to install packages with apt. There is no
  display or GPU: use `QT_QPA_PLATFORM=offscreen` for tests, and `xvfb-run` + Mesa (llvmpipe) for
  anything that must actually render (screenshots/smoke tests).
- **macOS (later, M2):** interactive graphics work. **Always ask the user before installing anything
  on the Mac** (Homebrew packages, Qt installers, pip packages, etc.).

## Commands

```bash
scripts/setup-linux.sh                       # install toolchain + Qt 6.4.2 dev packages (apt)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure   # unit tests (offscreen) + 3D smoke test (xvfb-run)
build/host/qtplugins-host --extensions-dir build/extensions [--open org.qtplugins.tictactoe]
cmake --install build --prefix <dir>         # bin/qtplugins-host + bin/extensions/*.so
scripts\build-windows.ps1 -Test -Deploy      # Windows: MSVC 2019 x64 build (see docs/windows-setup.md)
```

The smoke tests save screenshots to `build/tests/smoke-*.png` (tic-tac-toe) and
`build/tests/mm-*.png` (Mustard and Mayonnaise). Look at them after any visual change. Quick 3D needs OpenGL, so anything rendering must run under `xvfb-run` here.

New extensions use `qtplugins_add_extension()` (`cmake/QtPluginsExtension.cmake`) and must give
their `qt_add_resources` a unique prefix (`/qtplugins/<name>`). See README.md for details.

(Update this section if the commands change.)

## Workflow

- **Commit after every major chunk of work** (e.g. each plan phase/step that builds and passes
  tests). Use clear messages; don't batch unrelated work into one commit.
- Keep `PLAN.md` status checkboxes current as steps complete.
- Before committing, make sure the project configures, builds, and `ctest` passes.
