import { useTranslation } from "react-i18next";
import type { AppSettings } from "@/lib/types";
import { Label } from "@/components/ui/label";
import { Switch } from "@/components/ui/switch";
import { NumberInput } from "@/components/ui/number-input";

interface Props {
  maxLogLines: number;
  pollInterval: number;
  autoScroll: boolean;
  onChange: <K extends keyof AppSettings>(key: K, value: AppSettings[K]) => void;
}

export default function AdvancedSettingsContent({ maxLogLines, pollInterval, autoScroll, onChange }: Props) {
  const { t } = useTranslation();
  return (
    <div className="space-y-4 px-1">
      <div className="grid grid-cols-2 gap-3">
        <div className="space-y-1.5"><Label className="text-xs">{t("advanced.maxLogLines")}</Label>
          <NumberInput value={maxLogLines} onChange={(v) => onChange("maxLogLines", v as any)} min={100} max={10000} step={100} /></div>
        <div className="space-y-1.5"><Label className="text-xs">{t("advanced.pollInterval")}</Label>
          <NumberInput value={pollInterval} onChange={(v) => onChange("pollInterval", v as any)} min={500} max={30000} step={500} /></div>
      </div>
      <div className="flex items-center justify-between">
        <Label className="text-xs cursor-pointer">{t("advanced.autoScroll")}</Label>
        <Switch checked={autoScroll} onCheckedChange={(c) => onChange("autoScroll", c as any)} />
      </div>
    </div>
  );
}
