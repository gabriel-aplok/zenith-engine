<p align="center">
  <img src="https://github.com/gabriel-aplok.png" width=30% height=30%>
</p>

**Zenith Engine** is a **modern, open-source, high-level game engine** built for speed, portability, and developer productivity. Powered by C++, OpenGL, GLFW, and the high-performance Jolt Physics engine, Zenith enables you to create stunning 3D games and interactive experiences with minimal boilerplate.

Featuring an **intuitive editor**, **real-time debugging tools**, and a **modular architecture**, Zenith gets you from idea to prototype — fast.

**Give a :star: if you find the project useful!** Your support fuels innovation and helps us deliver powerful new features.

![GitHub contributors](https://img.shields.io/github/contributors/kolpa-engine/kolpa?style=for-the-badge)
[![GitHub issues](https://img.shields.io/github/issues/kolpa-engine/kolpa?style=for-the-badge)](https://github.com/kolpa-engine/kolpa/issues)
![Number of GitHub closed issues](https://img.shields.io/github/issues-closed/kolpa-engine/kolpa?style=for-the-badge)
![GitHub pull requests](https://img.shields.io/github/issues-pr-raw/kolpa-engine/kolpa?style=for-the-badge)
![GitHub commits](https://img.shields.io/github/commit-activity/m/kolpa-engine/kolpa?style=for-the-badge)
[![Website](https://img.shields.io/website-up-down-green-red/http/shields.io.svg?style=for-the-badge)](https://zenith-engine.com/)
[![GitHub stars](https://img.shields.io/github/stars/kolpa-engine/kolpa?style=for-the-badge)](https://github.com/kolpa-engine/kolpa/stargazers)

<!-- ![C++](https://img.shields.io/badge/C++-23-blue.svg?style=for-the-badge)
![CMake](https://img.shields.io/badge/CMake-3.28%2B-green.svg?style=for-the-badge)
![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange.svg?style=for-the-badge)
![Windows](https://img.shields.io/badge/platform-Windows-lightgrey.svg?style=for-the-badge) -->

> Zenith Engine is under active development..
> Contributions, bug reports, and feedback are highly encouraged and appreciated!

## Features

- **C++/C API** – High-performance core with modern C++ and clean C bindings.
- **OpenGL + GLFW** – Fast, lightweight rendering and windowing.
- **Dear ImGui UI** – Built-in editor and debug interface with docking and real-time tools.
- **Jolt Physics** – Deterministic, multi-threaded physics with debug renderer and profiler. -** Modular Design** – Separate Editor, Game, and Library modules for flexibility
- **Hot-Reloading** – Shaders and assets update live during development.
- **Entity-Component System** – Simple, hierarchical scenes with runtime serialization.

> Actively evolving — Audio, scripting, and Vulkan support on the roadmap.

## Prerequisites

| Tool                   | Version | Link                                                              |
| ---------------------- | ------- | ----------------------------------------------------------------- |
| **Git**                | Latest  | [git-scm.com](https://git-scm.com/)                               |
| **Visual Studio 2022** | 17.8+   | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/) |
| **CMake**              | 3.28+   | Included with VS or [cmake.org](https://cmake.org/download/)      |
| **vcpkg**              | Latest  | Will install below                                                |

> **Install Visual Studio with**:
>
> - \[x] **Desktop development with C++**
> - \[x] **CMake tools for C++**
> - \[x] **Git for Windows**

## Setup (One-Time)

### 1. Clone the Repository

```bash
git clone git@github.com:gabriel-aplok/zenith-engine.git
cd zenith-engine
```

### 2. Install vcpkg (Dependency Manager)

> Or fallow the Microsoft offical steps on [here](https://learn.microsoft.com/vcpkg/get_started/get-started-vs)

```bash
# In PowerShell (as Administrator recommended)
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
```

> Set Environment Variable (do this once):

```bash
# Add to System Environment Variables
$env:VCPKG_ROOT="C:\vcpkg"
$env:PATH="$env:VCPKG_ROOT;$env:PATH"
```

> Or use user variable if no admin rights.

### 3. Install Dependencies via vcpkg

```bash
vcpkg install --triplet x64-windows
```

or just:

```bash
vcpkg install
```

This installs:

- `glad`
- `glfw3`
- `glm`
- `imgui-node-editor`
- `imguizmo`
- `imgui[docking-experimental, glfw-binding, opengl3-binding]`
- `joltphysics[debugrenderer, profiler]`
- `lodepng`
- `nlohmann-json`
- `spdlog`
- `tinyobjloader`

> Takes 5–15 minutes first time.

### 4. Integrate vcpkg with Visual Studio (Optional but Recommended)

```bash
vcpkg integrate install
```

> Output:
> `Applied user-wide integration for this vcpkg root.`
> This auto-copies DLLs in VS — no manual DLL hunting

## Build & Run (Every Time)

### Visual Studio (Recommended)

1. Open Visual Studio
2. `File → Open → CMake...`
3. Select `CMakeLists.txt` in the repo root
4. Wait for CMake to configure
5. Press F5 or `Build → Build All`

> Two executables:

- `zenith-editor.exe` → Engine editor
- `zenith-game.exe` → Game runtime

## Troubleshooting

| Problem              | Version                                        |
| -------------------- | ---------------------------------------------- |
| glfw3.dll not found  | Run `vcpkg integrate install` or rebuild in VS |
| `VCPKG_ROOT` not set | Set system env var to `C:\vcpkg`               |
| CMake errors         | Delete `out/` and retry                        |
| Shaders missing      | They are copied automatically on build         |

## Development Tips

- Use **Visual Studio IntelliSense** — full C++23 support

## Contributing

Pull latest: `git pull`
Create branch: `git checkout -b feature/awesome-thing`
Commit: `git commit -m "Add awesome thing"`
Push: `git push origin feature/awesome-thing`
Open **Pull Request** on GitHub

## License

Check [LICENSE](https://github.com/gabriel-aplok/zenith-engine/blob/main/LICENSE) for more information.
