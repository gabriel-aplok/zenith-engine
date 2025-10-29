# Zenith Game Engine

![C++](https://img.shields.io/badge/C++-23-blue.svg?style=for-the-badge)
![CMake](https://img.shields.io/badge/CMake-3.28%2B-green.svg?style=for-the-badge)
![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange.svg?style=for-the-badge)
![Windows](https://img.shields.io/badge/platform-Windows-lightgrey.svg?style=for-the-badge)

A modern, modular game engine built with C++23, OpenGL, and physics (Jolt).\
Includes an **editor** and **game runtime**.

> **Collaborators**: [Gabriel Lima](https://github.com/gabriel-aplok) & [Aislan A.](https://github.com/a6xdev)

---

## Features

- Modern C++23
- OpenGL rendering (GLFW)
- Dear ImGui UI
- Jolt Physics
- Editor, Game and Library modules

---

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

---

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
VCPKG_ROOT = C:\vcpkg
```

> Or use user variable if no admin rights.

### 3. Install Dependencies via vcpkg

```bash
cd C:\vcpkg
.\vcpkg install --triplet x64-windows
```

This installs:

- `glad`
- `glfw3`
- `glm`
- `imgui[docking]`
- `joltphysics`
- `lodepng`
- `nlohmann-json`

> Takes 5–15 minutes first time.

### 4. Integrate vcpkg with Visual Studio (Optional but Recommended)

```bash
.\vcpkg integrate install
```

> Output:
> `Applied user-wide integration for this vcpkg root.`
> This auto-copies DLLs in VS — no manual DLL hunting

## Build & Run (Every Time)

### Option A: Visual Studio (Recommended)

1. Open Visual Studio
2. `File → Open → CMake...`
3. Select `CMakeLists.txt` in the repo root
4. Wait for CMake to configure
5. Press F5 or `Build → Build All`

> Two executables:

- `zenith-editor.exe` → Engine editor
- `zenith-game.exe` → Game runtime

### Option B: Command Line (Visual Studio 17 2022)

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build .
```

Run:

```bash
.\zenith-editor.exe
.\zenith-game.exe
```

> Shaders and DLLs are automatically copied

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

Ill decide the licence lol © Zenith Engine Team
