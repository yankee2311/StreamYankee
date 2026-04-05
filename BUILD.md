# Guía de Compilación y Ejecución - OBS Propio

## Requisitos Previos

### Windows
```powershell
# Instalar Chocolatey (si no está instalado)
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Instalar dependencias
choco install cmake -y
choco install nodejs-lts -y
choco install visualstudio2022-workload-nativedesktop -y
choco install ffmpeg -y

# Agregar FFmpeg al PATH
setx PATH "%PATH%;C:\ProgramData\chocolatey\bin"
```

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake nodejs npm ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev

# Instalar Node.js 18+
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt install -y nodejs
```

### macOS
```bash
# Instalar Homebrew (si no está instalado)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Instalar dependencias
brew install cmake node ffmpeg

# Instalar Xcode Command Line Tools
xcode-select --install
```

## Compilación Paso a Paso

### 1. Configurar Variables de Entorno

**Windows (PowerShell):**
```powershell
$env:FFMPEG_ROOT = "C:\ProgramData\chocolatey\lib\ffmpeg\tools\ffmpeg\bin"
$env:Path += ";$env:FFMPEG_ROOT"
```

**Linux/macOS:**
```bash
export FFMPEG_ROOT=/usr/local
export PATH=$PATH:$FFMPEG_ROOT/bin
```

### 2. Compilar el Core C++

```bash
# Navegar al directorio del proyecto
cd "C:\Users\capacitacion.operaci\Desktop\OBS Propio"

# Crear directorio de compilación
mkdir -p core/build
cd core/build

# Configurar con CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../dist

# Compilar
cmake --build . --config Release -- -j$(nproc)

# Instalar
cmake --install .
```

### 3. Compilar el Bridge de Node.js

```bash
# Navegar al directorio del bridge
cd ../../bridge

# Instalar dependencias de Node.js
npm install

# Recompilar el addon nativo
npm run build
```

### 4. Configurar la UI de Electron

```bash
# Navegar al directorio de la UI
cd ../ui

# Instalar dependencias
npm install

# Compilar TypeScript y Vite
npm run build
```

### 5. Build Completo (Script Automático)

Crear archivo `build.bat` (Windows) o `build.sh` (Linux/macOS):

**Windows (build.bat):**
```batch
@echo off
echo ========================================
echo Building OBS Propio - Windows
echo ========================================

echo [1/5] Building C++ Core...
cd core
if not exist build mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build . --config Release
cmake --install . --prefix ../dist
cd ..\..

echo [2/5] Building Node.js Bridge...
cd bridge
call npm install
call npm run build
cd ..

echo [3/5] Building Electron UI...
cd ui
call npm install
call npm run build
cd ..

echo [4/5] Installing Root Dependencies...
call npm install

echo [5/5] Build Complete!
echo ========================================
echo Run 'npm start' to launch the application
pause
```

**Linux/macOS (build.sh):**
```bash
#!/bin/bash
set -e

echo "========================================"
echo "Building OBS Propio - Unix"
echo "========================================"

echo "[1/5] Building C++ Core..."
cd core
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../dist
make -j$(nproc)
make install
cd ..

echo "[2/5] Building Node.js Bridge..."
cd ../bridge
npm install
npm run build
cd ..

echo "[3/5] Building Electron UI..."
cd ui
npm install
npm run build
cd ..

echo "[4/5] Installing Root Dependencies..."
npm install

echo "[5/5] Build Complete!"
echo "========================================"
echo "Run 'npm start' to launch the application"
```

## Ejecución

### Modo Desarrollo

```bash
# Terminal 1: Arrancar la UI en modo desarrollo
cd ui
npm run dev

# Terminal 2: Arrancar Electron (después de que UI compile)
cd ui
npm run electron:dev
```

### Modo Producción

```bash
# Compilar todo
npm run build

# Ejecutar
npm start
```

### Primera Ejecución

La primera vez que ejecutes la aplicación:

1. Se abrirá la ventana principal de OBS Propio
2. Podrás agregar fuentes de video desde el panel izquierdo
3. Las escenas se gestionan en el panel derecho
4. Los controles de streaming están en la barra inferior
5. El scoreboard de fútbol aparecerá automáticamente en el preview

## Estructura del Proyecto

```
OBS Propio/
├── core/                 # Motor de video C++
│   ├── include/          # Headers (.hpp)
│   ├── src/              # Implementaciones (.cpp)
│   ├── tests/            # Tests unitarios
│   └── CMakeLists.txt     # Build system
├── bridge/               # Bridge Node.js ↔ C++
│   ├── src/               # Addon nativo (.cc)
│   ├── binding.gyp        # Configuración node-gyp
│   └── package.json
├── ui/                   # Interfaz de usuario
│   ├── src/
│   │   ├── main/          # Proceso principal Electron
│   │   └── renderer/       # UI React
│   ├── public/
│   ├── index.html
│   └── package.json
├── tests/                # Tests de integración
├── docs/                 # Documentación
├── assets/               # Recursos (fuentes, imágenes)
├── package.json          # Dependencias raíz
└── README.md
```

## Solución de Problemas

### Error: "FFmpeg not found"

**Solución:**
```bash
# Verificar instalación de FFmpeg
ffmpeg -version

# Si no está instalado, agregar al PATH
setx PATH "%PATH%;C:\ffmpeg\bin"  # Windows
export PATH=$PATH:/usr/local/bin   # Unix
```

### Error: "Module not found: obs-propio-bridge"

**Solución:**
```bash
cd bridge
npm install
npm run build
```

### Error: "Cannot find module '../core/dist/lib/obs_core'"

**Solución:**
```bash
cd core/build
cmake --build . --config Release
cmake --install . --prefix ../dist
```

### La UI no compila

**Solución:**
```bash
cd ui

# Limpiar caché
rm -rf node_modules dist
npm install
npm run build
```

### Error de CMake: "Could not find FFmpeg"

**Solución:**
```bash
# Especificar ruta de FFmpeg manualmente
cmake .. -DFFMPEG_ROOT=/path/to/ffmpeg -DCMAKE_BUILD_TYPE=Release
```

## Tests

### Ejecutar Tests del Core C++

```bash
cd core/build
ctest --output-on-failure
```

### Tests de Integración

```bash
npm run test
```

## Distribución

### Generar Instalador

```bash
# Windows
npm run build
cd ui
npm run electron:build

# Los instaladores estarán en ui/dist/
```

### Empaquetar para Distribución

```bash
npm run electron:pack
```

## Configuración de Desarrollo

### Configurar VS Code (Opcional)

Crear `.vscode/settings.json`:
```json
{
  "cmake.configureSettings": {
    "CMAKE_BUILD_TYPE": "Debug",
    "FFMPEG_ROOT": "/usr/local"
  },
  "cmake.buildDirectory": "${workspaceFolder}/core/build",
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

### Configurar CLion (Opcional)

1. File → Open → Seleccionar directorio `core`
2. File → Settings → Build → CMake
3. Agregar perfil "Debug" y "Release"
4. Configurar `FFMPEG_ROOT` en variables de entorno

## Logs y Debugging

Los logs se guardan en:
- **Windows:** `C:\Users\<usuario>\AppData\Local\obs-propio\logs\`
- **Linux:** `~/.local/share/obs-propio/logs/`
- **macOS:** `~/Library/Logs/obs-propio/`

### Habilitar Debug Logging

```typescript
// En el renderer process
localStorage.setItem('DEBUG', 'true');
```

## Performance Tuning

### Optimizar para Hardware Low-End

Editar `ui/src/renderer/App.tsx`:
```typescript
dispatch(setHardwareCapabilities({
  hasNVENC: false,
  hasQuickSync: false,
  hasVCE: false,
  cpuCores: 4,
  recommendedResolution: 720,  // Reducir de 1080
  recommendedFPS: 30,
  recommendedBitrate: 2000    // Reducir de 4500
}));
```

## Roadmap de Desarrollo

### Completado ✅
- [x] Estructura del proyecto
- [x] Core C++ con headers completos
- [x] Bridge Node.js
- [x] UI Electron + React
- [x] Store Redux
- [x] Componentes principales

### En Progreso 🚧
- [ ] Integración completa del bridge
- [ ] Sistema de transiciones completo
- [ ] Motor de replay funcional
- [ ] Overlays avanzados

### Pendiente 📋
- [ ] Tests automatizados
- [ ] Documentación de API
- [ ] Empaquetado multiplataforma
- [ ] Sistema de plugins

## Soporte

Para problemas o preguntas:
1. Revisar los logs en la carpeta de logs
2. Consultar la documentación en `docs/`
3. Abrir issue en el repositorio con:
   - Sistema operativo
   - Versión de Node.js
   - Log de error completo
   - Pasos para reproducir

## Licencia

MIT License - Ver archivo LICENSE para más detalles.