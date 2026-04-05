# Generar Ejecutable de Windows

param(
    [switch]$Portable
)

Write-Host "=== GENERANDO EJECUTABLE DE WINDOWS ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent
$uiDir = Join-Path $projectRoot "ui"
$distDir = Join-Path $uiDir "dist"

# Verificar prerequisitos
$coreLib = Join-Path $projectRoot "core\dist\lib\obs_core.lib"
$bridgeNode = Join-Path $projectRoot "bridge\build\Release\obs_addon.node"

if (-not (Test-Path $coreLib)) {
    Write-Host "❌ Core no compilado" -ForegroundColor Red
    Write-Host "   Ejecutar: .\scripts\build_core.ps1" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $bridgeNode)) {
    Write-Host "⚠️  Bridge no compilado, generando standalone" -ForegroundColor Yellow
}

# Copiar archivos necesarios a dist
Write-Host "`n📦 Copiando archivos..." -ForegroundColor Yellow

if (-not (Test-Path $distDir)) {
    New-Item -ItemType Directory -Path $distDir -Force | Out-Null
}

# Copiar bridge si existe
if (Test-Path $bridgeNode) {
    Copy-Item $bridgeNode -Destination $distDir -Force
    Write-Host "✅ obs_addon.node copiado" -ForegroundColor Green
}

# Copiar DLLs de FFmpeg si existen
$ffmpegPath = $env:FFMPEG_ROOT
if ($ffmpegPath -and (Test-Path $ffmpegPath)) {
    $dlls = @("avcodec-60.dll", "avformat-60.dll", "avutil-58.dll", "swscale-7.dll", "swresample-4.dll")
    
    foreach ($dll in $dlls) {
        $dllPath = Join-Path $ffmpegPath $dll
        $dllPathAlt = $ffmpegPath -replace "bin", ""
        $dllPathAlt = Join-Path $dllPathAlt $dll
        
        if (Test-Path $dllPath) {
            Copy-Item $dllPath -Destination $distDir -Force
            Write-Host "✅ $dll copiado" -ForegroundColor Gray
        } elseif (Test-Path $dllPathAlt) {
            Copy-Item $dllPathAlt -Destination $distDir -Force
            Write-Host "✅ $dll copiado" -ForegroundColor Gray
        }
    }
}

Push-Location $uiDir

# Compilar con electron-builder
Write-Host "🔨 Empaquetando aplicación..." -ForegroundColor Yellow

try {
    if ($Portable) {
        Write-Host "   Generando paquete portable..." -ForegroundColor Gray
        & npm run electron:pack 2>&1 | Out-Null
    } else {
        Write-Host "   Generando instalador..." -ForegroundColor Gray
        & npm run electron:build 2>&1 | Out-Null
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "Error en electron-builder"
    }
} catch {
    Write-Host "❌ Error empaquetando: $_" -ForegroundColor Red
    Write-Host "   Verificar que electron-builder esté instalado" -ForegroundColor Yellow
    Write-Host "   npm install electron-builder --save-dev" -ForegroundColor Yellow
    Pop-Location
    exit 1
}

Pop-Location

# Verificar resultado
$installerPath = Join-Path $uiDir "dist\OBS Propio Setup*.exe"
$portablePath = Join-Path $uiDir "dist\win-unpacked"

if (Test-Path $installerPath) {
    $installer = Get-ChildItem $installerPath | Select-Object -First 1
    Write-Host "`n✅ Instalador generado exitosamente" -ForegroundColor Green
    Write-Host "   📁 $($installer.FullName)" -ForegroundColor Cyan
    
    # Mostrar tamaño
    $sizeMB = [math]::Round($installer.Length / 1MB, 2)
    Write-Host "   Tamaño: $sizeMB MB" -ForegroundColor Gray
    
} elseif (Test-Path $portablePath) {
    Write-Host "`n✅ Versión portable generada" -ForegroundColor Green
    Write-Host "   📁 $portablePath" -ForegroundColor Cyan
    
    $exePath = Join-Path $portablePath "OBS Propio.exe"
    if (Test-Path $exePath) {
        Write-Host "   Ejecutable: $exePath" -ForegroundColor Gray
    }
} else {
    Write-Host "`n❌ No se generó el ejecutable" -ForegroundColor Red
    Write-Host "   Verificar configuración de electron-builder" -ForegroundColor Yellow
    exit 1
}

# Crear archivo de instrucciones
$readmePath = Join-Path $uiDir "dist\LEEME.txt"
$readmeContent = @"
OBS Propio - Plataforma de Streaming Multicámara
=================================================

INSTALACIÓN:
1. Ejecutar "OBS Propio Setup.exe" o descomprimir versión portable
2. Seguir el asistente de instalación
3. Ejecutar "OBS Propio.exe"

REQUISITOS:
- Windows 10 o superior
- FFmpeg (incluido en la instalación)
- Microsoft Visual C++ Redistributable

USO BÁSICO:
1. Agregar fuentes de video (cámaras, archivos)
2. Crear escenas y organizar fuentes
3. Agregar overlays (marcador, lower thirds)
4. Iniciar streaming o grabación
5. Usar replay buffer para repeticiones

SOPORTE:
- GitHub: https://github.com/tu-usuario/obs-propio
- Documentación: docs/TECHNICAL_DOCUMENTATION.md

LICENCIA:
MIT License - Ver archivo LICENSE para más detalles.
"@

Set-Content -Path $readmePath -Value $readmeContent -Encoding UTF8

Write-Host "`n📝 Instrucciones creadas en: LEEME.txt" -ForegroundColor Gray
Write-Host "`n✅ Build completo" -ForegroundColor Green
exit 0