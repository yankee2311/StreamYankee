# Build Completo - Script Maestro

param(
    [switch]$BuildCore = $true,
    [switch]$BuildBridge = $true,
    [switch]$BuildUI = $true,
    [switch]$RunTests = $true,
    [switch]$GenerateExe = $true,
    [switch]$Clean,
    [switch]$SkipFFmpeg
)

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

Write-Host @"

╔═══════════════════════════════════════════════════════════════╗
║                    OBS PROPIO - BUILD SCRIPT                  ║
║              Plataforma de Streaming Multicámara               ║
╚═══════════════════════════════════════════════════════════════╝

"@ -ForegroundColor Cyan

$startTime = Get-Date
$projectRoot = $PSScriptRoot | Split-Path -Parent
$scriptsDir = $PSScriptRoot

# Banner con estado
Write-Host "Estado del Build:" -ForegroundColor White
Write-Host "  Core C++:      $(if($BuildCore){'✓'}else{'○'})" -ForegroundColor Cyan
Write-Host "  Bridge Node:   $(if($BuildBridge){'✓'}else{'○'})" -ForegroundColor Cyan
Write-Host "  UI Electron:   $(if($BuildUI){'✓'}else{'○'})" -ForegroundColor Cyan
Write-Host "  Tests:         $(if($RunTests){'✓'}else{'○'})" -ForegroundColor Cyan
Write-Host "  Ejecutable:    $(if($GenerateExe){'✓'}else{'○'})" -ForegroundColor Cyan
Write-Host ""

# Verificar prerrequisitos
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
Write-Host "FASE 0: Verificando Prerrequisitos" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray

& "$scriptsDir\check_prerequisites.ps1"
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Prerrequisitos incompletos" -ForegroundColor Red
    Write-Host "   Instalar componentes faltantes y reiniciar" -ForegroundColor Yellow
    exit 1
}

# Limpiar si se solicita
if ($Clean) {
    Write-Host "`n🧹 Limpiando builds anteriores..." -ForegroundColor Yellow
    
    $dirsToClean = @(
        "core\build",
        "core\dist",
        "bridge\build",
        "ui\dist"
    )
    
    foreach ($dir in $dirsToClean) {
        $fullPath = Join-Path $projectRoot $dir
        if (Test-Path $fullPath) {
            Remove-Item -Recurse -Force $fullPath
            Write-Host "   Eliminado: $dir" -ForegroundColor Gray
        }
    }
    
    Write-Host "✅ Limpieza completada" -ForegroundColor Green
}

# Fase 1: Core C++
if ($BuildCore) {
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "FASE 1: Compilando Core C++" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    
    & "$scriptsDir\build_core.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Error compilando Core" -ForegroundColor Red
        Write-Host "   Verificar errores de CMake arriba" -ForegroundColor Yellow
        exit 1
    }
}

# Fase 2: Bridge Node.js
if ($BuildBridge) {
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "FASE 2: Compilando Bridge Node.js" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    
    & "$scriptsDir\build_bridge.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Error compilando Bridge" -ForegroundColor Red
        Write-Host "   Verificar que Core esté compilado" -ForegroundColor Yellow
        exit 1
    }
}

# Fase 3: UI
if ($BuildUI) {
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "FASE 3: Compilando UI Electron + React" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    
    & "$scriptsDir\build_ui.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Error compilando UI" -ForegroundColor Red
        Write-Host "   Verificar errores de TypeScript arriba" -ForegroundColor Yellow
        exit 1
    }
}

# Fase 4: Pruebas
if ($RunTests) {
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "FASE 4: Ejecutando Pruebas" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    
    & "$scriptsDir\run_tests.ps1"
    # No detener si tests fallan, solo advertir
}

# Fase 5: Ejecutable
if ($GenerateExe) {
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    Write-Host "FASE 5: Generando Ejecutable de Windows" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
    
    & "$scriptsDir\build_executable.ps1"
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ Error generando ejecutable" -ForegroundColor Red
        exit 1
    }
}

# Finalizar
$endTime = Get-Date
$duration = $endTime - $startTime

Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
Write-Host "✅ BUILD COMPLETADO" -ForegroundColor Green
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor DarkGray
Write-Host "Duración: $($duration.ToString('mm\:ss'))" -ForegroundColor White
Write-Host ""
Write-Host "Para ejecutar:" -ForegroundColor Cyan
Write-Host "  cd ui" -ForegroundColor Gray
Write-Host "  npm start" -ForegroundColor Gray
Write-Host ""
Write-Host "Distribución:" -ForegroundColor Cyan
Write-Host "  $($projectRoot)\ui\dist\OBS Propio Setup.exe" -ForegroundColor Gray
Write-Host ""

# Validación final
Write-Host "Ejecutar validación completa:" -ForegroundColor Yellow
Write-Host "  .\scripts\validate_installation.ps1" -ForegroundColor Gray
Write-Host ""

exit 0