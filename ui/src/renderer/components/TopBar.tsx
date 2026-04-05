import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from '../store';

const TopBar: React.FC = () => {
  const dispatch = useDispatch();
  const { isStreaming, isRecording } = useSelector((state: RootState) => state.streaming);

  return (
    <div className="top-bar">
      <h1>OBS Propio</h1>
      <div className="top-bar-controls">
        <button 
          className={isStreaming ? 'danger' : 'success'}
          onClick={() => {
            if (isStreaming) {
              console.log('Stop streaming');
            } else {
              console.log('Start streaming');
            }
          }}
        >
          {isStreaming ? 'Stop Streaming' : 'Start Streaming'}
        </button>
        <button 
          className={isRecording ? 'danger' : 'secondary'}
          onClick={() => {
            if (isRecording) {
              console.log('Stop recording');
            } else {
              console.log('Start recording');
            }
          }}
        >
          {isRecording ? 'Stop Recording' : 'Start Recording'}
        </button>
      </div>
    </div>
  );
};

export default TopBar;