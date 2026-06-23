# bgfx Shader Sources

The canonical shader sources live under `resources/shaders/` as `.sc` files.

Generate the binary headers used by the C++ renderer with:

```powershell
.\tools\build_bgfx_shaders.ps1
```

The script searches for `shaderc` in PATH and common vcpkg locations. The generated `*.bin.h` files are build artifacts only and do not need to be committed.
