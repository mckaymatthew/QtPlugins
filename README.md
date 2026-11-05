# QtPlugins

Runtime-loadable Qt 6 extensions (`.so` / `.dll` / macOS plugin) and a minimal host that loads them
from a directory. There are two example extensions, both rendered with Qt Quick 3D:

- **Tic-tac-toe** on a 3D board, against a friend or the computer.
- **Mustard and Mayonnaise**, a three-phase game:
  1. At work late at night, type C++ until you're too tired to read it.
  2. Drive home on a winding road at a wandering speed. Keep the car between the yellow line
     ("Get out the mustard!") and the white line ("Stay off the mayo!") while your vision keeps
     blurring.
  3. Go home to bed, or stop at What-A-Burger for a cheeseburger you may not want to look at too
     closely, then head back to work.

![Tic-tac-toe extension](docs/tictactoe.png)
![Mustard and Mayonnaise extension](docs/mustardmayo.png)

```
sdk/include/qtplugins/IExtension.h   the host ⇄ extension contract (header-only)
host/                                example host: ExtensionManager (reusable) + MainWindow (demo)
plugins/hello/                       smallest possible extension
plugins/tictactoe/                   GameModel (rules + minimax AI) and the 3D QML board
plugins/mustardmayo/                 typing/driving/phase models, six 3D scenes, a blur shader
tests/                               unit tests + an Xvfb rendering smoke test
cmake/QtPluginsExtension.cmake       qtplugins_add_extension() helper
```

## Building

Requires Qt 6.4+ (Core, Gui, Widgets, Quick, Quick3D, Test), CMake ≥ 3.22 and Ninja.

```bash
scripts/setup-linux.sh                       # Ubuntu 24.04: installs Qt 6.4.2 + tools via apt
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build/host/qtplugins-host --extensions-dir build/extensions
```

Host options: `--extensions-dir <dir>` (default: `extensions/` next to the executable, or
`Contents/PlugIns/extensions` in a macOS bundle) and `--open <id>` to open an extension at startup,
e.g. `--open org.qtplugins.tictactoe` or `--open org.qtplugins.mustardmayo`.

On Linux, `ctest` runs the smoke tests `smoke_tictactoe` and `smoke_mustardmayo` under `xvfb-run`.
Mesa software rendering is enough. The tests write screenshots to `build/tests/smoke-*.png`
(tic-tac-toe) and `build/tests/mm-*.png` (one per Mustard and Mayonnaise scene).

## How it works

An extension is a Qt plugin whose root object implements `IExtension`:

```cpp
class IExtension {
public:
    virtual ~IExtension() = default;
    virtual QWidget *createWidget(QWidget *parent) = 0;
};
```

Each extension embeds a metadata JSON via `Q_PLUGIN_METADATA`:

```json
{ "id": "org.example.mygame", "name": "My Game", "version": "1.0.0",
  "description": "...", "apiVersion": 1 }
```

`ExtensionManager::loadFromDirectory()` reads each library's metadata with
`QPluginLoader::metaData()` (without loading it) and rejects anything with the wrong interface IID,
an unsupported `apiVersion`, a missing or duplicate `id`. Only then does it load the library and
`qobject_cast` the root object to `IExtension`. Every file it skips is recorded with a reason,
and the example host shows these on its *Extensions* tab.

## Writing a new extension

1. Create `plugins/<name>/` with a class deriving from `QObject` and `IExtension`, carrying
   `Q_PLUGIN_METADATA(IID IExtension_iid FILE "<name>.json")` and `Q_INTERFACES(IExtension)`
   (see `plugins/hello/`).
2. In its `CMakeLists.txt`: `qtplugins_add_extension(<name> <sources...>)` and link whatever Qt
   modules it needs. Add `add_subdirectory(<name>)` to `plugins/CMakeLists.txt`.
3. Compile QML and other assets into the library with `qt_add_resources`, using a **unique
   prefix** such as `/qtplugins/<name>`. All loaded plugins share a single resource namespace.
4. For Qt Quick content, follow `TicTacToeExtension`: put a `QQuickView` in
   `QWidget::createWindowContainer`. That renders with the platform's native graphics API
   (Metal / Direct3D / Vulkan / OpenGL). In Qt < 6.7, `QQuickWidget` only renders through OpenGL.
5. Keep game logic in a plain C++ library so it can be unit tested without a GPU.

## Integrating into an existing host application

Copy these into the application:

- `sdk/include/qtplugins/IExtension.h`
- `host/ExtensionManager.h` and `host/ExtensionManager.cpp`

Then:

```cpp
auto *extensions = new ExtensionManager(qApp);          // keep alive as long as extensions are used
extensions->loadFromDirectory(appDir + "/extensions");
for (const auto &ext : extensions->loadedExtensions())
    addMenuEntry(ext.name, [=] { showSomewhere(ext.instance->createWidget(nullptr)); });
```

`MainWindow` is only a demo of one possible UI. Plugins stay loaded for the lifetime of the
manager. Destroy any widgets an extension created before you unload it.

### Compatibility rules

Qt plugins are native C++. The host and every extension **must** be built with:

- the same Qt version series (Qt refuses plugins built against a *newer* Qt than the host),
- the same compiler / C++ ABI (on Windows: the same MSVC toolset family, or MinGW for both),
- the same build type (Debug vs Release). This is mandatory on Windows, where debug and release
  runtimes differ, and strongly recommended everywhere else.

Ship one build of each extension per platform and host Qt build. When `IExtension` changes,
bump both `IExtension_iid` and `QTPLUGINS_API_VERSION` so that old plugins are rejected cleanly
and never crash the host.

## Deployment

`cmake --install build --prefix <dir>` produces:

| Platform | Host | Extensions |
|---|---|---|
| Linux / Windows | `bin/qtplugins-host[.exe]` | `bin/extensions/` |
| macOS | `qtplugins-host.app` | `qtplugins-host.app/Contents/PlugIns/extensions/` |

The host doesn't link Qt Quick or Qt Quick 3D, but the tic-tac-toe extension does. A deployment
therefore has to include **the Qt libraries and QML modules the extensions need**, not only the
ones the host needs. Deployment tools that only scan the host executable will miss them.
Mustard and Mayonnaise also loads JPEG photos, so the `qjpeg` image-format plugin must be deployed.
`windeployqt` and `macdeployqt` include it by default.

- **Linux (system Qt):** depend on the distro packages: `libqt6quick3d6`, `qml6-module-quick3d`,
  `qml6-module-qtquick`, `qml6-module-qtquick-window`, `libqt6quick6` and the Mesa GL
  drivers. For a self-contained bundle, use a tool such as `linuxdeploy` with its Qt plugin
  (pass the extension `.so` files and `QML_SOURCES_PATHS=plugins/tictactoe/qml`).
- **Windows:** run `windeployqt` on the host **and** on each extension DLL, passing
  `--qmldir plugins/tictactoe/qml` so that it copies `QtQuick3D` and its QML imports.
  To run tests from the build tree, Qt's `bin` directory must be on `PATH`.
- **macOS:** run `macdeployqt qtplugins-host.app -qmldir=plugins/tictactoe/qml
  -executable=qtplugins-host.app/Contents/PlugIns/extensions/libtictactoe.so`. Re-sign the bundle
  afterwards. Every extension must be signed with the same team identity as the host, or
  hardened-runtime library validation will refuse to load it.

The installed binaries keep an RPATH to a non-system Qt (`CMAKE_INSTALL_RPATH_USE_LINK_PATH`).
That is convenient for local installs. For distribution, use the deployment tools above.

## Platforms

- **Linux:** developed on headless Ubuntu 24.04 with Qt 6.4.2 from apt.
- **macOS (Apple silicon):** not yet verified. See [docs/macos-setup.md](docs/macos-setup.md). Install Qt 6.4.x–6.5 LTS with the Qt online
  installer or `aqtinstall`. Homebrew ships a much newer Qt.
- **Windows:** not yet verified. Use the Qt online installer with an MSVC 2019/2022 kit.
