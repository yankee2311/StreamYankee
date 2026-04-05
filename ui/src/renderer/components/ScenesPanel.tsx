import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from '../store';
import { setActiveScene, addScene, removeScene } from '../store/slices/coreSlice';

const ScenesPanel: React.FC = () => {
  const dispatch = useDispatch();
  const { scenes, activeScene } = useSelector((state: RootState) => state.core);

  const handleAddScene = () => {
    const newId = Date.now();
    dispatch(addScene({ id: newId, name: `Scene ${scenes.length + 1}`, sources: [], active: false }));
  };

  const handleSelectScene = (id: number) => {
    dispatch(setActiveScene(id));
  };

  const handleRemoveScene = (id: number) => {
    dispatch(removeScene(id));
  };

  return (
    <div className="scenes-panel">
      <div className="panel-header">
        <span>Scenes</span>
        <button onClick={handleAddScene}>+</button>
      </div>
      <ul className="scene-list">
        {scenes.map((scene) => (
          <li
            key={scene.id}
            className={`scene-item ${activeScene === scene.id ? 'active' : ''}`}
            onClick={() => handleSelectScene(scene.id)}
          >
            <span>{scene.name}</span>
            <button
              className="small"
              onClick={(e) => {
                e.stopPropagation();
                handleRemoveScene(scene.id);
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

export default ScenesPanel;