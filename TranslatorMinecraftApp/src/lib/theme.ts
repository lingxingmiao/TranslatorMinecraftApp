export type ThemePreset = "iris" | "ocean" | "forest" | "sunset" | "mono";
export type ThemeMode = "dark" | "light";
export type AppLocale = "zh-CN" | "en-US";

export interface ThemeConfig {
  preset: ThemePreset;
  mode: ThemeMode;
  glassEnabled: boolean;
  customHue: number | null;
  bgImage: string | null;
  blurIntensity: number;
}

export const THEME_HUES: Record<ThemePreset, number> = {
  iris: 252, ocean: 210, forest: 160, sunset: 30, mono: 0,
};

export const THEME_SAT_FACTORS: Record<ThemePreset, number> = {
  iris: 1, ocean: 1, forest: 1, sunset: 1, mono: 0.04,
};

export const THEME_LABELS: Record<ThemePreset, { zh: string; en: string }> = {
  iris: { zh: "鸢尾紫 Iris", en: "Iris Purple" },
  ocean: { zh: "深海蓝 Ocean", en: "Ocean Blue" },
  forest: { zh: "森林绿 Forest", en: "Forest Green" },
  sunset: { zh: "落日橙 Sunset", en: "Sunset Orange" },
  mono: { zh: "极简灰 Mono", en: "Mono Gray" },
};
