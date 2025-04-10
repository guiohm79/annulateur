import React, { useEffect, useRef } from 'react';

interface VUMeterProps {
  level: number; // 0-100
  width?: number;
  height?: number;
  className?: string;
}

export const VUMeter: React.FC<VUMeterProps> = ({ 
  level, 
  width = 200, 
  height = 50,
  className = ''
}) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Effacer le canvas
    ctx.clearRect(0, 0, width, height);

    // Définir les couleurs en fonction du niveau
    let gradient;
    if (level < 70) {
      gradient = ctx.createLinearGradient(0, 0, width, 0);
      gradient.addColorStop(0, '#3b82f6');  // Bleu
      gradient.addColorStop(0.7, '#10b981'); // Vert
      gradient.addColorStop(1, '#f59e0b');   // Jaune
    } else {
      gradient = ctx.createLinearGradient(0, 0, width, 0);
      gradient.addColorStop(0, '#3b82f6');   // Bleu
      gradient.addColorStop(0.5, '#10b981');  // Vert
      gradient.addColorStop(0.7, '#f59e0b');  // Jaune
      gradient.addColorStop(0.9, '#ef4444');  // Rouge
    }

    // Dessiner la barre de niveau
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width * (level / 100), height);

    // Dessiner les graduations
    ctx.fillStyle = '#1f2937';
    for (let i = 1; i < 10; i++) {
      ctx.fillRect(width * (i / 10) - 1, 0, 1, height);
    }

    // Dessiner un indicateur de crête
    if (level > 80) {
      ctx.fillStyle = '#ef4444';
      ctx.fillRect(width * (level / 100) - 2, 0, 2, height);
    }
  }, [level, width, height]);

  return (
    <canvas 
      ref={canvasRef} 
      width={width} 
      height={height}
      className={className}
    />
  );
};
