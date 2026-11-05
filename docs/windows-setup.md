# Windows (x64) setup

Builds with MSVC and Ninja. Continuous integration (`.github/workflows/windows.yml`) builds and
tests the same configuration on every push.

## 1. Install the tools

- **Visual Studio 2022** (Community or Build Tools), with the *Desktop development with C++*
  workload. This includes CMake and Ninja.
- **Qt 6.5.3**, kit **MSVC 2019 64-bit** (it is ABI-compatible with VS 2022), with the
  **Qt Quick 3D** and **Qt Shader Tools** add-ons. Install it with either:
  - the [Qt online installer](https://www.qt.io/download-qt-installer) (needs a free Qt account), or
  - `aqtinstall` (no account):

    ```bat
    pip install aqtinstall
    aqt install-qt windows desktop 6.5.3 win64_msvc2019_64 -m qtquick3d qtshadertools qtquicktimeline -O C:\Qt
    ```

    If downloads time out, see the `aqt` workaround in [macos-setup.md](macos-setup.md). It works
    the same way on Windows.

## 2. Build and test

Open **x64 Native Tools Command Prompt for VS 2022**. It must be the x64 prompt: the default
Developer prompt builds 32-bit. Then run:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.5.3\msvc2019_64
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest` puts Qt's DLLs on `PATH` for the tests itself. The smoke tests open real windows and render
through Direct3D 11.

The Visual Studio generator (`-G "Visual Studio 17 2022"`) also works. Pass `--config Release` to
`cmake --build`, and `-C Release` to `ctest`. Extensions still land in `build\extensions`.

## 3. Run

From the build tree, Qt's DLLs must be on `PATH`:

```bat
set PATH=C:\Qt\6.5.3\msvc2019_64\bin;%PATH%
build\host\qtplugins-host.exe --extensions-dir build\extensions --open org.qtplugins.mustardmayo
```

## 4. Deploy

```bat
cmake --install build --prefix dist
dist\bin\qtplugins-host.exe
```

On Windows, `cmake --install` also runs `windeployqt` on the host and on each extension, passing
their QML directories. `dist\bin` then contains everything Qt needs, including the Qt Quick 3D QML
modules and the JPEG image plugin, and runs on a machine without Qt. The target machine also needs
the [Microsoft Visual C++ Redistributable](https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist)
(x64), which most Windows machines already have.

The host is a GUI application with no console window, so `qWarning` output isn't visible. Run it
under a debugger, or set `QT_LOGGING_TO_CONSOLE=1` and start it from a console, to see messages.
