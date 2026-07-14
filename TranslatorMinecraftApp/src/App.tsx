import { useState, useCallback, useEffect } from "react";
import { useTranslation } from "react-i18next";
import type { TaskInfo, TaskType, AppSettings } from "@/lib/types";
import { DEFAULT_SAVE_PATH, DEFAULT_MAX_LOG_LINES, DEFAULT_POLL_INTERVAL } from "@/lib/constants";
import { submitTranslateTask, submitSeparateTask, submitMergeTask, getTaskStatus, getTaskLogs, cancelTask, downloadTaskResult } from "@/lib/api";
import { usePolling } from "@/hooks/useHooks";
import { useTheme } from "@/hooks/useTheme";
import { useNativeGlass } from "@/hooks/useNativeGlass";
import { BackgroundEffect } from "@/components/layout/BackgroundEffect";
import { TooltipProvider } from "@/components/ui/tooltip";
import TitleBar from "@/components/layout/TitleBar";
import TaskList from "@/components/tasks/TaskList";
import TaskDetail from "@/components/tasks/TaskDetail";
import NewTaskDialog from "@/components/tasks/NewTaskDialog";
import SettingsDialog from "@/components/settings/SettingsDialog";
import { Button } from "@/components/ui/button";
import { DropdownMenu, DropdownMenuContent, DropdownMenuItem, DropdownMenuTrigger } from "@/components/ui/dropdown-menu";
import { Plus, Settings, FileText, FileInput, FileOutput } from "lucide-react";
import { motion } from "framer-motion";

const getDefaultSettings = (theme: ReturnType<typeof useTheme>["theme"]): AppSettings => ({
  apiUrl: "http://127.0.0.1:8000",
  apiKey: "", savePath: DEFAULT_SAVE_PATH,
  maxLogLines: DEFAULT_MAX_LOG_LINES, pollInterval: DEFAULT_POLL_INTERVAL,
  autoScroll: true, endpoints: [], theme,
});

function App() {
  const { t } = useTranslation();
  const themeCtx = useTheme();
  const { theme } = themeCtx;
  useNativeGlass();

  const [tasks, setTasks] = useState<TaskInfo[]>([]);
  const [selectedTaskId, setSelectedTaskId] = useState<string | null>(null);
  const [settings, setSettings] = useState<AppSettings>(() => getDefaultSettings(theme));
  const [newTaskDialogOpen, setNewTaskDialogOpen] = useState(false);
  const [settingsDialogOpen, setSettingsDialogOpen] = useState(false);
  const [autoScroll, setAutoScroll] = useState(true);

  const selectedTask = tasks.find((t) => t.taskId === selectedTaskId) || null;

  // Disable right-click context menu
  useEffect(() => {
    const handler = (e: MouseEvent) => e.preventDefault();
    document.addEventListener("contextmenu", handler);
    return () => document.removeEventListener("contextmenu", handler);
  }, []);

  useEffect(() => {
    try {
      const savedTasks = localStorage.getItem("tasks");
      if (savedTasks) setTasks(JSON.parse(savedTasks));
      const savedSettings = localStorage.getItem("settings");
      if (savedSettings) {
        const parsed = JSON.parse(savedSettings);
        setSettings({ ...getDefaultSettings(theme), ...parsed });
        setAutoScroll(parsed.autoScroll ?? true);
      }
    } catch { /* ignore */ }
  }, []);

  useEffect(() => { localStorage.setItem("tasks", JSON.stringify(tasks)); }, [tasks]);
  useEffect(() => { localStorage.setItem("settings", JSON.stringify(settings)); }, [settings]);

  const pollActiveTasks = useCallback(async () => {
    const activeTasks = tasks.filter((t) => t.status === "processing" || t.status === "queued");
    if (activeTasks.length === 0) return;
    const updatedTasks = [...tasks];
    for (const task of activeTasks) {
      try {
        const statusRes = await getTaskStatus(task.taskId);
        const idx = updatedTasks.findIndex((t) => t.taskId === task.taskId);
        if (idx === -1) continue;
        updatedTasks[idx] = { ...updatedTasks[idx], status: statusRes.status, progress: statusRes.progress ?? 0, filename: statusRes.filename || updatedTasks[idx].filename, downloadUrl: statusRes.downloadUrl || updatedTasks[idx].downloadUrl };
        try { const logRes = await getTaskLogs(task.taskId); updatedTasks[idx].logs = logRes.logs || []; } catch { /* ignore */ }
        if (statusRes.status === "completed" && statusRes.filename) { try { await downloadTaskResult(task.taskId, statusRes.filename); } catch { /* ignore */ } }
      } catch { /* ignore */ }
    }
    setTasks(updatedTasks);
  }, [tasks]);

  usePolling(pollActiveTasks, settings.pollInterval, tasks.some((t) => t.status === "processing" || t.status === "queued"));

  const handleTaskCreate = useCallback(async (type: TaskType, files: Record<string, File | null>, options: Record<string, boolean>) => {
    const { file0, file1 } = files;
    if (!file0) return;
    try {
      let res;
      switch (type) {
        case "translate": res = await submitTranslateTask(file0, file1 || null, options.allMode ?? false, options.exportInspection ?? false); break;
        case "separate": res = await submitSeparateTask(file0, file1!); break;
        case "merge": res = await submitMergeTask(file0, files.notlang!, file1!); break;
      }
      const newTask: TaskInfo = { taskId: res.taskId, displayName: file0.name, typeName: type, status: res.status, progress: 0, logs: [], filename: file0.name, createdAt: new Date().toISOString() };
      setTasks((prev) => [newTask, ...prev]);
      setSelectedTaskId(newTask.taskId);
    } catch (error) {
      setTasks((prev) => [{ taskId: `error-${Date.now()}`, displayName: file0.name, typeName: type, status: "failed", progress: 0, logs: [{ level: "ERROR", message: error instanceof Error ? error.message : t("task.createFailed") }], createdAt: new Date().toISOString() }, ...prev]);
    }
  }, [t]);

  const handleCancelTask = useCallback(async () => {
    if (!selectedTaskId) return;
    try { await cancelTask(selectedTaskId); } catch { /* ignore */ }
    setTasks((prev) => prev.map((t) => t.taskId === selectedTaskId ? { ...t, status: "cancelled" as const } : t));
  }, [selectedTaskId]);

  const handleCancelAllTasks = useCallback(async () => {
    const activeTasks = tasks.filter((t) => t.status === "processing" || t.status === "queued");
    await Promise.allSettled(activeTasks.map((t) => cancelTask(t.taskId)));
    setTasks((prev) => prev.map((t) => t.status === "processing" || t.status === "queued" ? { ...t, status: "cancelled" as const } : t));
  }, [tasks]);

  const handleDownloadResult = useCallback(async () => {
    if (!selectedTask?.filename) return;
    try { await downloadTaskResult(selectedTask.taskId, selectedTask.filename); } catch { /* ignore */ }
  }, [selectedTask]);

  const handleRenameTask = useCallback((taskId: string) => {
    const newName = prompt(t("task.renamePrompt"));
    if (newName) setTasks((prev) => prev.map((t) => t.taskId === taskId ? { ...t, displayName: newName } : t));
  }, [t]);

  const handleDeleteTask = useCallback((taskId: string) => {
    setTasks((prev) => prev.filter((t) => t.taskId !== taskId));
    if (selectedTaskId === taskId) setSelectedTaskId(null);
  }, [selectedTaskId]);

  const handleSaveSettings = useCallback((newSettings: AppSettings) => {
    setSettings(newSettings);
    setAutoScroll(newSettings.autoScroll);
    setSettingsDialogOpen(false);
  }, []);

  return (
    <>
      <BackgroundEffect glassEnabled={theme.glassEnabled} bgImage={settings.theme.bgImage} blurIntensity={settings.theme.blurIntensity} />
      <TooltipProvider delayDuration={300}>
        <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} transition={{ duration: 0.3 }}
          className="flex flex-col h-screen bg-background text-foreground overflow-hidden relative z-10"
        >
          <TitleBar title={t("app.title")} />
          <motion.div initial={{ opacity: 0, y: -8 }} animate={{ opacity: 1, y: 0 }} transition={{ duration: 0.25, delay: 0.05 }}
            className="flex items-center justify-between px-3 py-1.5 bg-sidebar-background"
          >
            <div className="flex items-center gap-1.5">
              <DropdownMenu>
                <DropdownMenuTrigger asChild>
                  <Button size="sm" className="h-7 text-xs gap-1"><Plus className="h-3.5 w-3.5" />{t("app.newTask")}</Button>
                </DropdownMenuTrigger>
                <DropdownMenuContent align="start">
                  <DropdownMenuItem onSelect={() => setNewTaskDialogOpen(true)}><FileText className="h-4 w-4 text-primary" />{t("task.translate")}</DropdownMenuItem>
                  <DropdownMenuItem onSelect={() => setNewTaskDialogOpen(true)}><FileInput className="h-4 w-4 text-sky-400" />{t("task.separate")}</DropdownMenuItem>
                  <DropdownMenuItem onSelect={() => setNewTaskDialogOpen(true)}><FileOutput className="h-4 w-4 text-amber-400" />{t("task.merge")}</DropdownMenuItem>
                </DropdownMenuContent>
              </DropdownMenu>
              <Button variant="ghost" size="sm" className="h-7 text-xs gap-1 text-destructive hover:text-destructive"
                disabled={!tasks.some((t) => t.status === "processing" || t.status === "queued")} onClick={handleCancelAllTasks}
              >{t("app.cancelAll")}</Button>
            </div>
            <div className="flex items-center gap-1">
              <Button variant="ghost" size="sm" className="h-7 text-xs gap-1" onClick={() => setSettingsDialogOpen(true)}>
                <Settings className="h-3.5 w-3.5" />{t("app.settings")}
              </Button>
            </div>
          </motion.div>
          <motion.div initial={{ opacity: 0, y: 8 }} animate={{ opacity: 1, y: 0 }} transition={{ duration: 0.3, delay: 0.1 }}
            className="flex flex-1 min-h-0"
          >
            <div className="w-72 shrink-0 bg-sidebar-background flex flex-col glass-sidebar">
              <div className="px-3 py-1.5 text-[11px] text-muted-foreground">{t("app.taskList")} ({tasks.length})</div>
              <div className="flex-1 min-h-0">
                <TaskList tasks={tasks} selectedTaskId={selectedTaskId} onSelectTask={setSelectedTaskId}
                  onRenameTask={handleRenameTask} onDeleteTask={handleDeleteTask} />
              </div>
            </div>
            <div className="flex-1 min-w-0 flex flex-col">
              <TaskDetail task={selectedTask} autoScroll={autoScroll} onCancel={handleCancelTask} onDownload={handleDownloadResult} />
            </div>
          </motion.div>
          <NewTaskDialog open={newTaskDialogOpen} onOpenChange={setNewTaskDialogOpen} onTaskCreate={handleTaskCreate} />
          <SettingsDialog open={settingsDialogOpen} onOpenChange={setSettingsDialogOpen} settings={settings} onSave={handleSaveSettings} />
        </motion.div>
      </TooltipProvider>
    </>
  );
}

export default App;
