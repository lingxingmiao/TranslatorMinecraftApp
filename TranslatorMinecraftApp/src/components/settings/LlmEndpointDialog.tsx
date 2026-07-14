import { useState } from "react";
import { useTranslation } from "react-i18next";
import type { LlmEndpointConfig } from "@/lib/types";
import { Dialog, DialogContent, DialogHeader, DialogTitle, DialogFooter } from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";
import { Slider } from "@/components/ui/slider";
import { Switch } from "@/components/ui/switch";
import { ScrollArea } from "@/components/ui/scroll-area";
import { Separator } from "@/components/ui/separator";

interface LlmEndpointDialogProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  endpoint: LlmEndpointConfig | null;
  onSave: (config: LlmEndpointConfig) => void;
}

const defaultEndpoint: () => LlmEndpointConfig = () => ({
  id: crypto.randomUUID(), name: "", url: "", key: "", model: "",
  temperature: 0.7, topP: 0.9, topK: 40, presencePenalty: 0, frequencyPenalty: 0,
  seed: -1, maxRetries: 3, retryDelay: 1000, maxConcurrency: 1, weight: 1,
  activeHours: "0-24", enabled: true,
});

export default function LlmEndpointDialog({ open, onOpenChange, endpoint: initialEndpoint, onSave }: LlmEndpointDialogProps) {
  const { t } = useTranslation();
  const [config, setConfig] = useState<LlmEndpointConfig>(initialEndpoint || defaultEndpoint());
  const update = <K extends keyof LlmEndpointConfig>(key: K, value: LlmEndpointConfig[K]) => setConfig((prev) => ({ ...prev, [key]: value }));

  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="sm:max-w-2xl max-h-[85vh]">
        <DialogHeader><DialogTitle>{initialEndpoint ? t("llm.editTitle") : t("llm.newTitle")}</DialogTitle></DialogHeader>
        <ScrollArea className="max-h-[60vh] pr-4">
          <div className="space-y-6">
            <Card><CardHeader className="px-4 py-3"><CardTitle className="text-sm font-medium text-muted-foreground">{t("llm.basicInfo")}</CardTitle></CardHeader>
              <CardContent className="px-4 pb-4 pt-0">
                <div className="grid grid-cols-2 gap-3">
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.endpointName")}</Label>
                    <Input value={config.name} onChange={(e) => update("name", e.target.value)} placeholder={t("llm.endpointNamePlaceholder")} /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.endpointEnabled")}</Label>
                    <div className="flex items-center h-9"><Switch checked={config.enabled} onCheckedChange={(c) => update("enabled", c)} /></div></div>
                </div>
              </CardContent>
            </Card>
            <Separator />
            <Card><CardHeader className="px-4 py-3"><CardTitle className="text-sm font-medium text-muted-foreground">{t("llm.connection")}</CardTitle></CardHeader>
              <CardContent className="px-4 pb-4 pt-0">
                <div className="space-y-1.5"><Label className="text-xs">{t("llm.apiUrl")}</Label>
                  <Input value={config.url} onChange={(e) => update("url", e.target.value)} placeholder="https://api.openai.com/v1" /></div>
                <div className="grid grid-cols-2 gap-3 mt-3">
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.apiKey")}</Label>
                    <Input type="password" value={config.key} onChange={(e) => update("key", e.target.value)} placeholder="sk-..." /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.model")}</Label>
                    <Input value={config.model} onChange={(e) => update("model", e.target.value)} placeholder="gpt-4o" /></div>
                </div>
              </CardContent>
            </Card>
            <Separator />
            <Card><CardHeader className="px-4 py-3"><CardTitle className="text-sm font-medium text-muted-foreground">{t("llm.params")}</CardTitle></CardHeader>
              <CardContent className="px-4 pb-4 pt-0">
                <div className="grid grid-cols-2 gap-4">
                  <div className="space-y-2"><Label className="text-xs">{t("llm.presencePenalty", { val: config.presencePenalty.toFixed(2) })}</Label>
                    <Slider value={[config.presencePenalty]} onValueChange={([v]) => update("presencePenalty", v)} min={-2} max={2} step={0.01} /></div>
                  <div className="space-y-2"><Label className="text-xs">{t("llm.frequencyPenalty", { val: config.frequencyPenalty.toFixed(2) })}</Label>
                    <Slider value={[config.frequencyPenalty]} onValueChange={([v]) => update("frequencyPenalty", v)} min={-2} max={2} step={0.01} /></div>
                </div>
              </CardContent>
            </Card>
            <Separator />
            <Card><CardHeader className="px-4 py-3"><CardTitle className="text-sm font-medium text-muted-foreground">{t("llm.advanced")}</CardTitle></CardHeader>
              <CardContent className="px-4 pb-4 pt-0">
                <div className="grid grid-cols-2 gap-3">
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.maxRetries")}</Label>
                    <Input type="number" value={config.maxRetries} onChange={(e) => update("maxRetries", Number(e.target.value))} /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.retryDelay")}</Label>
                    <Input type="number" value={config.retryDelay} onChange={(e) => update("retryDelay", Number(e.target.value))} /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.maxConcurrency")}</Label>
                    <Input type="number" value={config.maxConcurrency} onChange={(e) => update("maxConcurrency", Number(e.target.value))} /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.weight")}</Label>
                    <Input type="number" value={config.weight} onChange={(e) => update("weight", Number(e.target.value))} /></div>
                  <div className="space-y-1.5"><Label className="text-xs">{t("llm.activeHours")}</Label>
                    <Input value={config.activeHours} onChange={(e) => update("activeHours", e.target.value)} placeholder="0-24" /></div>
                </div>
              </CardContent>
            </Card>
          </div>
        </ScrollArea>
        <DialogFooter>
          <Button variant="outline" onClick={() => onOpenChange(false)}>{t("llm.cancel")}</Button>
          <Button onClick={() => onSave(config)}>{t("llm.save")}</Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
}
