# Compilar UI Electron + React

param(
    [switch]$Clean
)

Write-Host "=== COMPILANDO UI ELECTRON + REACT ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent
$uiDir = Join-Path $projectRoot "ui"

Push-Location $uiDir

# Limpiar si se solicita
if ($Clean) {
    Write-Host "🧹 Limpiando builds anteriores..." -ForegroundColor Yellow
    
    $distPath = Join-Path $uiDir "dist"
    if (Test-Path $distPath) {
        Remove-Item -Recurse -Force $distPath
    }
}

# Instalar dependencias
Write-Host "📦 Instalando dependencias npm..." -ForegroundColor Yellow

# Verificar package.json
if (-not (Test-Path "package.json")) {
    Write-Host "❌ No se encuentra package.json" -ForegroundColor Red
    Pop-Location
    exit 1
}

& npm install --silent 2>&1 | ForEach-Object {
    if ($_ -match "error|ERR") {
        Write-Host "   $_" -ForegroundColor Yellow
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error instalando dependencias" -ForegroundColor Red
    Write-Host "   Intentar: npm cache clean --force" -ForegroundColor Yellow
    Pop-Location
    exit 1
}

Write-Host "✅ Dependencias instaladas" -ForegroundColor Green

# Compilar TypeScript y Vite
Write-Host "🔨 Compilando TypeScript..." -ForegroundColor Yellow

# Verificar tsconfig
if (-not (Test-Path "tsconfig.json")) {
    Write-Host "❌ No se encuentra tsconfig.json" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "🔨 Compilando Vite bundle..." -ForegroundColor Yellow

& npm run build 2>&1 | Out-Null

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Error en compilación" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# Verificar resultado
$mainPath = Join-Path $uiDir "dist\main\main.js"
$rendererPath = Join-Path $uiDir "dist\renderer\index.html"

if ((Test-Path $mainPath) -and (Test-Path $rendererPath)) {
    Write-Host "✅ UI compilada exitosamente" -ForegroundColor Green
    Write-Host "   Main: $mainPath" -ForegroundColor Gray
    Write-Host "   Renderer: $rendererPath" -ForegroundColor Gray
    exit 0
} else {
    Write-Host "❌ No se generaron los bundles correctamente" -ForegroundColor Red
    Write-Host "   Verificar errores de TypeScript" -ForegroundColor Yellow
    exit 1
}