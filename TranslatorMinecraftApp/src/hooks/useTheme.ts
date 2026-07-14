import { useState, useCallback } from "react";
import type { ThemeConfig, ThemePreset, ThemeMode } from "@/lib/theme";
import { createThemeManager, type ThemeManager } from "@/lib/theme-manager";

let globalManager: ThemeManager | null = null;

function getManager(): ThemeManager {
  if (!globalManager) {
    globalManager = createThemeManager();
  }
  return globalManager;
}

export function useTheme() {
  const mgr = getManager();
  const [theme, setThemeState] = useState<ThemeConfig>(() => mgr.get());

  const setPreset = useCallback((preset: ThemePreset) => {
    mgr.set({ preset, customHue: null });
    setThemeState(mgr.get());
  }, []);

  const setMode = useCallback((mode: ThemeMode) => {
    mgr.set({ mode });
    setThemeState(mgr.get());
  }, []);

  const toggleMode = useCallback(() => {
    const nextMode = theme.mode === "dark" ? "light" : "dark";
    mgr.set({ mode: nextMode });
    setThemeState(mgr.get());
  }, [theme.mode]);

  const setGlassEnabled = useCallback((enabled: boolean) => {
    mgr.set({ glassEnabled: enabled });
    setThemeState(mgr.get());
  }, []);

  const setCustomHue = useCallback((hue: number) => {
    mgr.set({ customHue: hue });
    setThemeState(mgr.get());
  }, []);

  const clearCustomHue = useCallback(() => {
    mgr.set({ customHue: null });
    setThemeState(mgr.get());
  }, []);

  return {
    theme,
    setPreset,
    setMode,
    toggleMode,
    setGlassEnabled,
    setCustomHue,
    clearCustomHue,
  };
}
