# Configurar Variables de Entorno

Write-Host "=== Configurando Variables de Entorno ===" -ForegroundColor Cyan

$projectRoot = $PSScriptRoot | Split-Path -Parent

# Buscar FFmpeg
$ffmpegPaths = @(
    "C:\ProgramData\chocolatey\lib\ffmpeg\tools\ffmpeg\bin",
    "C:\ffmpeg\bin",
    "${env:ProgramFiles}\ffmpeg\bin",
    "${env:LOCALAPPDATA}\ffmpeg\bin"
)

$ffmpegFound = $false
foreach ($path in $ffmpegPaths) {
    if (Test-Path "$path\ffmpeg.exe") {
        $ffmpegFound = $true
        $env:FFMPEG_ROOT = $path
        Write-Host "✅ FFmpeg encontrado: $path" -ForegroundColor Green
        
        # Agregar a PATH si no está
        if ($env:Path -notlike "*$path*") {
            $env:Path += ";$path"
            [System.Environment]::SetEnvironmentVariable("Path", $env:Path, "User")
        }
        break
    }
}

if (-not $ffmpegFound) {
    Write-Host "⚠️  FFmpeg no encontrado automáticamente" -ForegroundColor Yellow
    Write-Host "   Descargar de: https://www.gyan.dev/ffmpeg/builds/" -ForegroundColor Yellow
    Write-Host "   Extraer a: C:\ffmpeg\" -ForegroundColor Yellow
    Write-Host "   O instalar con: choco install ffmpeg -y" -ForegroundColor Yellow
}

# Configurar FFMPEG_ROOT
if ($ffmpegFound) {
    [System.Environment]::SetEnvironmentVariable("FFMPEG_ROOT", $env:FFMPEG_ROOT, "User")
    Write-Host "✅ FFMPEG_ROOT configurado" -ForegroundColor Green
}

# Configurar variables del proyecto
$env:OBS_PROPIO_ROOT = $projectRoot
[System.Environment]::SetEnvironmentVariable("OBS_PROPIO_ROOT", $projectRoot, "User")

Write-Host "✅ OBS_PROPIO_ROOT = $projectRoot" -ForegroundColor Green

# Verificar Node.js
$nodeModulesPath = Join-Path $projectRoot "node_modules"
if (-not (Test-Path $nodeModulesPath)) {
    Write-Host "`n📦 Instalando dependencias npm raíz..." -ForegroundColor Yellow
    Push-Location $projectRoot
    npm install
    Pop-Location
}

Write-Host "`n✅ Variables de entorno configuradas correctamente" -ForegroundColor Green
Write-Host "Reiniciar PowerShell para aplicar cambios permanentes" -ForegroundColor Yellow