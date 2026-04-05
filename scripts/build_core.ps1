# Compilar Core C++

param(
    [string]$Config = "Release",
    [switch]$Clean
)

Write-Host "=== COMPILANDO CORE C++ ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent
$coreDir = Join-Path $projectRoot "core"
$buildDir = Join-Path $coreDir "build"
$distDir = Join-Path $coreDir "dist"

# Limpiar si se solicita
if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "🧹 Limpiando build anterior..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

# Crear directorio de build
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
    Write-Host "📁 Directorio build creado" -ForegroundColor Gray
}

Push-Location $buildDir

# Detectar generador
$generator = if (Get-Command "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" -ErrorAction SilentlyContinue) {
    "Visual Studio 17 2022"
} elseif (Get-Command "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\IDE\devenv.exe" -ErrorAction SilentlyContinue) {
    "Visual Studio 16 2019"
} else {
    "Ninja"
}

Write-Host "⚙️  Generador: $generator" -ForegroundColor Gray
Write-Host "⚙️  Configurando CMake..." -ForegroundColor Yellow

$cmakeArgs = @(
    "..",
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DCMAKE_INSTALL_PREFIX=../dist"
)

if ($generator -eq "Visual Studio 17 2022") {
    $cmakeArgs += "-G"
    $cmakeArgs += "Visual Studio 17 2022"
    $cmakeArgs += "-A"
    $cmakeArgs += "x64"
} elseif ($generator -eq "Visual Studio 16 2019") {
    $cmakeArgs += "-G"
    $cmakeArgs += "Visual Studio 16 2019"
    $cmakeArgs += "-A"
    $cmakeArgs += "x64"
} else {
    $cmakeArgs += "-G"
    $cmakeArgs += "Ninja"
}

& cmake $cmakeArgs 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error en configuración de CMake" -ForegroundColor Red
    Write-Host "   Verificar que FFmpeg esté instalado y en PATH" -ForegroundColor Yellow
    Pop-Location
    exit 1
}

Write-Host "✅ CMake configurado" -ForegroundColor Green

# Compilar
Write-Host "🔨 Compilando ($Config)..." -ForegroundColor Yellow

if ($generator -like "Visual Studio*") {
    & cmake --build . --config $Config --parallel 8 2>&1 | Out-Null
} else {
    & cmake --build . --config $Config 2>&1 | Out-Null
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error en compilación" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Instalar
Write-Host "📦 Instalando..." -ForegroundColor Yellow
& cmake --install . --prefix ../dist 2>&1 | Out-Null

Pop-Location

# Verificar resultado
$coreLib = Join-Path $distDir "lib\obs_core.lib"
if (Test-Path $coreLib) {
    Write-Host "✅ Core C++ compilado exitosamente" -ForegroundColor Green
    Write-Host "   Librería: $coreLib" -ForegroundColor Gray
    exit 0
} else {
    Write-Host "❌ No se generó obs_core.lib" -ForegroundColor Red
    exit 1
}