import { useTranslation } from "react-i18next";
import type { TaskInfo } from "@/lib/types";
import { Badge } from "@/components/ui/badge";
import { Progress } from "@/components/ui/progress";
import { Button } from "@/components/ui/button";
import { Separator } from "@/components/ui/separator";
import LogViewer from "@/components/logs/LogViewer";
import { Tooltip, TooltipContent, TooltipTrigger } from "@/components/ui/tooltip";
import { Download, XCircle } from "lucide-react";
import { cn } from "@/lib/utils";
import { motion, AnimatePresence } from "framer-motion";

interface TaskDetailProps {
  task: TaskInfo | null;
  autoScroll: boolean;
  onScrollChange?: (isNearBottom: boolean) => void;
  onCancel: () => void;
  onDownload: () => void;
}

const statusColorMap: Record<string, string> = {
  queued: "bg-amber-500/10 text-amber-400 border-amber-500/20",
  processing: "bg-sky-500/10 text-sky-400 border-sky-500/20",
  completed: "bg-emerald-500/10 text-emerald-400 border-emerald-500/20",
  failed: "bg-red-500/10 text-red-400 border-red-500/20",
  cancelled: "bg-gray-500/10 text-gray-400 border-gray-500/20",
};

const detailVariants = {
  initial: { opacity: 0, x: 20 },
  animate: { opacity: 1, x: 0 },
  exit: { opacity: 0, x: -20 },
};

export default function TaskDetail({ task, autoScroll, onScrollChange, onCancel, onDownload }: TaskDetailProps) {
  const { t } = useTranslation();

  if (!task) {
    return (
      <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }}
        className="flex items-center justify-center h-full text-muted-foreground"
      >
        <p className="text-sm">{t("app.noTaskSelected")}</p>
      </motion.div>
    );
  }

  const isActive = task.status === "processing" || task.status === "queued";
  const isCompleted = task.status === "completed";
  const isFailed = task.status === "failed";

  return (
    <AnimatePresence mode="wait">
      <motion.div key={task.taskId} variants={detailVariants} initial="initial" animate="animate" exit="exit"
        transition={{ duration: 0.2, ease: "easeOut" }} className="flex flex-col h-full"
      >
        <div className="p-4 space-y-3">
          <div className="flex items-start justify-between">
            <div>
              <h2 className="text-base font-semibold">
                {task.displayName || task.filename || task.taskId.slice(0, 8)}
              </h2>
              <div className="flex items-center gap-2 mt-1">
                <span className="text-xs text-muted-foreground">
                  {t(`task.${task.typeName}`)}
                </span>
                <span className={cn("inline-flex items-center px-2 py-0.5 rounded text-[11px] font-medium border", statusColorMap[task.status] || "")}>
                  {t(`task.status${task.status.charAt(0).toUpperCase()}${task.status.slice(1)}`)}
                </span>
              </div>
            </div>

            <div className="flex items-center gap-1">
              {isActive && (
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button variant="ghost" size="icon" className="text-muted-foreground hover:text-destructive" onClick={onCancel}>
                      <XCircle className="h-4 w-4" />
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>{t("task.cancel")}</TooltipContent>
                </Tooltip>
              )}
              {isCompleted && (
                <Tooltip>
                  <TooltipTrigger asChild>
                    <Button variant="ghost" size="icon" className="text-muted-foreground hover:text-primary" onClick={onDownload}>
                      <Download className="h-4 w-4" />
                    </Button>
                  </TooltipTrigger>
                  <TooltipContent>{t("task.download")}</TooltipContent>
                </Tooltip>
              )}
            </div>
          </div>

          <AnimatePresence>
            {task.status === "processing" && (
              <motion.div initial={{ opacity: 0, height: 0 }} animate={{ opacity: 1, height: "auto" }}
                exit={{ opacity: 0, height: 0 }} className="space-y-1 overflow-hidden"
              >
                <div className="flex justify-between text-xs text-muted-foreground">
                  <span>{t("task.progress")}</span>
                  <span className="tabular-nums">{task.progress}%</span>
                </div>
                <Progress value={task.progress} className="h-1.5" />
              </motion.div>
            )}
          </AnimatePresence>

          <AnimatePresence>
            {isFailed && task.logs.length > 0 && (
              <motion.div initial={{ opacity: 0, y: -8 }} animate={{ opacity: 1, y: 0 }} exit={{ opacity: 0, y: -8 }}
                className="text-xs text-destructive bg-destructive/5 p-2 rounded border border-destructive/20"
              >
                {t("task.errorHint")}
              </motion.div>
            )}
          </AnimatePresence>
        </div>

        <Separator />

        <div className="flex-1 min-h-0">
          <div className="px-4 py-2 text-xs font-medium text-muted-foreground bg-secondary/20 border-b border-border">
            {t("task.logs")}
          </div>
          <LogViewer logs={task.logs} maxLines={1000} autoScroll={autoScroll} onScrollChange={onScrollChange} />
        </div>
      </motion.div>
    </AnimatePresence>
  );
}
