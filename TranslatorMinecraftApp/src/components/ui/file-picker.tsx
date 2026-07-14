import * as React from "react";
import { useTranslation } from "react-i18next";
import { Button } from "@/components/ui/button";
import { cn } from "@/lib/utils";
import { Upload, X, FileText } from "lucide-react";

interface FilePickerProps {
  value: File | null;
  onChange: (file: File | null) => void;
  accept?: string;
  placeholder?: string;
  disabled?: boolean;
}

const FilePicker = React.forwardRef<HTMLButtonElement, FilePickerProps>(
  ({ value, onChange, accept = ".json", placeholder, disabled }, ref) => {
    const { t } = useTranslation();
    const inputRef = React.useRef<HTMLInputElement>(null);
    const handleClick = () => inputRef.current?.click();
    const handleChange = (e: React.ChangeEvent<HTMLInputElement>) => {
      onChange(e.target.files?.[0] || null);
      if (inputRef.current) inputRef.current.value = "";
    };
    const handleClear = (e: React.MouseEvent) => { e.stopPropagation(); onChange(null); };

    return (
      <div className="relative">
        <input ref={inputRef} type="file" accept={accept} onChange={handleChange} className="hidden" disabled={disabled} />
        {value ? (
          <div onClick={handleClick}
            className={cn("flex items-center gap-2.5 px-3 py-2 rounded-lg border border-border",
              "bg-background hover:bg-secondary/40 cursor-pointer transition-colors group"
            )}
          >
            <FileText className="h-4 w-4 text-primary shrink-0" />
            <span className="flex-1 text-sm truncate min-w-0">{value.name}</span>
            <span className="text-[11px] text-muted-foreground shrink-0 tabular-nums">{(value.size / 1024).toFixed(0)} KB</span>
            <button type="button" onClick={handleClear}
              className="p-0.5 rounded hover:bg-secondary/60 text-muted-foreground hover:text-destructive transition-colors opacity-0 group-hover:opacity-100"
            ><X className="h-3.5 w-3.5" /></button>
          </div>
        ) : (
          <Button ref={ref} type="button" variant="outline" size="sm" disabled={disabled}
            onClick={handleClick} className="w-full h-10 gap-2 text-xs text-muted-foreground"
          >
            <Upload className="h-4 w-4" />
            {placeholder || t("task.selectFile")}
          </Button>
        )}
      </div>
    );
  }
);
FilePicker.displayName = "FilePicker";
export { FilePicker };
