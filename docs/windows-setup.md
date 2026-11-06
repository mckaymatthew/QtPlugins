# Windows (x64) setup

The Windows target is **MSVC 2019 x64** (toolset v142, compiler 19.29), matching Qt's
`msvc2019_64` kit. Continuous integration (`.github/workflows/windows.yml`) builds and tests
exactly that configuration on every push. Build the host and every extension with the same toolset.

## 1. Install the tools

- **The MSVC 2019 x64 compiler.** Install either:
  - **Visual Studio 2019** (Community or Build Tools) with the *Desktop development with C++*
    workload, or
  - **Visual Studio 2022** with *Desktop development with C++*, plus the individual component
    *MSVC v142 - VS 2019 C++ x64/x86 build tools*.

  Both include CMake and Ninja.
- **Qt 6.5.3**, kit **MSVC 2019 64-bit**, with the
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

### The easy way: `scripts\build-windows.ps1`

From any PowerShell prompt (no Developer prompt needed):

```powershell
scripts\build-windows.ps1 -Test                 # configure, build, run the tests
scripts\build-windows.ps1 -Test -Deploy -Run    # ...then deploy to dist\ and launch the game
scripts\build-windows.ps1 -QtDir D:\Qt\6.5.3\msvc2019_64 -Config Debug -Clean
scripts\build-windows.ps1 -Test -Run -Software # software rendering (VMs, no GPU driver)
Get-Help scripts\build-windows.ps1 -Detailed    # all options
```

The script finds the MSVC 2019 (v142) toolset through `vswhere`, loads its x64 environment, checks
that `cl` is 19.29 for x64, and finds Qt under `C:\Qt` (or uses `-QtDir`). If PowerShell refuses to
run scripts, use `powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1 -Test`. CI
builds through this script, under both PowerShell 7 and Windows PowerShell 5.1.

### By hand

Open a prompt set up for the MSVC 2019 x64 compiler:

- **VS 2019:** *x64 Native Tools Command Prompt for VS 2019*.
- **VS 2022:** any command prompt, then run
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.29`.
  Adjust the path for your edition.

`cl` should report `Version 19.29` and `for x64`. Then run:

```bat
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:\Qt\6.5.3\msvc2019_64
cmake --build build
ctest --test-dir build --output-on-failure
```

`ctest` puts Qt's DLLs on `PATH` for the tests itself. The smoke tests open real windows and render
through Direct3D 11.

CMake warns at configure time if the compiler isn't MSVC 2019 x64.

The Visual Studio generators also work: `-G "Visual Studio 16 2019" -A x64`, or with VS 2022,
`-G "Visual Studio 17 2022" -A x64 -T v142`. Pass `--config Release` to `cmake --build`, and
`-C Release` to `ctest`. Extensions still land in `build\extensions`.

## 3. Run

From the build tree, Qt's DLLs must be on `PATH`:

```bat
set PATH=C:\Qt\6.5.3\msvc2019_64\bin;%PATH%
build\host\qtplugins-host.exe --extensions-dir build\extensions --open org.qtplugins.mustardmayo
```

## Graphics: no Vulkan or OpenGL needed

On Windows, Qt Quick and Qt Quick 3D render through **Direct3D 11**, which is part of Windows.
Neither building nor running needs the Vulkan SDK, Vulkan runtime, or the Microsoft Store
"OpenCL, OpenGL, and Vulkan Compatibility Pack".

- CMake prints `Could NOT find WrapVulkanHeaders (missing: Vulkan_INCLUDE_DIR)` while configuring.
  This is only a notice: Qt checks for optional Vulkan headers. The build doesn't use them, and CI
  builds without them.
- Shaders (`blur.frag`) are compiled by Qt's own `qsb` tool, which has its shader compilers built
  in. It doesn't need a Vulkan or DirectX SDK.

**Software rendering.** If the machine has no working GPU driver (a VM, a remote desktop session,
or Windows on ARM under emulation), use WARP, Direct3D 11's software rasterizer. It also ships with
Windows:

```bat
set QSG_RHI_BACKEND=d3d11
set QSG_RHI_PREFER_SOFTWARE_RENDERER=1
build\host\qtplugins-host.exe --extensions-dir build\extensions
```

`scripts\build-windows.ps1 -Software` sets these for `-Test` and `-Run`. They work the same for
the deployed `dist\bin\qtplugins-host.exe`. WARP is slower than a GPU but handles both games.
CI runs every test this way.

To see which backend and adapter Qt picked, set `QSG_INFO=1` and `QT_LOGGING_TO_CONSOLE=1`, then run
the host from a console.

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
