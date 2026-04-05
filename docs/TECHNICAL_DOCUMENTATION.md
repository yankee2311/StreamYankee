# Documentación Técnica - OBS Propio

## Arquitectura General

### Visión General

OBS Propio es una aplicación de producción de video profesional para transmisiones deportivas, enfocada específicamente en fútbol. La arquitectura está dividida en tres capas principales:

1. **Core C++**: Motor de video de alto rendimiento
2. **Bridge Node.js**: Capa de comunicación entre C++ y JavaScript
3. **Electron + React**: Interfaz de usuario moderna y responsive

### Flujo de Datos

```
Video Input (FFmpeg)
    ↓
InputManager (C++)
    ↓
VideoSource → Decoder
    ↓
SceneEngine (compositing)
    ↓
SwitchingEngine (transiciones)
    ↓
OverlayEngine (marcador, overlays)
    ↓
OutputEngine (encoding/streaming)
    ↓
RTMP Output / Local Recording
```

## Componentes del Sistema

### 1. Core C++

#### InputManager
**Responsabilidades:**
- Detección de dispositivos de video
- Gestión de múltiples fuentes
- Enumeración de webcams, capturadoras, pantallas
- Control de ciclo de vida de inputs

**API Principal:**
```cpp
std::vector<InputDevice> enumerateDevices();
SourceId createInput(const InputConfig& config);
void destroyInput(SourceId id);
```

#### VideoSource
**Responsabilidades:**
- Decodificación de video con FFmpeg
- Gestión de buffers de frames
- Seek y playback control
- Soporte para archivos, dispositivos, streams

**Características:**
- Threading separado para decoding
- Frame pool para eficiencia de memoria
- Soporte para múltiples formatos (H.264, H.265, VP9)

#### Decoder
**Responsabilidades:**
- Integración con FFmpeg/libavcodec
- Conversión de formatos de pixel
- Hardware acceleration (NVENC, QuickSync, VCE)

#### SceneEngine
**Responsabilidades:**
- Composición de múltiples capas
- Gestión de escenas y transiciones
- Renderizado en tiempo real
- Preview vs Program monitors

**Estructura de Scene:**
```cpp
struct Scene {
    SceneId id;
    std::string name;
    std::vector<SceneLayer> layers;
    std::vector<std::shared_ptr<Overlay>> overlays;
};
```

#### SwitchingEngine
**Responsabilidades:**
- Transiciones suaves entre escenas
- Efectos de transición (fade, wipe, slide)
- Timing y easing functions

**Transiciones Disponibles:**
- Cut (instantáneo)
- Fade (fundido)
- Wipe (direccional)
- Slide (deslizamiento)
- Dissolve (disolución)

#### OverlayEngine
**Responsabilidades:**
- Sistema de capas de overlays
- Renderizado de marcadores deportivos
- Lower thirds dinámicos
- Imágenes y texto

**Implementaciones:**
- **FootballScoreboard**: Marcador de fútbol con equipos, score, tiempo
- **LowerThird**: Animación de entrada de jugadores
- **ImageOverlay**: Imágenes estáticas superpuestas
- **TextOverlay**: Texto dinámico

#### OutputEngine
**Responsabilidades:**
- Encoding a múltiples formatos
- Streaming RTMP a plataformas
- Grabación local (MP4, MKV)
- Detección de hardware acceleration

**Encoders Soportados:**
- x264 (CPU)
- NVENC (NVIDIA)
- QuickSync (Intel)
- VCE (AMD)

#### ReplayEngine
**Responsabilidades:**
- Buffer circular de video
- Extracción de segmentos
- Reproducción a velocidades variables
- Memory-mapped storage para eficiencia

**Configuración:**
```cpp
struct ReplayConfig {
    int durationSeconds = 300;  // 5 minutos
    Resolution resolution{1280, 720};
    bool useMemoryMapping = true;
    size_t maxMemoryMB = 1024;
};
```

#### HardwareDetector
**Responsabilidades:**
- Detección de GPU (NVIDIA, AMD, Intel)
- Detección de CPU cores/threads
- Medición de RAM disponible
- Recomendaciones automáticas de configuración

### 2. Bridge Node.js (N-API)

#### Arquitectura del Bridge
El bridge utiliza N-API para comunicación de alto rendimiento entre C++ y JavaScript:

```cpp
class OBSCore : public Napi::ObjectWrap<OBSCore> {
    // Input management
    Napi::Value CreateInput(const Napi::CallbackInfo& info);
    Napi::Value DestroyInput(const Napi::CallbackInfo& info);
    
    // Scene management
    Napi::Value CreateScene(const Napi::CallbackInfo& info);
    Napi::Value SetActiveScene(const Napi::CallbackInfo& info);
    
    // Streaming control
    Napi::Value StartStreaming(const Napi::CallbackInfo& info);
    Napi::Value StopStreaming(const Napi::CallbackInfo& info);
    
    // Hardware detection
    Napi::Value DetectHardware(const Napi::CallbackInfo& info);
};
```

#### API de JavaScript

```typescript
interface OBSCore {
    initialize(options: {
        logPath?: string;
        consoleLog?: boolean;
    }): Promise<boolean>;
    
    createInput(config: InputConfig): Promise<number>;
    destroyInput(id: number): Promise<void>;
    
    createScene(name: string): Promise<number>;
    setActiveScene(id: number): Promise<boolean>;
    
    startStreaming(config: StreamConfig): Promise<boolean>;
    stopStreaming(): Promise<void>;
    
    getFrame(timestamp: number): Promise<Frame>;
    
    detectHardware(): Promise<HardwareCapabilities>;
}
```

### 3. Electron + React UI

#### Arquitectura de la UI

**State Management: Redux**
- `coreSlice`: Fuentes, escenas, hardware capabilities
- `streamingSlice`: Estado de streaming/recording
- `replaySlice`: Buffer de replay
- `uiSlice**: Estado de la UI (scoreboard, tema)

**Componentes Principales:**
- **TopBar**: Controles de streaming/recording
- **SourcesPanel**: Lista de fuentes de video
- **ScenesPanel**: Gestión de escenas
- **MonitorView**: Preview y Program monitors con overlays
- **ControlsBar**: Controles de reproducción y replay
- **ReplayPanel**: Timeline del buffer de replay

#### Flujo de Datos en Redux

```
User Action → Action Creator → Middleware (async)
    ↓
Reducer → New State → UI Update
```

#### Integración con Bridge

```typescript
// En App.tsx
useEffect(() => {
    const initCore = async () => {
        const obs = require('obs-propio-bridge');
        const core = new obs.OBSCore();
        
        await core.initialize({
            logPath: app.getPath('logs'),
            consoleLog: isDev
        });
        
        const hwCaps = await core.detectHardware();
        dispatch(setHardwareCapabilities(hwCaps));
    };
    
    initCore();
}, []);
```

## Formatos y Protocolos

### Video Formats
- **Container**: MP4, MKV, MOV, AVI
- **Codecs**: H.264, H.265, VP8, VP9
- **Pixel Formats**: YUV420P, YUV422P, RGBA

### Streaming Protocols
- **RTMP**: Stream a YouTube, Twitch, Facebook
- **Local Recording**: MP4/MKV con codec configurable
- **Adaptive Bitrate**: Ajuste automático según hardware

### Audio Formats
- **Codecs**: AAC, MP3, Opus
- **Sample Rates**: 44.1kHz, 48kHz
- **Channels**: Mono, Stereo

## Performance Optimization

### Memory Management
- **Frame Pooling**: Reutilización de frames para evitar allocations
- **Circular Buffers**: Para replay engine (mmap)
- **Object Pooling**: Para overlays y scenas

### Threading Model
```
[Decoding Thread 1] → Frame Queue → [Scene Engine] → [Output Thread]
[Decoding Thread 2] ↗                              ↓
                                                    [Encoding Thread]
```

### GPU Acceleration
- **NVENC**: NVIDIA hardware encoding
- **QuickSync**: Intel hardware encoding
- **VCE**: AMD hardware encoding
- **Fallback**: x264 CPU encoding si no hay GPU

### Low-End Hardware Fallbacks
1. Reducir resolución de output (1080p → 720p → 480p)
2. Reducir framerate (60fps → 30fps → 24fps)
3. Reducir bitrate automáticamente
4. Desactivar features avanzados (blur, shadows)

## Known Limitations

### Current
1. No hay audio mixing avanzado
2. Transiciones limitadas a 5 tipos
3. Replay buffer limitado a RAM disponible
4. No hay sistema de plugins extensible

### Planned Improvements
1. Audio mixer con múltiples tracks
2. Más efectos de transición
3. Replay buffer con SSD caching
4. Sistema de plugins basado en WASM

## Debugging

### Enable Verbose Logging

```cpp
// En core/src/logger.cpp
Logger::instance().setLevel(spdlog::level::trace);
Logger::instance().initialize("", true);
```

### Debug Mode en UI

```typescript
// En renderer
localStorage.setItem('DEBUG', 'true');
console.log('OBS Core initialized', coreState);
```

### Performance Profiling

```bash
# Perf en Linux
perf record -p $(pgrep obs-propio) -g -- sleep 30
perf report

# VTune en Windows
vtune -collect hotspots -- ./obs-propio.exe
```

## Testing

### Unit Tests (C++)
```bash
cd core/build
ctest --output-on-failure
```

### Integration Tests (JavaScript)
```bash
cd tests/integration
npm test
```

### Manual Testing Checklist
- [ ] Agregar video file como source
- [ ] Cambiar entre scenes
- [ ] Agregar webcam
- [ ] Iniciar streaming RTMP
- [ ] Grabar video local
- [ ] Activar replay buffer
- [ ] Extraer replay de 30s
- [ ] Agregar scoreboard overlay
- [ ] Configurar lower third
- [ ] Cambiar resoluciones

## Security Considerations

### Input Validation
- Todos los strings de entrada se validan
- Paths de archivos se sanitizan
- URLs de streams se validan contra whitelist

### Resource Limits
- máximo de sources: 10 simultáneos
- máximo de scenes: 20
- máximo de overlays por scene: 15
- máximo de replay buffer: 10 minutos

### Network Security
- RTMP sobre TLS cuando sea posible
- Validación de certificados
- Timeout de conexión: 30 segundos

## Extensibility

### Adding New Input Types

1. Extender `InputType` enum en `common.hpp`
2. Implementar detección en `InputManager::enumerateDevices()`
3. Agregar case en `VideoSource::openInput()`

### Adding New Transitions

1. Agregar tipo a `TransitionType` enum
2. Implementar en `SwitchingEngine::apply*Transition()`
3. Agregar opción en UI

### Adding New Overlays

1. Heredar de `Overlay` base class
2. Implementar `render()` y `update()`
3. Registrar en `OverlayEngine`
4. Agregar configuración en UI

## License and Attribution

This project uses:
- FFmpeg LGPL/GPL libraries
- spdlog logging library (MIT)
- nlohmann/json library (MIT)
- React (MIT)
- Electron (MIT)

Full license texts in `LICENSE` and `THIRD_PARTY_LICENSES.md`.