import { useCallback, useEffect, useRef, useState } from "react";

type TauriWindow = {
  minimize: () => Promise<void>;
  toggleMaximize: () => Promise<void>;
  close: () => Promise<void>;
};

/**
 * Safely access Tauri window API.
 * Only loads @tauri-apps/api/window when running inside Tauri webview.
 * Vite's optimizeDeps.exclude prevents it from caching a stale version.
 */
export function useTauriWindow() {
  const [win, setWin] = useState<TauriWindow | null>(null);
  const [isTauri, setIsTauri] = useState(false);
  const loaded = useRef(false);

  useEffect(() => {
    if (loaded.current) return;
    loaded.current = true;

    (async () => {
      try {
        const { getCurrentWindow } = await import("@tauri-apps/api/window");
        const currentWindow = getCurrentWindow();
        setWin({
          minimize: () => currentWindow.minimize(),
          toggleMaximize: () => currentWindow.toggleMaximize(),
          close: () => currentWindow.close(),
        });
        setIsTauri(true);
      } catch {
        // Not running inside Tauri webview
        setIsTauri(false);
      }
    })();
  }, []);

  const minimize = useCallback(() => win?.minimize(), [win]);
  const toggleMaximize = useCallback(() => win?.toggleMaximize(), [win]);
  const close = useCallback(() => win?.close(), [win]);

  return { isTauri, minimize, toggleMaximize, close };
}
