# Guía de Configuración de GitHub Actions

## Resumen

Este proyecto incluye **6 workflows de GitHub Actions** completamente configurados para automatizar el build, test y release.

---

## Workflows Incluidos

### 1. `build.yml` - Build Principal

**Disparadores:**
- Push a `main` o `develop`
- Pull requests a `main`
- Manual (workflow_dispatch)

**Plataformas:**
- ✅ Windows (x64)
- ✅ Linux (x64)
- ✅ macOS (x64/ARM64)

**Pasos:**
1. Checkout del código
2. Instalar dependencias (CMake, Node.js, FFmpeg, etc.)
3. Compilar Core C++
4. Compilar Bridge Node.js
5. Compilar UI Electron + React
6. Ejecutar tests
7. Generar ejecutables/instaladores
8. Subir artefactos

**Cache:**
- Cache de Core C++ compilado
- Cache de node_modules
- Cache de CMake

---

### 2. `release.yml` - Crear Releases

**Disparadores:**
- Release creado en GitHub
- Manual (workflow_dispatch)

**Funcionalidades:**
- Actualiza versión en package.json
- Compila para las 3 plataformas
- Genera instaladores:
  - Windows: `.exe` (NSIS)
  - Linux: `.AppImage`, `.deb`
  - macOS: `.dmg`, `.zip`
- Sube artefactos al release
- Crea notas de release

---

### 3. `test.yml` - Tests Automatizados

**Disparadores:**
- Push a `main` o `develop`
- Pull requests
- Diario a las 2 AM UTC

**Tests:**
- ✅ Tests unitarios C++ (Linux, Windows, macOS)
- ✅ Tests de integración Node.js
- ✅ Code quality checks (ESLint, cpplint)
- ✅ Security audit
- ✅ Performance benchmarks

---

### 4. `security.yml` - Análisis de Seguridad

**Disparadores:**
- Push a `main`
- Pull requests
- Diario a las 4 AM UTC

**Herramientas:**
- npm audit
- Snyk security scan
- CodeQL analysis
- Semgrep

---

### 5. `dependabot.yml` - Actualizaciones Automáticas

**Configuración:**
- Actualizaciones semanales de dependencias npm
- Actualizaciones de GitHub Actions
- Auto-merge para dependencias seguras
- Notificaciones de actualizaciones mayores

---

### 6. `docs.yml` - Documentación

**Disparadores:**
- Push a `main` que afecte `docs/`
- Manual

**Genera:**
- Documentación de API (TypeDoc)
- Documentación de C++ (Doxygen)
- Deploy a GitHub Pages

---

## Configuración Inicial

### 1. Habilitar GitHub Actions

1. Ve a tu repositorio en GitHub
2. Click en "Settings" → "Actions" → "General"
3. Selecciona "Allow all actions and reusable workflows"
4. Click "Save"

### 2. Configurar Secrets (Opcional para releases)

1. Ve a "Settings" → "Secrets and variables" → "Actions"
2. Agregar los siguientes secrets:

```
GITHUB_TOKEN (automático)
SNYK_TOKEN (opcional, para security scan)
```

### 3. Configurar Branch Protection (Recomendado)

1. Ve a "Settings" → "Branches"
2. Click "Add rule" para `main`
3. Configurar:
   - ✅ Require status checks to pass before merging
   - ✅ Require branches to be up to date before merging
   - Status checks requeridos:
     - `build-windows`
     - `build-linux`
     - `build-macos`
     - `test-integration`
   - ✅ Require signed commits (opcional)

---

## Uso

### Build Automático

Cada push a `main` o `develop` disparará:

```yaml
# Automático
git push origin main
# GitHub Actions compilará para Windows, Linux y macOS
```

### Crear un Release

1. Ve a "Releases" en GitHub
2. Click "Draft a new release"
3. Agregar tag (ej: `v1.0.0`)
4. Escribir release notes
5. Click "Publish release"
6. GitHub Actions automáticamente:
   - Actualizará versiones
   - Compilará para todas las plataformas
   - Subirá los artefactos

**O manual:**

```bash
# Crear tag
git tag v1.0.0
git push origin v1.0.0

# O usar workflow_dispatch
gh workflow run release.yml -f version=1.0.0
```

### Ejecutar Tests Manualmente

```bash
# Usando gh CLI
gh workflow run test.yml

# O desde la UI
# Actions → test.yml → Run workflow
```

---

## Artefactos Generados

### Windows
- `OBS-Propio-Setup-{version}.exe` - Instalador NSIS
- `OBS-Propio-{version}-win.exe` - Ejecutable portable

### Linux
- `OBS-Propio-{version}.AppImage` - AppImage portátil
- `obs-propio_{version}_amd64.deb` - Paquete Debian

### macOS
- `OBS-Propio-{version}.dmg` - Imagen de disco
- `OBS-Propio-{version}-mac.zip` - Archivo comprimido

---

## Badges para README

Agrega estos badges al README.md:

```markdown
[![Build Status](https://github.com/TU_USUARIO/obs-propio/workflows/Build/badge.svg)](https://github.com/TU_USUARIO/obs-propio/actions)
[![Tests](https://github.com/TU_USUARIO/obs-propio/workflows/Test/badge.svg)](https://github.com/TU_USUARIO/obs-propio/actions)
[![Security](https://github.com/TU_USUARIO/obs-propio/workflows/Security/badge.svg)](https://github.com/TU_USUARIO/obs-propio/actions)
[![Release](https://img.shields.io/github/v/release/TU_USUARIO/obs-propio)](https://github.com/TU_USUARIO/obs-propio/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
```

---

## Costos de GitHub Actions

### Tiempo de Ejecución Estimado

| Workflow | Windows | Linux | macOS | Total |
|----------|---------|-------|-------|-------|
| build.yml | ~8 min | ~6 min | ~10 min | ~24 min |
| test.yml | ~3 min | ~2 min | ~4 min | ~9 min |
| release.yml | ~8 min | ~6 min | ~10 min | ~24 min |
| security.yml | ~5 min | - | - | ~5 min |

**Total mensual estimado:** ~2 horas × 30 días = ~60 horas

### Plan Gratuito de GitHub

- **Límite:** 2,000 minutos/mes para repositorios privados
- **Repos públicos:** Sin límite
- **Consejo:** Usar cache agresivamente para reducir tiempo

---

## Solución de Problemas

### Error: "CMake not found"

**Solución:**
- El workflow usa `lukka/get-cmake` que descarga CMake automáticamente
- Si falla, verificar conexión a GitHub

### Error: "FFmpeg not found"

**Solución:**
- Windows: `choco install ffmpeg -y`
- Linux: `sudo apt-get install ffmpeg`
- macOS: `brew install ffmpeg`

### Error: "Node module not found"

**Solución:**
- Ilmplitar cache de npm: Actions → Caches → Delete cache
- Verificar que package-lock.json esté actualizado

### Error: "Build timeout"

**Solución:**
- Aumentar timeout en workflow:
  ```yaml
  - name: Build
    run: cmake --build . --parallel 4
    timeout-minutes: 30
  ```

---

## Mejores Prácticas

1. **Siempre verificar el build localmente antes de push**
   ```bash
   cd scripts
   .\build_complete.ps1
   ```

2. **Usar branches de feature**
   ```bash
   git checkout -b feature/mi-feature
   git push origin feature/mi-feature
   # Crear PR para disparar tests
   ```

3. **Revisar logs de Actions**
   - Click en el workflow run
   - Click en el job fallido
   - Revisar el paso que falló

4. **Usar cache efectivamente**
   - Los workflows ya tienen cache configurado
   - No modificar cache paths sin actualizar keys

---

## Próximos Pasos

1. ✅ Workflows creados en `.github/workflows/`
2. ✅ Dependabot configurado
3. ✅ Actions composite para setup

**Para activar:**
1. Push a GitHub
2. Ir a "Actions" tab
3. Verificar que los workflows aparezcan
4. Ejecutar manualmente el primer build: `gh workflow run build.yml`

---

## Soporte

Para problemas con GitHub Actions:
- [Documentación de GitHub Actions](https://docs.github.com/en/actions)
- [Comunidad de GitHub](https://github.community/)
- Crear un issue en el repositorio con la etiqueta `github-actions`