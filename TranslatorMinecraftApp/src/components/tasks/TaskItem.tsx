import { useTranslation } from "react-i18next";
import { type TaskInfo, type TaskStatus } from "@/lib/types";
import { Badge } from "@/components/ui/badge";
import { cn } from "@/lib/utils";
import { ContextMenu, ContextMenuContent, ContextMenuItem, ContextMenuTrigger } from "@/components/ui/context-menu";
import { motion } from "framer-motion";

interface TaskItemProps {
  task: TaskInfo;
  isSelected: boolean;
  onSelect: () => void;
  onRename: () => void;
  onDelete: () => void;
  index?: number;
}

const statusConfig: Record<TaskStatus, { variant: "info" | "warning" | "success" | "destructive" | "secondary"; icon: string }> = {
  queued: { variant: "secondary", icon: "⌛" },
  processing: { variant: "info", icon: "⏳" },
  completed: { variant: "success", icon: "✓" },
  failed: { variant: "destructive", icon: "✗" },
  cancelled: { variant: "secondary", icon: "⊘" },
};

export default function TaskItem({ task, isSelected, onSelect, onRename, onDelete, index = 0 }: TaskItemProps) {
  const { t } = useTranslation();
  const cfg = statusConfig[task.status];

  return (
    <motion.div initial={{ opacity: 0, x: -12 }} animate={{ opacity: 1, x: 0 }}
      transition={{ duration: 0.25, delay: index * 0.04, ease: "easeOut" }} layout
    >
      <ContextMenu>
        <ContextMenuTrigger>
          <div onClick={onSelect}
            className={cn("flex items-center gap-3 px-3 py-2.5 cursor-pointer rounded-md transition-all duration-150",
              "hover:bg-secondary/60 border border-transparent",
              isSelected ? "bg-secondary border-primary/20 shadow-sm" : "bg-transparent"
            )}
          >
            <span className={cn("text-base w-5 text-center shrink-0", task.status === "processing" && "animate-pulse")}>
              {cfg.icon}
            </span>
            <div className="flex-1 min-w-0">
              <div className="text-sm font-medium truncate">
                {task.displayName || task.filename || task.taskId.slice(0, 8)}
              </div>
              <div className="flex items-center gap-2 mt-0.5">
                <span className="text-[11px] text-muted-foreground">{t(`task.${task.typeName}`)}</span>
                <Badge variant={cfg.variant} className="text-[10px] h-4 px-1.5">{t(`task.status${task.status.charAt(0).toUpperCase()}${task.status.slice(1)}`)}</Badge>
              </div>
            </div>
            {task.status === "processing" && (
              <div className="text-xs text-muted-foreground shrink-0 tabular-nums">{task.progress}%</div>
            )}
          </div>
        </ContextMenuTrigger>
        <ContextMenuContent className="w-48">
          <ContextMenuItem onSelect={onRename}>{t("task.rename")}</ContextMenuItem>
          <ContextMenuItem onSelect={onDelete} className="text-destructive focus:text-destructive">{t("task.delete")}</ContextMenuItem>
        </ContextMenuContent>
      </ContextMenu>
    </motion.div>
  );
}
