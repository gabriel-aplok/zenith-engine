param(
    [string]$Shaderc,
    [string]$ShaderDir = (Join-Path $PSScriptRoot "..\resources\shaders"),
    [string]$OutputDir = (Join-Path $PSScriptRoot "..\out\build\windows-debug\generated\render\shaders")
)

$ErrorActionPreference = "Stop"

function Resolve-Shaderc {
    param([string]$Explicit)

    if ($Explicit -and (Test-Path $Explicit)) {
        return (Resolve-Path $Explicit).Path
    }

    $commands = @(
        "shaderc.exe",
        "shadercRelease.exe",
        "shaderc-debug.exe",
        "shaderc"
    )

    foreach ($name in $commands) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) {
            return $cmd.Source
        }
    }

    $roots = @()
    if ($env:VCPKG_ROOT) { $roots += $env:VCPKG_ROOT }
    $roots += "C:\vcpkg"
    $roots += (Join-Path $env:USERPROFILE "vcpkg")
    if ($env:VCPKG_ROOT) {
        $roots += (Join-Path $env:VCPKG_ROOT "installed")
        $roots += (Join-Path $env:VCPKG_ROOT "packages")
    }

    foreach ($root in ($roots | Where-Object { $_ -and (Test-Path $_) })) {
        $matches = Get-ChildItem -Path $root -Recurse -File -Include "shaderc*.exe" -ErrorAction SilentlyContinue
        if ($matches) {
            return $matches[0].FullName
        }
    }

    return $null
}

function Resolve-BgfxCommonDir {
    $roots = @()
    if ($env:VCPKG_ROOT) { $roots += $env:VCPKG_ROOT }
    $roots += "C:\vcpkg"
    $roots += (Join-Path $env:USERPROFILE "vcpkg")
    $roots += (Join-Path $env:LOCALAPPDATA "vcpkg")
    $roots += "C:\vcpkg\buildtrees"
    $roots += (Join-Path $env:LOCALAPPDATA "vcpkg\buildtrees")

    foreach ($root in ($roots | Where-Object { $_ -and (Test-Path $_) })) {
        $matches = Get-ChildItem -Path $root -Recurse -File -Filter "common.sh" -ErrorAction SilentlyContinue
        if ($matches) {
            return $matches[0].Directory.FullName
        }
    }

    return $null
}

function Resolve-BgfxShaderDir {
    $roots = @()
    if ($env:VCPKG_ROOT) { $roots += $env:VCPKG_ROOT }
    $roots += "C:\vcpkg"
    $roots += (Join-Path $env:USERPROFILE "vcpkg")

    foreach ($root in ($roots | Where-Object { $_ -and (Test-Path $_) })) {
        $matches = Get-ChildItem -Path $root -Recurse -File -Filter "bgfx_shader.sh" -ErrorAction SilentlyContinue
        if ($matches) {
            return $matches[0].Directory.FullName
        }
    }

    return $null
}

$Shaderc = Resolve-Shaderc -Explicit $Shaderc
if (-not $Shaderc) {
    throw "shaderc executable not found. Set BGFX_SHADERC or pass -Shaderc."
}

$BgfxCommonDir = Resolve-BgfxCommonDir
if (-not $BgfxCommonDir) {
    throw "bgfx_shader.sh not found. Make sure the bgfx source tree is installed with vcpkg."
}

$BgfxShaderDir = Resolve-BgfxShaderDir
if (-not $BgfxShaderDir) {
    throw "bgfx_shader.sh not found. Make sure the bgfx source tree is installed with vcpkg."
}

if (-not (Test-Path $ShaderDir)) {
    throw "Shader source directory not found: $ShaderDir"
}

$varying = Join-Path $ShaderDir "varying.def.sc"
$vertexSource = Join-Path $ShaderDir "mesh_vs.sc"
$fragmentSource = Join-Path $ShaderDir "mesh_fs.sc"

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$commonIncludeArgs = @("-i", $ShaderDir, "-i", $BgfxCommonDir, "-i", $BgfxShaderDir)

function Invoke-BgfxShaderc {
    param(
        [string]$InputPath,
        [string]$OutputPath,
        [string]$Type,
        [string]$Platform,
        [string]$Profile,
        [string]$ArrayName
    )

    & $Shaderc @commonIncludeArgs -f $InputPath -o $OutputPath --type $Type --platform $Platform -p $Profile --varyingdef $varying --bin2c $ArrayName
}

Invoke-BgfxShaderc -InputPath $vertexSource -OutputPath (Join-Path $OutputDir "mesh_vs_dx11.bin.h") -Type vertex -Platform windows -Profile s_5_0 -ArrayName mesh_vs_dx11
Invoke-BgfxShaderc -InputPath $fragmentSource -OutputPath (Join-Path $OutputDir "mesh_fs_dx11.bin.h") -Type fragment -Platform windows -Profile s_5_0 -ArrayName mesh_fs_dx11

Invoke-BgfxShaderc -InputPath $vertexSource -OutputPath (Join-Path $OutputDir "mesh_vs_glsl430.bin.h") -Type vertex -Platform windows -Profile 430 -ArrayName mesh_vs_glsl430

Invoke-BgfxShaderc -InputPath $fragmentSource -OutputPath (Join-Path $OutputDir "mesh_fs_glsl430.bin.h") -Type fragment -Platform windows -Profile 430 -ArrayName mesh_fs_glsl430

Invoke-BgfxShaderc -InputPath $vertexSource -OutputPath (Join-Path $OutputDir "mesh_vs_gles300.bin.h") -Type vertex -Platform windows -Profile 300_es -ArrayName mesh_vs_gles300
Invoke-BgfxShaderc -InputPath $fragmentSource -OutputPath (Join-Path $OutputDir "mesh_fs_gles300.bin.h") -Type fragment -Platform windows -Profile 300_es -ArrayName mesh_fs_gles300

Invoke-BgfxShaderc -InputPath $vertexSource -OutputPath (Join-Path $OutputDir "mesh_vs_spirv.bin.h") -Type vertex -Platform windows -Profile spirv -ArrayName mesh_vs_spirv
Invoke-BgfxShaderc -InputPath $fragmentSource -OutputPath (Join-Path $OutputDir "mesh_fs_spirv.bin.h") -Type fragment -Platform windows -Profile spirv -ArrayName mesh_fs_spirv
