import React, { useRef, useEffect } from 'react';
import { useSelector } from 'react-redux';
import { RootState } from '../store';

const MonitorView: React.FC = () => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { showScoreboard, matchData } = useSelector((state: RootState) => state.ui);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const drawFrame = () => {
      ctx.fillStyle = '#000000';
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      ctx.fillStyle = '#0078d4';
      ctx.fillRect(100, 100, 200, 150);

      const text = 'Preview';
      ctx.font = '24px Arial';
      ctx.fillStyle = '#ffffff';
      ctx.textAlign = 'center';
      ctx.fillText(text, canvas.width / 2, canvas.height / 2);

      requestAnimationFrame(drawFrame);
    };

    drawFrame();
  }, []);

  return (
    <div className="monitors">
      <div className="monitor-wrapper">
        <div className="monitor-label">Preview</div>
        <div className="monitor preview">
          <canvas ref={canvasRef} width={960} height={540} />
          {showScoreboard && (
            <div className="scoreboard-overlay">
              <div className="team-info">
                <div className="team-logo"></div>
                <div className="team-name">{matchData.team1Name}</div>
                <div className="team-score">{matchData.team1Score}</div>
              </div>
              <div className="match-info">
                <div className="match-time">{matchData.minute}'</div>
                <div className="match-period">{matchData.period}</div>
              </div>
              <div className="team-info">
                <div className="team-logo"></div>
                <div className="team-name">{matchData.team2Name}</div>
                <div className="team-score">{matchData.team2Score}</div>
              </div>
            </div>
          )}
        </div>
      </div>

      <div className="monitor-wrapper">
        <div className="monitor-label">Program</div>
        <div className="monitor program">
          <canvas ref={canvasRef} width={960} height={540} />
          {showScoreboard && (
            <div className="scoreboard-overlay">
              <div className="team-info">
                <div className="team-logo"></div>
                <div className="team-name">{matchData.team1Name}</div>
                <div className="team-score">{matchData.team1Score}</div>
              </div>
              <div className="match-info">
                <div className="match-time">{matchData.minute}'</div>
                <div className="match-period">{matchData.period}</div>
              </div>
              <div className="team-info">
                <div className="team-logo"></div>
                <div className="team-name">{matchData.team2Name}</div>
                <div className="team-score">{matchData.team2Score}</div>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default MonitorView;