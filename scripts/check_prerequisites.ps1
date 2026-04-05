# Verificar Prerrequisitos
Write-Host "=== Verificando Prerrequisitos ===" -ForegroundColor Cyan

$missing = @()

# Verificar Node.js
Write-Host "`n[1/6] Node.js..." -NoNewline
if (Get-Command node -ErrorAction SilentlyContinue) {
    $nodeVersion = node --version
    Write-Host " OK ($nodeVersion)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "Node.js"
}

# Verificar npm
Write-Host "[2/6] npm..." -NoNewline
if (Get-Command npm -ErrorAction SilentlyContinue) {
    $npmVersion = npm --version
    Write-Host " OK ($npmVersion)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "npm"
}

# Verificar CMake
Write-Host "[3/6] CMake..." -NoNewline
if (Get-Command cmake -ErrorAction SilentlyContinue) {
    $cmakeVersion = (cmake --version | Select-Object -First 1) -replace 'cmake version '
    Write-Host " OK ($cmakeVersion)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "CMake"
}

# Verificar Visual Studio
Write-Host "[4/6] Visual Studio..." -NoNewline
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    Write-Host " OK ($vsPath)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "Visual Studio 2022"
}

# Verificar FFmpeg
Write-Host "[5/6] FFmpeg..." -NoNewline
if (Get-Command ffmpeg -ErrorAction SilentlyContinue) {
    Write-Host " OK" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "FFmpeg"
}

# Verificar Python (para node-gyp)
Write-Host "[6/6] Python..." -NoNewline
if (Get-Command python -ErrorAction SilentlyContinue) {
    $pythonVersion = python --version 2>&1
    Write-Host " OK ($pythonVersion)" -ForegroundColor Green
} else {
    Write-Host " FALTA" -ForegroundColor Red
    $missing += "Python"
}

Write-Host "`n"

if ($missing.Count -gt 0) {
    Write-Host "❌ Componentes faltantes:" -ForegroundColor Red
    foreach ($item in $missing) {
        Write-Host "   • $item" -ForegroundColor Yellow
    }
    
    Write-Host "`nInstalación automática:" -ForegroundColor Cyan
    
    if (Get-Command choco -ErrorAction SilentlyContinue) {
        Write-Host "Ejecutando instalación con Chocolatey..." -ForegroundColor Yellow
        
        if ($missing -contains "Visual Studio 2022") {
            choco install visualstudio2022-workload-nativedesktop -y
        }
        if ($missing -contains "CMake") {
            choco install cmake -y
        }
        if ($missing -contains "Node.js") {
            choco install nodejs-lts -y
        }
        if ($missing -contains "FFmpeg") {
            choco install ffmpeg -y
        }
        if ($missing -contains "Python") {
            choco install python -y
        }
        
        Write-Host "`n✅ Instalación completada. Reiniciar PowerShell y ejecutar nuevamente." -ForegroundColor Green
    } else {
        Write-Host "`nChocolatey no está instalado." -ForegroundColor Yellow
        Write-Host "Instalar manualmente o ejecutar:" -ForegroundColor Yellow
        Write-Host "  Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))" -ForegroundColor Gray
    }
    
    exit 1
} else {
    Write-Host "✅ Todos los prerrequisitos están instalados" -ForegroundColor Green
    exit 0
}