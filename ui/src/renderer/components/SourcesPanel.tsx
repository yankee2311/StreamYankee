import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from '../store';
import { selectSource, removeSource } from '../store/slices/coreSlice';

const SourcesPanel: React.FC = () => {
  const dispatch = useDispatch();
  const { sources } = useSelector((state: RootState) => state.core);
  const { selectedSource } = useSelector((state: RootState) => state.ui);

  const handleAddSource = () => {
    console.log('Add source');
  };

  const handleRemoveSource = (id: number) => {
    dispatch(removeSource(id));
  };

  const handleSelectSource = (id: number) => {
    dispatch(selectSource(id));
  };

  return (
    <div className="sources-panel">
      <div className="panel-header">
        <span>Sources</span>
        <button onClick={handleAddSource}>+</button>
      </div>
      <ul className="source-list">
        {sources.map((source) => (
          <li
            key={source.id}
            className={`source-item ${selectedSource === source.id ? 'active' : ''}`}
            onClick={() => handleSelectSource(source.id)}
          >
            <span>{source.name}</span>
            <button
              className="small"
              onClick={(e) => {
                e.stopPropagation();
                handleRemoveSource(source.id);
              }}
            >
              ×
            </button>
          </li>
        ))}
      </ul>
    </div>
  );
};

export default SourcesPanel;