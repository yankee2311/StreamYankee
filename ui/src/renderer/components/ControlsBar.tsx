import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from '../store';
import { startStreaming, stopStreaming, startRecording, stopRecording } from '../store/slices/streamingSlice';

const ControlsBar: React.FC = () => {
  const dispatch = useDispatch();
  const { isStreaming, isRecording, streamTime, recordTime } = useSelector(
    (state: RootState) => state.streaming
  );
  const { isReplayBuffering } = useSelector((state: RootState) => state.replay);

  const formatTime = (seconds: number): string => {
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = seconds % 60;
    return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
  };

  return (
    <div className="controls-bar">
      <button
        className={isStreaming ? 'danger' : 'success'}
        onClick={() => {
          if (isStreaming) {
            dispatch(stopStreaming());
          } else {
            dispatch(startStreaming());
          }
        }}
      >
        {isStreaming ? 'Stop Streaming' : 'Start Streaming'}
      </button>

      {isStreaming && (
        <div style={{ color: '#fff', fontFamily: 'monospace', fontSize: '18px' }}>
          {formatTime(streamTime)}
        </div>
      )}

      <button
        className={isRecording ? 'danger' : 'secondary'}
        onClick={() => {
          if (isRecording) {
            dispatch(stopRecording());
          } else {
            dispatch(startRecording('/tmp/recording.mp4'));
          }
        }}
      >
        {isRecording ? 'Stop Recording' : 'Start Recording'}
      </button>

      {isRecording && (
        <div style={{ color: '#fff', fontFamily: 'monospace', fontSize: '18px' }}>
          {formatTime(recordTime)}
        </div>
      )}

      <button
        className={isReplayBuffering ? 'danger' : 'secondary'}
        onClick={() => {
          console.log('Replay buffer');
        }}
      >
        {isReplayBuffering ? 'Save Replay' : 'Start Replay Buffer'}
      </button>
    </div>
  );
};

export default ControlsBar;