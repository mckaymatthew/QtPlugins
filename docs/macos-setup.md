# macOS (Apple silicon) setup

How to build and run the project on a Mac (M1/M2/…). As of this writing it has only been built
on Linux, so the first macOS build may need small fixes (see *Known risks* below).

## Option A: let Claude Code do it

```bash
cd QtPlugins
claude
```

Then ask it to *"do Phase 6 from PLAN.md"*. It reads `CLAUDE.md`, so it asks before installing
anything on the Mac.

## Option B: by hand

### 1. Install the tools

Skip anything you already have.

- **Xcode command line tools:** `xcode-select --install`
- **CMake + Ninja:** `brew install cmake ninja`, or tick them under *Developer and Designer Tools*
  in the Qt online installer.
- **Qt 6.5.3** (the long-term-support release closest to the project's 6.4 target), with the
  **Qt Quick 3D** and **Qt Shader Tools** add-ons. Use either:
  - the [Qt online installer](https://www.qt.io/download-qt-installer) (needs a free Qt account), or
  - `aqtinstall` (no account):

    ```bash
    pip3 install aqtinstall
    aqt install-qt mac desktop 6.5.3 clang_64 -m qtquick3d qtshadertools qtquicktimeline -O ~/Qt
    ```

#### If `aqt` fails with `ReadTimeoutError` / `Connection error`

aqt's default timeouts are short, and its fallback mirrors can be slow. Give it longer timeouts and
point it at one mirror you know is fast:

```bash
cat > aqt-settings.ini <<'EOF'
[requests]
connection_timeout: 30
response_timeout: 300
max_retries_on_connection_error: 10
retry_backoff: 2
EOF

aqt -c aqt-settings.ini install-qt mac desktop 6.5.3 clang_64 \
    -m qtquick3d qtshadertools qtquicktimeline -O ~/Qt \
    --base https://mirrors.ocf.berkeley.edu/qt/
```

Other mirrors to try with `--base`: `https://qt.mirror.constant.com/` and
`https://mirror.netcologne.de/qtproject/`. The full list is at
<https://download.qt.io/static/mirrorlist/>. It's safe to re-run after a partial failure.

Check the install afterwards: `~/Qt/6.5.3/macos/lib/QtQuick3D.framework` must exist.

### 2. Build and run the tests

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=~/Qt/6.5.3/macos
cmake --build build
ctest --test-dir build --output-on-failure
```

Adjust `CMAKE_PREFIX_PATH` if Qt is somewhere else. On macOS the rendering smoke test runs
directly, without Xvfb, so a window flashes up briefly. Screenshots land in
`build/tests/smoke-*.png`.

### 3. Run the game

```bash
QSG_INFO=1 build/host/qtplugins-host.app/Contents/MacOS/qtplugins-host \
    --extensions-dir build/extensions --open org.qtplugins.tictactoe
```

`QSG_INFO=1` logs the graphics backend Qt picked. It should say **Metal**.

## Known risks on first macOS build

- **`ld: framework 'AGL' not found`.** Qt ≤ 6.5.3 links the AGL framework, which newer Xcode SDKs
  no longer include. The top-level `CMakeLists.txt` strips it out. If you still hit this error,
  delete `build/` and configure again.
- **Plugin file suffix.** CMake may name plugins `.so` rather than `.dylib` on macOS. The host
  accepts both.
- **App bundle paths.** The installed host looks in `Contents/PlugIns/extensions`; in the build
  tree, pass `--extensions-dir` as shown above.
- **Metal rendering differences.** Lighting, shadows and MSAA can look different from Mesa/OpenGL
  on Linux. Compare against `docs/tictactoe.png`.
- **Code signing.** Local builds are ad-hoc signed and load fine. For distribution, the host and
  every plugin must be signed with the same team identity (see README *Deployment*).

If something fails, paste the output into a Claude Code session on the Mac, and commit fixes back to
`main`.
