import { useEffect, useRef } from "react";

/**
 * 监听 `<html data-acrylic>` 属性，实时切换 Tauri 原生窗口效果。
 * 预览（mgr.preview）和保存（mgr.set）都会修改此属性，因此始终保持同步。
 */
export function useNativeGlass() {
  const applied = useRef(false);

  useEffect(() => {
    const check = async () => {
      const isTauri = typeof window !== "undefined" && (window as any).__TAURI_INTERNALS__;
      if (!isTauri) return;

      const enabled = document.documentElement.hasAttribute("data-acrylic");
      if (enabled === applied.current) return;
      applied.current = enabled;

      try {
        const { getCurrentWindow } = await import("@tauri-apps/api/window");
        const win = getCurrentWindow();
        const platform = navigator.userAgent.toLowerCase();

        if (enabled) {
          if (platform.includes("windows")) {
            await (win as any).setEffects({ effects: [{ effect: "mica" }] });
          } else if (platform.includes("mac os")) {
            await (win as any).setEffects({ effects: [{ effect: "fullScreenUI", radius: 10 }] });
          } else {
            await (win as any).setEffects({ effects: [{ effect: "blur", radius: 10 }] });
          }
        } else {
          await (win as any).clearEffects();
        }
      } catch {
        // Tauri API 不可用 → 回退 CSS backdrop-filter
      }
    };

    // Check immediately, then poll
    check();
    const id = setInterval(check, 300);
    return () => clearInterval(id);
  }, []);
}
