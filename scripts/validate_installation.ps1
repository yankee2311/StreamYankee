# Validar Instalación

Write-Host "=== VALIDANDO INSTALACIÓN ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent
$errors = @()
$warnings = @()

# Verificar estructura de directorios
Write-Host "`n[1/5] Estructura de Directorios..." -ForegroundColor Yellow

$requiredDirs = @(
    "core",
    "core\include",
    "core\src",
    "bridge",
    "bridge\src",
    "ui",
    "ui\src",
    "ui\src\main",
    "ui\src\renderer",
    "ui\src\renderer\components",
    "ui\src\renderer\store",
    "scripts",
    "tests",
    "docs"
)

foreach ($dir in $requiredDirs) {
    $path = Join-Path $projectRoot $dir
    if (Test-Path $path) {
        Write-Host " ✅ $dir" -ForegroundColor Green
    } else {
        Write-Host " ❌ $dir FALTA" -ForegroundColor Red
        $errors += "Directorio faltante: $dir"
    }
}

# Verificar archivos críticos
Write-Host "`n[2/5] Archivos Críticos..." -ForegroundColor Yellow

$requiredFiles = @{
    "core\CMakeLists.txt" = "Build system C++"
    "core\include\common.hpp" = "Tipos comunes"
    "core\include\input_manager.hpp" = "Gestor de inputs"
    "core\src\input_manager.cpp" = "Implementación input"
    "bridge\binding.gyp" = "Configuración node-gyp"
    "bridge\src\obs_addon.cc" = "Bridge C++/JS"
    "ui\package.json" = "Dependencias UI"
    "ui\vite.config.ts" = "Configuración Vite"
    "ui\src\renderer\App.tsx" = "Aplicación React"
    "package.json" = "Dependencias raíz"
    "README.md" = "Documentación"
    "BUILD.md" = "Instrucciones build"
}

foreach ($file in $requiredFiles.GetEnumerator()) {
    $path = Join-Path $projectRoot $file.Key
    if (Test-Path $path) {
        Write-Host " ✅ $($file.Key)" -ForegroundColor Green
    } else {
        Write-Host " ❌ $($file.Key) FALTA" -ForegroundColor Red
        $errors += "Archivo faltante: $($file.Key)"
    }
}

# Verificar compilaciones
Write-Host "`n[3/5] Estado de Compilación..." -ForegroundColor Yellow

$buildChecks = @{
    "core\build\core.vcxproj" = "Core configurado"
    "core\dist\lib\obs_core.lib" = "Core compilado"
    "bridge\build\Release\obs_addon.node" = "Bridge compilado"
    "ui\dist\main\main.js" = "UI compilada"
    "ui\dist\renderer\index.html" = "UI bundle"
}

foreach ($check in $buildChecks.GetEnumerator()) {
    $path = Join-Path $projectRoot $check.Key
    if (Test-Path $path) {
        Write-Host " ✅ $($check.Value)" -ForegroundColor Green
    } else {
        Write-Host " ⚠️  $($check.Value) - No compilado" -ForegroundColor Yellow
        $warnings += "No compilado: $($check.Key)"
    }
}

# Verificar FFmpeg
Write-Host "`n[4/5] Dependencias Externas..." -ForegroundColor Yellow

Write-Host " FFmpeg..." -NoNewline
if (Get-Command ffmpeg -ErrorAction SilentlyContinue) {
    $version = (ffmpeg -version 2>&1 | Select-Object -First 1) -replace 'ffmpeg version '
    Write-Host " OK (v$version)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $errors += "FFmpeg no encontrado en PATH"
}

Write-Host " Node.js..." -NoNewline
if (Get-Command node -ErrorAction SilentlyContinue) {
    $version = node --version
    Write-Host " OK ($version)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $errors += "Node.js no encontrado"
}

Write-Host " npm..." -NoNewline
if (Get-Command npm -ErrorAction SilentlyContinue) {
    $version = npm --version
    Write-Host " OK ($version)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $errors += "npm no encontrado"
}

Write-Host " CMake..." -NoNewline
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $version = (cmake --version | Select-Object -First 1) -replace 'cmake version '
    Write-Host " OK ($version)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $errors += "CMake no encontrado"
}

# Verificar configuración
Write-Host "`n[5/5] Configuración..." -ForegroundColor Yellow

# Variables de entorno
Write-Host " FFMPEG_ROOT..." -NoNewline
if ($env:FFMPEG_ROOT) {
    Write-Host " OK ($($env:FFMPEG_ROOT))" -ForegroundColor Green
} else {
    Write-Host " NO CONFIGURADO" -ForegroundColor Yellow
    $warnings += "FFMPEG_ROOT no configurado"
}

# Dependencias npm
Write-Host " node_modules..." -NoNewline
$nodeModules = Join-Path $projectRoot "node_modules"
if (Test-Path $nodeModules) {
    $count = (Get-ChildItem $nodeModules -Directory).Count
    Write-Host " OK ($count paquetes)" -ForegroundColor Green
} else {
    Write-Host " NO INSTALADO" -ForegroundColor Yellow
    $warnings += "node_modules no instalado, ejecutar: npm install"
}

# Resumen
Write-Host "`n" -NoNewline
Write-Host "═════════════════════════════════════════════════" -ForegroundColor DarkGray

if ($errors.Count -eq 0 -and $warnings.Count -eq 0) {
    Write-Host "✅ VALIDACIÓN EXITOSA - Todo está en orden" -ForegroundColor Green
    Write-Host "`nPróximos pasos:" -ForegroundColor Cyan
    Write-Host "  1. Ejecutar: .\scripts\build_complete.ps1" -ForegroundColor Gray
    Write-Host "  2. Ejecutar: .\scripts\build_executable.ps1" -ForegroundColor Gray
    Write-Host "  3. Distribuir: ui\dist\OBS Propio Setup.exe" -ForegroundColor Gray
    exit 0
}

if ($warnings.Count -gt 0) {
    Write-Host "⚠️  ADVERTENCIAS ($($warnings.Count)):" -ForegroundColor Yellow
    foreach ($warning in $warnings) {
        Write-Host "   • $warning" -ForegroundColor Yellow
    }
    Write-Host ""
}

if ($errors.Count -gt 0) {
    Write-Host "❌ ERRORES ($($errors.Count)):" -ForegroundColor Red
    foreach ($error in $errors) {
        Write-Host "   • $error" -ForegroundColor Red
    }
    Write-Host "`nAcción requerida:" -ForegroundColor Cyan
    Write-Host "   1. Instalar componentes faltantes" -ForegroundColor Gray
    Write-Host "   2. Ejecutar: .\scripts\check_prerequisites.ps1" -ForegroundColor Gray
    exit 1
}

exit 0