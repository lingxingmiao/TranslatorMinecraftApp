import { useTranslation } from "react-i18next";
import type { TaskInfo } from "@/lib/types";
import { ScrollArea } from "@/components/ui/scroll-area";
import TaskItem from "./TaskItem";
import { AnimatePresence } from "framer-motion";

interface TaskListProps {
  tasks: TaskInfo[];
  selectedTaskId: string | null;
  onSelectTask: (taskId: string) => void;
  onRenameTask: (taskId: string) => void;
  onDeleteTask: (taskId: string) => void;
}

export default function TaskList({ tasks, selectedTaskId, onSelectTask, onRenameTask, onDeleteTask }: TaskListProps) {
  const { t } = useTranslation();

  if (tasks.length === 0) {
    return (
      <div className="flex flex-col items-center justify-center h-full text-muted-foreground">
        <p className="text-xs">{t("app.noTasks")}</p>
        <p className="text-[11px] mt-1">{t("app.noTasksHint")}</p>
      </div>
    );
  }

  return (
    <ScrollArea className="h-full px-2 py-2">
      <div className="space-y-1">
        <AnimatePresence mode="popLayout">
          {tasks.map((task, i) => (
            <TaskItem key={task.taskId} task={task} index={i}
              isSelected={selectedTaskId === task.taskId}
              onSelect={() => onSelectTask(task.taskId)}
              onRename={() => onRenameTask(task.taskId)}
              onDelete={() => onDeleteTask(task.taskId)}
            />
          ))}
        </AnimatePresence>
      </div>
    </ScrollArea>
  );
}
