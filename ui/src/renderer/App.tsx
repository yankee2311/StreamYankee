import React, { useEffect, useState } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from './store';
import { setHardwareCapabilities, addSource, addScene, setActiveScene } from './store/slices/coreSlice';
import { startStreaming, stopStreaming } from './store/slices/streamingSlice';
import TopBar from './components/TopBar';
import SourcesPanel from './components/SourcesPanel';
import ScenesPanel from './components/ScenesPanel';
import MonitorView from './components/MonitorView';
import ControlsBar from './components/ControlsBar';
import ReplayPanel from './components/ReplayPanel';
import './index.css';

declare global {
  interface Window {
    electronAPI: {
      detectHardware: () => Promise<any>;
      getVideoDevices: () => Promise<any[]>;
      getAudioDevices: () => Promise<any[]>;
      on: (channel: string, callback: (...args: any[]) => void) => void;
      removeListener: (channel: string, callback: (...args: any[]) => void) => void;
    };
  }
}

const App: React.FC = () => {
  const dispatch = useDispatch();
  const { hardwareCapabilities } = useSelector((state: RootState) => state.core);
  const { isStreaming, isRecording } = useSelector((state: RootState) => state.streaming);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    initializeApp();
    setupEventListeners();
    
    return () => {
      cleanupEventListeners();
    };
  }, []);

  const initializeApp = async () => {
    try {
      if (window.electronAPI) {
        const hwCaps = await window.electronAPI.detectHardware();
        dispatch(setHardwareCapabilities(hwCaps));
      } else {
        dispatch(setHardwareCapabilities({
          hasNVENC: false,
          hasQuickSync: false,
          hasVCE: false,
          cpuCores: 4,
          cpuThreads: 8,
          totalRAM: 8 * 1024 * 1024 * 1024,
          availableRAM: 4 * 1024 * 1024 * 1024,
          recommendedResolution: 720,
          recommendedFPS: 30,
          recommendedBitrate: 2500
        }));
      }
      
      dispatch(addScene({ name: 'Live', id: 1 }));
      dispatch(addScene({ name: 'Replay', id: 2 }));
      dispatch(setActiveScene(1));
      
      setIsLoading(false);
    } catch (error) {
      console.error('Failed to initialize app:', error);
      setIsLoading(false);
    }
  };

  const setupEventListeners = () => {
    if (!window.electronAPI) return;
    
    window.electronAPI.on('add-source', (type: string) => {
      handleAddSource(type);
    });
    
    window.electronAPI.on('start-streaming', () => {
      dispatch(startStreaming());
    });
    
    window.electronAPI.on('stop-streaming', () => {
      dispatch(stopStreaming());
    });
  };

  const cleanupEventListeners = () => {
    if (!window.electronAPI) return;
  };

  const handleAddSource = (type: string) => {
    const sourceId = Date.now();
    dispatch(addSource({
      id: sourceId,
      name: `Source ${sourceId}`,
      type: type
    }));
  };

  if (isLoading) {
    return (
      <div className="app" style={{ justifyContent: 'center', alignItems: 'center' }}>
        <div style={{ textAlign: 'center' }}>
          <h2>Cargando OBS Propio...</h2>
          <p style={{ marginTop: '8px', color: '#999' }}>Inicializando motor de video...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="app">
      <TopBar />
      <div className="main-container">
        <SourcesPanel />
        <div className="preview-program-container">
          <MonitorView />
          <ControlsBar />
        </div>
        <ScenesPanel />
      </div>
      <ReplayPanel />
    </div>
  );
};

export default App;