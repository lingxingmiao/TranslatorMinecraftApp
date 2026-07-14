import { type ThemeConfig, type ThemeMode } from "@/lib/theme";
import { THEME_SAT_FACTORS } from "@/lib/theme";

const STORAGE_KEY = "theme-config";

const defaultTheme: ThemeConfig = {
  preset: "forest",
  mode: "dark",
  glassEnabled: true,
  customHue: null,
  bgImage: null,
  blurIntensity: 5,
};

function hsl(h: number, s: number, l: number) {
  return `${h} ${s}% ${l}%`;
}

const BG_HUE = 220;

function colorSetFor(mode: ThemeMode, h: number, satFactor: number): Record<string, string> {
  const s = (sat: number) => sat * satFactor;
  const isDark = mode === "dark";
  return {
    "--background": hsl(BG_HUE, isDark ? 30 : 20, isDark ? 6 : 96),
    "--foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 15),
    "--card": hsl(BG_HUE, isDark ? 28 : 18, isDark ? 10 : 98),
    "--card-foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 20),
    "--popover": hsl(BG_HUE, isDark ? 28 : 18, isDark ? 10 : 98),
    "--popover-foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 20),
    "--primary": hsl(h, s(60), isDark ? 65 : 50),
    "--primary-foreground": "0 0% 100%",
    "--secondary": hsl(BG_HUE, isDark ? 25 : 15, isDark ? 16 : 88),
    "--secondary-foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 20),
    "--muted": hsl(BG_HUE, isDark ? 25 : 15, isDark ? 16 : 88),
    "--muted-foreground": hsl(BG_HUE, isDark ? 15 : 10, isDark ? 60 : 50),
    "--accent": hsl(h, s(40), isDark ? 55 : 45),
    "--accent-foreground": "0 0% 100%",
    "--destructive": "0 70% 50%",
    "--destructive-foreground": "0 0% 100%",
    "--border": hsl(BG_HUE, isDark ? 25 : 15, isDark ? 18 : 82),
    "--input": hsl(BG_HUE, isDark ? 25 : 15, isDark ? 18 : 82),
    "--ring": hsl(h, s(60), isDark ? 65 : 55),
    "--sidebar-background": hsl(BG_HUE, isDark ? 26 : 18, isDark ? 8 : 94),
    "--sidebar-foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 15),
    "--sidebar-primary": hsl(h, s(60), isDark ? 65 : 50),
    "--sidebar-primary-foreground": "0 0% 100%",
    "--sidebar-accent": hsl(BG_HUE, isDark ? 23 : 15, isDark ? 14 : 86),
    "--sidebar-accent-foreground": hsl(BG_HUE, isDark ? 20 : 15, isDark ? 85 : 15),
    "--sidebar-border": hsl(BG_HUE, isDark ? 25 : 15, isDark ? 16 : 80),
    "--sidebar-ring": hsl(h, s(60), isDark ? 65 : 55),
  };
}

const PRESET_HUES: Record<string, number> = {
  iris: 252, ocean: 210, forest: 160, sunset: 30, mono: 220,
};

function systemDarkMode(): boolean {
  if (typeof window === "undefined") return true;
  return window.matchMedia("(prefers-color-scheme: dark)").matches;
}

export function createThemeManager() {
  let current: ThemeConfig = { ...defaultTheme };

  try {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (saved) {
      current = { ...defaultTheme, ...JSON.parse(saved) };
    } else {
      current.mode = systemDarkMode() ? "dark" : "light";
    }
  } catch {
    current.mode = systemDarkMode() ? "dark" : "light";
  }

  function save() {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(current));
  }

  function doApply() {
    const doc = document.documentElement;
    const h = current.customHue !== null ? current.customHue : (PRESET_HUES[current.preset] ?? 252);
    const satFactor = current.customHue !== null ? 1 : (THEME_SAT_FACTORS[current.preset] ?? 1);

    if (current.mode === "dark") doc.classList.add("dark");
    else doc.classList.remove("dark");

    // Glass attribute: data-acrylic (pure) or data-bg-enabled (image)
    if (current.glassEnabled && current.bgImage) {
      doc.removeAttribute("data-acrylic");
      doc.setAttribute("data-bg-enabled", "true");
    } else if (current.glassEnabled) {
      doc.removeAttribute("data-bg-enabled");
      doc.setAttribute("data-acrylic", "true");
    } else {
      doc.removeAttribute("data-acrylic");
      doc.removeAttribute("data-bg-enabled");
    }

    // Blur intensity as CSS custom property
    doc.style.setProperty("--glass-blur", `${current.blurIntensity * 2}px`);

    const colors = colorSetFor(current.mode, h, satFactor);
    Object.entries(colors).forEach(([key, val]) => {
      doc.style.setProperty(key, val);
    });
  }

  function get(): ThemeConfig {
    return { ...current };
  }

  function set(config: Partial<ThemeConfig>) {
    current = { ...current, ...config };
    save();
    doApply();
  }

  function preview(config: Partial<ThemeConfig>) {
    current = { ...current, ...config };
    doApply();
  }

  function restore(snapshot: ThemeConfig) {
    current = { ...snapshot };
    doApply();
  }

  const mediaQuery = typeof window !== "undefined"
    ? window.matchMedia("(prefers-color-scheme: dark)")
    : null;

  function onSystemChange(e: MediaQueryListEvent) {
    const saved = localStorage.getItem(STORAGE_KEY);
    if (!saved) {
      current.mode = e.matches ? "dark" : "light";
      doApply();
    }
  }

  mediaQuery?.addEventListener("change", onSystemChange);

  doApply();
  return { get, set, preview, restore };
}

export type ThemeManager = ReturnType<typeof createThemeManager>;
