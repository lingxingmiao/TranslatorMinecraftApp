import { useTranslation } from "react-i18next";
import type { LlmEndpointConfig } from "@/lib/types";
import { Button } from "@/components/ui/button";
import { Card, CardContent } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Plus, Pencil, Trash2 } from "lucide-react";
import { motion, AnimatePresence } from "framer-motion";

interface Props {
  endpoints: LlmEndpointConfig[];
  onAdd: () => void;
  onEdit: (ep: LlmEndpointConfig) => void;
  onDelete: (id: string) => void;
}

const itemVariants = {
  initial: { opacity: 0, y: -8, scale: 0.98 },
  animate: { opacity: 1, y: 0, scale: 1 },
  exit: { opacity: 0, scale: 0.95, transition: { duration: 0.15 } },
};

export default function EndpointSettingsContent({ endpoints, onAdd, onEdit, onDelete }: Props) {
  const { t } = useTranslation();
  return (
    <div className="space-y-4">
      <div className="flex items-center justify-between">
        <span className="text-sm text-muted-foreground">{t("settings.endpointCount", { count: endpoints.length })}</span>
        <Button size="sm" onClick={onAdd}><Plus className="h-4 w-4 mr-1" />{t("settings.endpointNew")}</Button>
      </div>
      <AnimatePresence mode="popLayout">
        {endpoints.length === 0 ? (
          <motion.div key="empty" initial={{ opacity: 0 }} animate={{ opacity: 1 }}
            className="text-center py-8 text-sm text-muted-foreground">{t("settings.endpointEmpty")}</motion.div>
        ) : (
          <div className="space-y-2">
            {endpoints.map((ep) => (
              <motion.div key={ep.id} layout variants={itemVariants} initial="initial" animate="animate" exit="exit" transition={{ duration: 0.2 }}>
                <Card className="bg-secondary/30 border-border">
                  <CardContent className="flex items-center justify-between p-3">
                    <div className="flex-1 min-w-0">
                      <div className="flex items-center gap-2">
                        <span className="text-sm font-medium">{ep.name || t("settings.endpointUnnamed")}</span>
                        <Badge variant={ep.enabled ? "success" : "secondary"}>{ep.enabled ? t("settings.endpointEnabled") : t("settings.endpointDisabled")}</Badge>
                      </div>
                      <div className="text-xs text-muted-foreground mt-0.5 truncate">{ep.url} | {ep.model || t("settings.endpointNoModel")}</div>
                    </div>
                    <div className="flex items-center gap-1 shrink-0 ml-2">
                      <Button variant="ghost" size="icon" className="h-8 w-8" onClick={() => onEdit(ep)}><Pencil className="h-3.5 w-3.5" /></Button>
                      <Button variant="ghost" size="icon" className="h-8 w-8 text-destructive hover:text-destructive" onClick={() => onDelete(ep.id)}><Trash2 className="h-3.5 w-3.5" /></Button>
                    </div>
                  </CardContent>
                </Card>
              </motion.div>
            ))}
          </div>
        )}
      </AnimatePresence>
    </div>
  );
}
