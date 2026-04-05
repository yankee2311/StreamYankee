import React from 'react';
import { useSelector } from 'react-redux';
import { RootState } from '../store';

const ReplayPanel: React.FC = () => {
  const { isActive, segments, currentSpeed } = useSelector((state: RootState) => state.replay);

  return (
    <div className="bottom-panel">
      <div className="replay-panel">
        <div className="panel-header">
          <span>Replay Buffer {isActive ? '●' : '○'}</span>
          <span>Speed: {currentSpeed}x</span>
        </div>
        <div className="replay-timeline">
          {segments.map((segment, index) => (
            <div
              key={segment.id}
              style={{
                position: 'absolute',
                left: `${(segment.startTime / 300) * 100}%`,
                width: `${(segment.duration / 300) * 100}%`,
                height: '20px',
                backgroundColor: '#0078d4',
                borderRadius: '2px',
                cursor: 'pointer'
              }}
              title={`${segment.startTime.toFixed(1)}s - ${segment.endTime.toFixed(1)}s`}
            />
          ))}
        </div>
      </div>
    </div>
  );
};

export default ReplayPanel;