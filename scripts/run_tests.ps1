# Ejecutar Pruebas

Write-Host "=== EJECUTANDO PRUEBAS ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent

# Tests del Core C++
Write-Host "`n[1/2] Tests del Core C++..." -ForegroundColor Yellow

$coreBuildDir = Join-Path $projectRoot "core\build"

if (Test-Path "$coreBuildDir\CTestTestfile.cmake") {
    Push-Location $coreBuildDir
    
    Write-Host " Ejecutando tests unitarios..." -ForegroundColor Gray
    & ctest --output-on-failure --build-config Release 2>&1 | ForEach-Object {
        if ($_ -match "Passed|Failed|tests passed") {
            Write-Host " $_" -ForegroundColor Cyan
        }
    }
    
    $ctestResult = $LASTEXITCODE
    Pop-Location
    
    if ($ctestResult -eq 0) {
        Write-Host "✅ Tests del Core pasados" -ForegroundColor Green
    } else {
        Write-Host "⚠️  Algunos tests del Core fallaron (código: $ctestResult)" -ForegroundColor Yellow
    }
} else {
    Write-Host "⚠️  Tests del Core no disponibles" -ForegroundColor Yellow
    Write-Host "   Compilar primero: .\scripts\build_core.ps1" -ForegroundColor Gray
}

# Tests de Integración Node.js
Write-Host "`n[2/2] Tests de Integración..." -ForegroundColor Yellow

$testsDir = Join-Path $projectRoot "tests"

if (Test-Path "$testsDir\package.json") {
    Push-Location $testsDir
    
    # Instalar dependencias de test
    if (-not (Test-Path "node_modules")) {
        Write-Host " Instalando dependencias de test..." -ForegroundColor Gray
        & npm install --silent 2>&1 | Out-Null
    }
    
    # Ejecutar Jest
    Write-Host " Ejecutando Jest..." -ForegroundColor Gray
    & npm test 2>&1 | ForEach-Object {
        if ($_ -match "PASS|FAIL|Tests:") {
            Write-Host " $_" -ForegroundColor Cyan
        }
    }
    
    $jestResult = $LASTEXITCODE
    Pop-Location
    
    if ($jestResult -eq 0) {
        Write-Host "✅ Tests de integración pasados" -ForegroundColor Green
    } else {
        Write-Host "⚠️  Algunos tests de integración fallaron" -ForegroundColor Yellow
    }
} else {
    Write-Host "⚠️  Tests de integración no configurados" -ForegroundColor Yellow
    Write-Host "   Crear tests/integration/ y package.json" -ForegroundColor Gray
}

# Validación básica
Write-Host "`n[VALIDACIÓN] Verificando compilación..." -ForegroundColor Yellow

$coreLib = Join-Path $projectRoot "core\dist\lib\obs_core.lib"
$bridgeNode = Join-Path $projectRoot "bridge\build\Release\obs_addon.node"
$uiMain = Join-Path $projectRoot "ui\dist\main\main.js"

$validations = @(
    @{Name="Core Library"; Path=$coreLib},
    @{Name="Bridge Addon"; Path=$bridgeNode},
    @{Name="UI Bundle"; Path=$uiMain}
)

$missing = @()
foreach ($v in $validations) {
    if (Test-Path $v.Path) {
        Write-Host " ✅ $($v.Name)" -ForegroundColor Green
    } else {
        Write-Host " ❌ $($v.Name) FALTA" -ForegroundColor Red
        $missing += $v.Name
    }
}

if ($missing.Count -gt 0) {
    Write-Host "`n⚠️  Componentes faltantes detectados" -ForegroundColor Yellow
    Write-Host "   Ejecutar: .\scripts\build_complete.ps1" -ForegroundColor Gray
    exit 1
}

Write-Host "`n✅ Pruebas completadas" -ForegroundColor Green
exit 0