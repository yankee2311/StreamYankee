# Plataforma de Streaming Multicámara Interactiva

Sistema de producción de video profesional para transmisiones deportivas (fútbol).

## Características Principales

- Gestión de múltiples fuentes de video
- Producción en tiempo real (switching, overlays, escenas)
- Repeticiones instantáneas con buffer circular
- Streaming multiplataforma (YouTube, Twitch, Facebook)
- Sistema de overlays deportivos (marcador, lower thirds)
- Optimizado para hardware low-end

## Arquitectura

```
┌─────────────────────────────────────────┐
│  Electron + React UI                    │
└────────────────┬────────────────────────┘
                 │ Node.js Native Addon (N-API)
┌────────────────┴────────────────┐
│  C++ Core Engine                 │
│  - InputManager                  │
│  - SceneEngine                   │
│  - SwitchingEngine               │
│  - ReplayEngine                  │
│  - OverlayEngine                 │
│  - OutputEngine                  │
└─────────────────────────────────┘
```

## Compilación

### Prerrequisitos

- CMake 3.20+
- C++17 compiler (GCC 9+, Clang 10+, MSVC 2019+)
- Node.js 18+
- FFmpeg 5.0+

### Build

```bash
# Instalar dependencias de Node.js
npm install

# Compilar C++ core
npm run build:core

# Compilar UI
npm run build:ui

# Ejecutar aplicación
npm start
```

## Desarrollo

```bash
# Modo desarrollo con hot reload
npm run dev
```

## Estructura del Proyecto

- `core/` - Motor de video en C++
- `bridge/` - Bridge Node.js ↔ C++ (N-API)
- `ui/` - Interfaz de usuario (Electron + React)
- `tests/` - Tests unitarios e integración
- `docs/` - Documentación técnica
- `assets/` - Recursos (fuentes, imágenes, templates)

## Licencia

MIT