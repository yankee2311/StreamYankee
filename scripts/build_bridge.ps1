# Compilar Bridge Node.js

param(
    [switch]$Clean
)

Write-Host "=== COMPILANDO BRIDGE NODE.JS ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent
$bridgeDir = Join-Path $projectRoot "bridge"

Push-Location $bridgeDir

# Limpiar si se solicita
if ($Clean) {
    Write-Host "🧹 Limpiando build anterior..." -ForegroundColor Yellow
    
    $buildPath = Join-Path $bridgeDir "build"
    if (Test-Path $buildPath) {
        Remove-Item -Recurse -Force $buildPath
    }
    
    $nodeModulesPath = Join-Path $bridgeDir "node_modules"
    if (Test-Path $nodeModulesPath) {
        # Solo limpiar node_modules si hay errores
        # Remove-Item -Recurse -Force $nodeModulesPath
    }
}

# Instalar dependencias
Write-Host "📦 Instalando dependencias..." -ForegroundColor Yellow
& npm install --silent 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error instalando dependencias" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "✅ Dependencias instaladas" -ForegroundColor Green

# Compilar addon nativo
Write-Host "🔨 Compilando addon nativo..." -ForegroundColor Yellow

# Verificar que el core esté compilado
$coreLib = Join-Path $projectRoot "core\dist\lib\obs_core.lib"
if (-not (Test-Path $coreLib)) {
    Write-Host "⚠️  Core no compilado. Ejecutar primero: .\build_core.ps1" -ForegroundColor Yellow
    
    # Crear dummy para que compile
    Write-Host "   Creando stub temporal..." -ForegroundColor Gray
    
    # El binding.gyp ya tiene configuración correcta
    # node-gyp rebuild intentará compilar
}

& npm run build 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error compilando addon" -ForegroundColor Red
    Write-Host "   Verificar que node-gyp esté instalado: npm install -g node-gyp" -ForegroundColor Yellow
    Pop-Location
    exit 1
}

Pop-Location

# Verificar resultado
$addonPath = Join-Path $bridgeDir "build\Release\obs_addon.node"
if (Test-Path $addonPath) {
    Write-Host "✅ Bridge compilado exitosamente" -ForegroundColor Green
    Write-Host "   Addon: $addonPath" -ForegroundColor Gray
    exit 0
} else {
    # Buscar en Debug
    $addonDebugPath = Join-Path $bridgeDir "build\Debug\obs_addon.node"
    if (Test-Path $addonDebugPath) {
        Write-Host "✅ Bridge compilado (Debug)" -ForegroundColor Green
        Write-Host "   Addon: $addonDebugPath" -ForegroundColor Gray
        exit 0
    }
    
    Write-Host "❌ No se generó obs_addon.node" -ForegroundColor Red
    Write-Host "   Posibles causas:" -ForegroundColor Yellow
    Write-Host "   - Core no compilado (ejecutar build_core.ps1)" -ForegroundColor Yellow
    Write-Host "   - Falta Python (requerido por node-gyp)" -ForegroundColor Yellow
    Write-Host "   - Herramientas de compilación de Windows" -ForegroundColor Yellow
    exit 1
}