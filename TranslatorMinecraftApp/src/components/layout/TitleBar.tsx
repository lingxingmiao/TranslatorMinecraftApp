import { useState, useCallback } from "react";
import { useTranslation } from "react-i18next";
import { useTauriWindow } from "@/hooks/useTauriWindow";
import { Button } from "@/components/ui/button";
import { Tooltip, TooltipContent, TooltipTrigger } from "@/components/ui/tooltip";
import { Minus, Square, X, Copy } from "lucide-react";

interface TitleBarProps {
  title: string;
  onMinimize?: () => void;
  onMaximize?: () => void;
  onClose?: () => void;
}

export default function TitleBar({ title, onMinimize, onMaximize, onClose }: TitleBarProps) {
  const { t } = useTranslation();
  const [isMaximized, setIsMaximized] = useState(false);
  const { minimize, toggleMaximize, close } = useTauriWindow();

  const handleMinimize = useCallback(() => { minimize(); onMinimize?.(); }, [minimize, onMinimize]);
  const handleMaximize = useCallback(() => { toggleMaximize(); setIsMaximized((prev) => !prev); onMaximize?.(); }, [toggleMaximize, onMaximize]);
  const handleClose = useCallback(() => { close(); onClose?.(); }, [close, onClose]);

  return (
    <div data-tauri-drag-region
      className="flex items-center justify-between h-10 bg-background border-b border-border select-none shrink-0"
    >
      <div className="flex items-center gap-2 pl-3" data-tauri-drag-region>
        <span className="text-sm font-semibold text-foreground/80 tracking-wide">{title}</span>
      </div>
      <div className="flex items-center h-full">
        <Tooltip><TooltipTrigger asChild>
          <Button variant="ghost" size="icon" className="h-full w-11 rounded-none hover:bg-secondary/50" onClick={handleMinimize}>
            <Minus className="h-4 w-4" />
          </Button>
        </TooltipTrigger><TooltipContent side="bottom">{t("titlebar.minimize")}</TooltipContent></Tooltip>
        <Tooltip><TooltipTrigger asChild>
          <Button variant="ghost" size="icon" className="h-full w-11 rounded-none hover:bg-secondary/50" onClick={handleMaximize}>
            {isMaximized ? <Copy className="h-3 w-3" /> : <Square className="h-3 w-3" />}
          </Button>
        </TooltipTrigger><TooltipContent side="bottom">{isMaximized ? t("titlebar.restore") : t("titlebar.maximize")}</TooltipContent></Tooltip>
        <Tooltip><TooltipTrigger asChild>
          <Button variant="ghost" size="icon" className="h-full w-11 rounded-none hover:bg-destructive/20 hover:text-destructive" onClick={handleClose}>
            <X className="h-4 w-4" />
          </Button>
        </TooltipTrigger><TooltipContent side="bottom">{t("titlebar.close")}</TooltipContent></Tooltip>
      </div>
    </div>
  );
}
