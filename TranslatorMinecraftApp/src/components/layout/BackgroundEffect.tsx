"use client";

import { useEffect, useRef } from "react";

interface BackgroundEffectProps {
  glassEnabled: boolean;
  bgImage: string | null;
  blurIntensity: number;
}

/**
 * 这里是全局毛玻璃 + 背景图片控制器zwz
 *
 * - 纯毛玻璃：设置 data-acrylic，CSS 联动让原生窗口效果透出
 * - 图片模式：设置 data-bg-enabled + 渲染图片层在内容下方
 */
export function BackgroundEffect({ glassEnabled, bgImage, blurIntensity }: BackgroundEffectProps) {
  const containerRef = useRef<HTMLDivElement | null>(null);

  useEffect(() => {
    const doc = document.documentElement;

    if (glassEnabled && bgImage) {
      doc.setAttribute("data-bg-enabled", "true");
      doc.removeAttribute("data-acrylic");
    } else if (glassEnabled && !bgImage) {
      doc.setAttribute("data-acrylic", "true");
      doc.removeAttribute("data-bg-enabled");
    } else {
      doc.removeAttribute("data-acrylic");
      doc.removeAttribute("data-bg-enabled");
    }
  }, [glassEnabled, bgImage]);

  if (!glassEnabled || !bgImage) return null;

  const b = blurIntensity;

  return (
    <div
      ref={containerRef}
      className="fixed inset-0 pointer-events-none overflow-hidden"
      style={{ zIndex: -1 }}
    >
      {/* Blurred image */}
      <div
        className="absolute inset-0"
        style={{
          backgroundImage: `url(${bgImage})`,
          backgroundSize: "cover",
          backgroundPosition: "center",
          backgroundRepeat: "no-repeat",
          filter: `blur(${b * 2.5}px) brightness(0.5)`,
          transform: "scale(1.08)",
        }}
      />
      {/* Dark overlay to keep text readable */}
      <div className="absolute inset-0 bg-background/40" />
    </div>
  );
}
