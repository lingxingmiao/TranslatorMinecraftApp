import { useTranslation } from "react-i18next";
import type { AppSettings } from "@/lib/types";
import { Label } from "@/components/ui/label";
import { Input } from "@/components/ui/input";

interface Props {
  apiUrl: string;
  apiKey: string;
  savePath: string;
  onChange: <K extends keyof AppSettings>(key: K, value: AppSettings[K]) => void;
}

export default function LocalSettingsContent({ apiUrl, apiKey, savePath, onChange }: Props) {
  const { t } = useTranslation();
  return (
    <div className="space-y-4">
      <div className="space-y-1.5"><Label className="text-xs">{t("settings.apiUrl")}</Label>
        <Input value={apiUrl} onChange={(e) => onChange("apiUrl", e.target.value as any)} placeholder="http://127.0.0.1:8000" /></div>
      <div className="space-y-1.5"><Label className="text-xs">{t("settings.apiKey")}</Label>
        <Input type="password" value={apiKey} onChange={(e) => onChange("apiKey", e.target.value as any)} placeholder={t("optional")} /></div>
      <div className="space-y-1.5"><Label className="text-xs">{t("settings.savePath")}</Label>
        <Input value={savePath} onChange={(e) => onChange("savePath", e.target.value as any)} placeholder="outputs" /></div>
    </div>
  );
}
