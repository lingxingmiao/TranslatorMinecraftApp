import { useState, useEffect, useRef } from "react";
import { useTranslation } from "react-i18next";
import type { AppSettings, LlmEndpointConfig } from "@/lib/types";
import type { ThemeConfig, ThemePreset } from "@/lib/theme";
import { createThemeManager } from "@/lib/theme-manager";
import { Dialog, DialogContent, DialogHeader, DialogTitle } from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { ScrollArea } from "@/components/ui/scroll-area";
import { Separator } from "@/components/ui/separator";
import { motion } from "framer-motion";
import LocalSettingsContent from "./LocalSettingsContent";
import EndpointSettingsContent from "./EndpointSettingsContent";
import AppearanceSettingsContent from "./AppearanceSettingsContent";
import AdvancedSettingsContent from "./AdvancedSettingsContent";
import LlmEndpointDialog from "./LlmEndpointDialog";

interface SettingsDialogProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  settings: AppSettings;
  onSave: (settings: AppSettings) => void;
}

const defaultSettings: AppSettings = {
  apiUrl: "http://127.0.0.1:8000",
  apiKey: "",
  savePath: "outputs",
  maxLogLines: 1000,
  pollInterval: 2000,
  autoScroll: true,
  endpoints: [],
  theme: { preset: "forest", mode: "dark", glassEnabled: true, customHue: null, bgImage: null, blurIntensity: 5 },
};

const tabVariants = {
  initial: { opacity: 0, x: 12 },
  animate: { opacity: 1, x: 0 },
  exit: { opacity: 0, x: -12 },
};

function TabPanel({ value, children }: { value: string; children: React.ReactNode }) {
  return (
    <TabsContent value={value} className="mt-4 space-y-4">
      <motion.div key={value} variants={tabVariants} initial="initial" animate="animate" exit="exit"
        transition={{ duration: 0.2, ease: "easeOut" }}>{children}</motion.div>
    </TabsContent>
  );
}

const mgr = createThemeManager();

export default function SettingsDialog({ open, onOpenChange, settings, onSave }: SettingsDialogProps) {
  const { t } = useTranslation();
  const [localSettings, setLocalSettings] = useState<AppSettings>(defaultSettings);
  const [endpointDialogOpen, setEndpointDialogOpen] = useState(false);
  const [editingEndpoint, setEditingEndpoint] = useState<LlmEndpointConfig | null>(null);
  const [dirty, setDirty] = useState(false);
  const snapshotRef = useRef<ThemeConfig | null>(null);
  const skipReinit = useRef(false);

  useEffect(() => {
    if (open) {
      snapshotRef.current = mgr.get();
      if (!skipReinit.current) {
        setLocalSettings({ ...defaultSettings, ...settings });
        setDirty(false);
      }
      skipReinit.current = false;
    }
  }, [open, settings]);

  const update = <K extends keyof AppSettings>(key: K, value: AppSettings[K]) => {
    setLocalSettings((prev) => ({ ...prev, [key]: value }));
    setDirty(true);
  };

  const theme = localSettings.theme;
  const doThemePreview = (updated: ThemeConfig) => {
    mgr.preview(updated);
    setLocalSettings((prev) => ({ ...prev, theme: updated }));
    setDirty(true);
  };
  const previewPreset = (preset: ThemePreset) => doThemePreview({ ...theme, preset, customHue: null });
  const previewToggleMode = () => doThemePreview({ ...theme, mode: theme.mode === "dark" ? "light" : "dark" });
  const previewGlass = (enabled: boolean) => doThemePreview({ ...theme, glassEnabled: enabled });
  const previewHue = (hue: number) => doThemePreview({ ...theme, customHue: hue });
  const previewClearHue = () => doThemePreview({ ...theme, customHue: null });

  const openEndpointDialog = (ep: LlmEndpointConfig | null) => {
    setEditingEndpoint(ep);
    setEndpointDialogOpen(true);
    onOpenChange(false);
  };

  const handleEndpointDialogChange = (v: boolean) => {
    setEndpointDialogOpen(v);
    if (!v) { skipReinit.current = true; onOpenChange(true); }
  };

  const handleSaveEndpoint = (config: LlmEndpointConfig) => {
    setLocalSettings((prev) => {
      const existing = prev.endpoints.findIndex((e) => e.id === config.id);
      const updatedEndpoints = existing >= 0
        ? prev.endpoints.map((e, i) => (i === existing ? config : e))
        : [...prev.endpoints, config];
      return { ...prev, endpoints: updatedEndpoints };
    });
    setDirty(true);
    setEndpointDialogOpen(false);
    setEditingEndpoint(null);
  };

  const handleDeleteEndpoint = (id: string) => {
    setLocalSettings((prev) => ({ ...prev, endpoints: prev.endpoints.filter((e) => e.id !== id) }));
    setDirty(true);
  };

  const handleSave = () => {
    mgr.set(localSettings.theme);
    onSave(localSettings);
    onOpenChange(false);
  };

  const handleClose = (v: boolean) => {
    if (!v) {
      if (snapshotRef.current) mgr.restore(snapshotRef.current);
      onOpenChange(false);
    }
  };

  return (
    <>
      <Dialog open={open} onOpenChange={handleClose}>
        <DialogContent className="sm:max-w-3xl max-h-[85vh]">
          <DialogHeader><DialogTitle>{t("settings.title")}</DialogTitle></DialogHeader>
          <Tabs defaultValue="local" className="flex-1">
            <TabsList className="grid w-full grid-cols-4">
              <TabsTrigger value="local">{t("settings.tab.local")}</TabsTrigger>
              <TabsTrigger value="endpoints">{t("settings.tab.llm")}</TabsTrigger>
              <TabsTrigger value="appearance">{t("settings.tab.appearance")}</TabsTrigger>
              <TabsTrigger value="advanced">{t("settings.tab.advanced")}</TabsTrigger>
            </TabsList>
            <ScrollArea className="flex-1 overflow-y-auto max-h-[55vh]" style={{ scrollbarGutter: "stable" }}>
              <TabPanel value="local">
                <LocalSettingsContent apiUrl={localSettings.apiUrl} apiKey={localSettings.apiKey}
                  savePath={localSettings.savePath} onChange={update} />
              </TabPanel>
              <TabPanel value="endpoints">
                <EndpointSettingsContent endpoints={localSettings.endpoints}
                  onAdd={() => openEndpointDialog(null)} onEdit={(ep) => openEndpointDialog(ep)}
                  onDelete={handleDeleteEndpoint} />
              </TabPanel>
              <TabPanel value="appearance">
                <AppearanceSettingsContent theme={localSettings.theme}
                  onPresetChange={previewPreset} onModeToggle={previewToggleMode}
                  onGlassChange={previewGlass} onHueChange={previewHue} onHueClear={previewClearHue}
                  onBgImageChange={(dataUrl) => doThemePreview({ ...theme, bgImage: dataUrl })}
                  onBlurChange={(v) => doThemePreview({ ...theme, blurIntensity: v })} />
              </TabPanel>
              <TabPanel value="advanced">
                <AdvancedSettingsContent maxLogLines={localSettings.maxLogLines}
                  pollInterval={localSettings.pollInterval} autoScroll={localSettings.autoScroll} onChange={update} />
              </TabPanel>
            </ScrollArea>
          </Tabs>
          <Separator />
          <div className="flex items-center justify-end gap-2 pt-2">
            <Button variant="outline" onClick={() => { if (snapshotRef.current) mgr.restore(snapshotRef.current); onOpenChange(false); }}>
              {t("settings.cancel")}
            </Button>
            <Button onClick={handleSave} disabled={!dirty}>{t("settings.save")}</Button>
          </div>
        </DialogContent>
      </Dialog>
      <LlmEndpointDialog open={endpointDialogOpen} onOpenChange={handleEndpointDialogChange}
        endpoint={editingEndpoint} onSave={handleSaveEndpoint} />
    </>
  );
}
