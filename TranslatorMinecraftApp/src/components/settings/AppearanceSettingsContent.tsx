import { useRef } from "react";
import { useTranslation } from "react-i18next";
import type { ThemePreset } from "@/lib/theme";
import type { ThemeConfig } from "@/lib/theme";
import { THEME_LABELS } from "@/lib/theme";
import { Button } from "@/components/ui/button";
import { Slider } from "@/components/ui/slider";
import { Switch } from "@/components/ui/switch";
import { NumberInput } from "@/components/ui/number-input";
import { Card, CardContent } from "@/components/ui/card";
import { Label } from "@/components/ui/label";
import { Sun, Moon, Sparkles, Palette, Image, Trash2, Eye } from "lucide-react";

interface Props {
  theme: ThemeConfig;
  onPresetChange: (preset: ThemePreset) => void;
  onModeToggle: () => void;
  onGlassChange: (enabled: boolean) => void;
  onHueChange: (hue: number) => void;
  onHueClear: () => void;
  onBgImageChange: (dataUrl: string | null) => void;
  onBlurChange: (intensity: number) => void;
}

const themes: ThemePreset[] = ["iris", "ocean", "forest", "sunset", "mono"];
const hueMap: Record<ThemePreset, string> = {
  iris: "hsl(252 60% 65%)", ocean: "hsl(210 60% 60%)",
  forest: "hsl(160 55% 50%)", sunset: "hsl(30 70% 55%)",
  mono: "hsl(220 0% 60%)",
};

function ThemeSwatch({ preset, label, selected, onClick }: { preset: ThemePreset; label: string; selected: boolean; onClick: () => void }) {
  return (
    <button type="button" onClick={onClick}
      className={`flex flex-col items-center gap-1.5 p-2 rounded-xl border-2 cursor-pointer transition-all duration-200 ${
        selected ? "border-primary bg-primary/8 shadow-sm ring-1 ring-primary/30" : "border-transparent hover:border-border hover:bg-secondary/40"
      }`}
    >
      <div className="w-7 h-7 rounded-full shadow-sm ring-1 ring-black/10" style={{ backgroundColor: hueMap[preset] }} />
      <span className="text-[10px] font-medium text-muted-foreground leading-tight text-center">{label}</span>
    </button>
  );
}

export default function AppearanceSettingsContent({
  theme, onPresetChange, onModeToggle, onGlassChange,
  onHueChange, onHueClear, onBgImageChange, onBlurChange,
}: Props) {
  const { t, i18n } = useTranslation();
  const lang = i18n.language.startsWith("en") ? "en" : "zh";
  const handleFilePick = () => {
    const input = document.createElement("input");
    input.type = "file"; input.accept = "image/*";
    input.onchange = () => {
      const file = input.files?.[0];
      if (!file) return;
      const reader = new FileReader();
      reader.onload = () => onBgImageChange(reader.result as string);
      reader.readAsDataURL(file);
    };
    input.click();
  };

  return (
    <div className="space-y-4 px-1">
      <Card><CardContent className="p-4 space-y-3">
        <div className="flex items-center gap-2">
          <Sparkles className="h-4 w-4 text-primary" />
          <span className="text-sm font-medium">{t("appearance.colorTheme")}</span>
          {theme.customHue !== null && (
            <Button variant="ghost" size="sm" className="h-5 text-[10px] px-1.5 ml-auto" onClick={onHueClear}>{t("appearance.resetPreset")}</Button>
          )}
        </div>
        <div className="grid grid-cols-5 gap-1">
          {themes.map((t) => (
            <ThemeSwatch key={t} preset={t} label={THEME_LABELS[t][lang as "zh" | "en"]}
              selected={theme.preset === t && theme.customHue === null} onClick={() => onPresetChange(t)} />
          ))}
        </div>
        <div className="flex items-center gap-3 pt-1">
          <div className="flex-1 space-y-1.5">
            <div className="relative h-5 rounded-md overflow-hidden"
              style={{ background: "linear-gradient(to right, hsl(0,70%,55%), hsl(120,70%,55%), hsl(240,70%,55%), hsl(360,70%,55%))" }}
            >
              <div className="absolute top-0 bottom-0 w-0.5 bg-white shadow-md"
                style={{ left: `${((theme.customHue ?? 252) / 360) * 100}%` }} />
            </div>
            <Slider value={[theme.customHue ?? 252]} onValueChange={([v]) => onHueChange(Math.round(v))} min={0} max={360} step={1} />
          </div>
          <NumberInput value={theme.customHue ?? 252} onChange={(v) => onHueChange(Math.max(0, Math.min(360, v)))} min={0} max={360} step={1} className="w-20" />
        </div>
      </CardContent></Card>

      <Card><CardContent className="p-4 space-y-4">
        <div className="flex items-center gap-2"><Eye className="h-4 w-4 text-primary" /><span className="text-sm font-medium">{t("appearance.bgAndGlass")}</span></div>
        <div className="flex items-center gap-3">
          <div className="flex-1 min-w-0">
            {theme.bgImage ? (
              <div className="relative rounded-lg overflow-hidden border border-border h-16 bg-secondary/30">
                <img src={theme.bgImage} alt="" className="w-full h-full object-cover" />
                <button type="button" onClick={() => onBgImageChange(null)}
                  className="absolute top-1 right-1 h-6 w-6 rounded-full bg-black/50 flex items-center justify-center hover:bg-black/70 transition-colors"
                ><Trash2 className="h-3 w-3 text-white" /></button>
              </div>
            ) : (
              <button type="button" onClick={handleFilePick}
                className="w-full h-16 rounded-lg border-2 border-dashed border-border hover:border-primary/50 flex items-center justify-center gap-2 transition-colors"
              ><Image className="h-4 w-4 text-muted-foreground" /><span className="text-xs text-muted-foreground">{t("appearance.selectImage")}</span></button>
            )}
          </div>
          <Label className="text-xs text-muted-foreground shrink-0 self-start mt-1">{t("appearance.bgLabel")}</Label>
        </div>
        <div className="space-y-2">
          <div className="flex items-center justify-between">
            <Label className="text-xs">{t("appearance.blurIntensity")}</Label>
            <span className="text-[11px] text-muted-foreground tabular-nums">{theme.blurIntensity.toFixed(1)}</span>
          </div>
          <div className="flex items-center gap-3">
            <Slider value={[theme.blurIntensity]} onValueChange={([v]) => onBlurChange(v)} min={0} max={10} step={0.5} className="flex-1" />
            <NumberInput value={theme.blurIntensity} onChange={onBlurChange} min={0} max={10} step={0.5} className="w-20" />
          </div>
        </div>
      </CardContent></Card>

      {/* Language */}
      <Card><CardContent className="p-4">
        <div className="flex items-center justify-between">
          <span className="text-sm font-medium">{t("appearance.language")}</span>
          <div className="flex items-center gap-1.5">
            <Button variant={lang === "zh" ? "default" : "outline"} size="sm" className="h-7 text-xs"
              onClick={() => i18n.changeLanguage("zh-CN")}>{t("lang.zh")}</Button>
            <Button variant={lang === "en" ? "default" : "outline"} size="sm" className="h-7 text-xs"
              onClick={() => i18n.changeLanguage("en-US")}>{t("lang.en")}</Button>
          </div>
        </div>
      </CardContent></Card>

      <Card><CardContent className="p-4 space-y-3">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            {theme.mode === "dark" ? <Moon className="h-4 w-4 text-primary" /> : <Sun className="h-4 w-4 text-amber-400" />}
            <div><span className="text-sm font-medium">{t("appearance.mode")}</span><p className="text-[11px] text-muted-foreground">{theme.mode === "dark" ? t("appearance.dark") : t("appearance.light")}</p></div>
          </div>
          <Button variant="outline" size="sm" className="gap-1.5 text-xs" onClick={onModeToggle}>
            {theme.mode === "dark" ? <><Sun className="h-3.5 w-3.5" /> {t("appearance.switchLight")}</> : <><Moon className="h-3.5 w-3.5" /> {t("appearance.switchDark")}</>}
          </Button>
        </div>
        <div className="flex items-center justify-between pt-1 border-t border-border">
          <div className="flex items-center gap-2">
            <div className="h-4 w-4 rounded-full bg-primary/30 backdrop-blur border border-primary/50" />
            <div><span className="text-sm font-medium">{t("appearance.glass")}</span><p className="text-[11px] text-muted-foreground">{t("appearance.glassDesc")}</p></div>
          </div>
          <Switch checked={theme.glassEnabled} onCheckedChange={onGlassChange} />
        </div>
      </CardContent></Card>
    </div>
  );
}
