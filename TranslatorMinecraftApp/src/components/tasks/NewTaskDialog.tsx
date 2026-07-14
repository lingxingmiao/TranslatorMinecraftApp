import { useState } from "react";
import { useTranslation } from "react-i18next";
import type { TaskType } from "@/lib/types";
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogDescription,
  DialogFooter,
} from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { Separator } from "@/components/ui/separator";
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group";
import { Checkbox } from "@/components/ui/checkbox";
import { FilePicker } from "@/components/ui/file-picker";
import { FileText, FileInput, FileOutput } from "lucide-react";

interface NewTaskDialogProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  onTaskCreate: (type: TaskType, files: Record<string, File | null>, options: Record<string, boolean>) => void;
}

export default function NewTaskDialog({
  open,
  onOpenChange,
  onTaskCreate,
}: NewTaskDialogProps) {
  const { t } = useTranslation();
  const [taskType, setTaskType] = useState<TaskType>("translate");
  const [file0, setFile0] = useState<File | null>(null);
  const [file1, setFile1] = useState<File | null>(null);
  const [notlangFile, setNotlangFile] = useState<File | null>(null);
  const [allMode, setAllMode] = useState(false);
  const [exportInspection, setExportInspection] = useState(false);

  const reset = () => {
    setTaskType("translate");
    setFile0(null);
    setFile1(null);
    setNotlangFile(null);
    setAllMode(false);
    setExportInspection(false);
  };

  const handleSubmit = () => {
    const files: Record<string, File | null> = { file0, file1, notlang: notlangFile };
    const options = { allMode, exportInspection };
    onTaskCreate(taskType, files, options);
    reset();
    onOpenChange(false);
  };

  const isTranslate = taskType === "translate";
  const isSeparate = taskType === "separate";
  const isMerge = taskType === "merge";

  const canSubmit = file0 && (isTranslate || (isSeparate && file1) || (isMerge && file1 && notlangFile));

  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="sm:max-w-md">
        <DialogHeader>
          <DialogTitle>{t("task.newTaskTitle")}</DialogTitle>
          <DialogDescription>{t("task.newTaskDesc")}</DialogDescription>
        </DialogHeader>

        <RadioGroup
          value={taskType}
          onValueChange={(v: string) => setTaskType(v as TaskType)}
          className="grid grid-cols-3 gap-3"
        >
          {[
            { value: "translate", icon: FileText, color: "text-primary", label: t("task.translateShort") },
            { value: "separate", icon: FileInput, color: "text-sky-400", label: t("task.separateShort") },
            { value: "merge", icon: FileOutput, color: "text-amber-400", label: t("task.mergeShort") },
          ].map(({ value, icon: Icon, color, label }) => (
            <Label key={value} htmlFor={value}
              className={`flex flex-col items-center gap-2 p-3 rounded-lg border-2 cursor-pointer transition-all ${
                taskType === value ? "border-primary bg-primary/10" : "border-border hover:border-primary/50"
              }`}
            >
              <RadioGroupItem value={value} id={value} className="sr-only" />
              <Icon className={`h-6 w-6 ${color}`} />
              <span className="text-xs font-medium">{label}</span>
            </Label>
          ))}
        </RadioGroup>

        <Separator />

        <div className="space-y-3">
          <div className="space-y-1.5">
            <Label className="text-xs">
              {isTranslate ? t("task.sourceFile") : isSeparate ? t("task.beforeFile") : t("task.oldFile")}
            </Label>
            <FilePicker value={file0} onChange={setFile0} accept=".json" placeholder={t("task.selectSourceFile")} />
          </div>

          {(isTranslate || isSeparate || isMerge) && (
            <div className="space-y-1.5">
              <Label className="text-xs">
                {isTranslate ? t("task.targetFile") : isSeparate ? t("task.afterFile") : t("task.newFile")}
              </Label>
              <FilePicker value={file1} onChange={setFile1} accept=".json" placeholder={t("task.selectTargetFile")} />
            </div>
          )}

          {isMerge && (
            <div className="space-y-1.5">
              <Label className="text-xs">{t("task.notlangFile")}</Label>
              <FilePicker value={notlangFile} onChange={setNotlangFile} accept=".json" placeholder={t("task.selectNotlangFile")} />
            </div>
          )}
        </div>

        {isTranslate && (
          <div className="flex items-center gap-6">
            <Label className="flex items-center gap-2 cursor-pointer text-xs">
              <Checkbox checked={allMode} onCheckedChange={(c) => setAllMode(c === true)} />
              {t("task.allMode")}
            </Label>
            <Label className="flex items-center gap-2 cursor-pointer text-xs">
              <Checkbox checked={exportInspection} onCheckedChange={(c) => setExportInspection(c === true)} />
              {t("task.exportInspection")}
            </Label>
          </div>
        )}

        <DialogFooter>
          <Button variant="outline" onClick={() => onOpenChange(false)}>{t("task.cancelBtn")}</Button>
          <Button onClick={handleSubmit} disabled={!canSubmit}>{t("task.submit")}</Button>
        </DialogFooter>
      </DialogContent>
    </Dialog>
  );
}
