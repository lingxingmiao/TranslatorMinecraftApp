import * as React from "react";
import { Button } from "@/components/ui/button";
import { cn } from "@/lib/utils";
import { ChevronUp, ChevronDown } from "lucide-react";

interface NumberInputProps extends Omit<React.ComponentProps<"input">, "type" | "onChange" | "value"> {
  value: number;
  onChange: (value: number) => void;
  min?: number;
  max?: number;
  step?: number;
}

const NumberInput = React.forwardRef<HTMLInputElement, NumberInputProps>(
  ({ className, value, onChange, min, max, step = 1, disabled, ...props }, ref) => {
    const handleUp = () => {
      const next = value + step;
      if (max !== undefined && next > max) return;
      onChange(next);
    };

    const handleDown = () => {
      const next = value - step;
      if (min !== undefined && next < min) return;
      onChange(next);
    };

    const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
      const raw = e.target.value;
      if (raw === "" || raw === "-") {
        onChange(0);
        return;
      }
      const parsed = Number(raw);
      if (isNaN(parsed)) return;
      let clamped = parsed;
      if (min !== undefined) clamped = Math.max(min, clamped);
      if (max !== undefined) clamped = Math.min(max, clamped);
      onChange(clamped);
    };

    return (
      <div
        className={cn(
          "flex items-center rounded-lg border border-input bg-background shadow-sm",
          "focus-within:ring-2 focus-within:ring-ring/40 focus-within:border-primary/50",
          "transition-all duration-150",
          disabled && "opacity-50 cursor-not-allowed",
          "min-w-[7rem] w-auto",
          className
        )}
      >
        {/* Decrease button */}
        <Button
          type="button"
          variant="ghost"
          size="icon"
          disabled={disabled || (min !== undefined && value <= min)}
          className={cn(
            "h-8 w-8 rounded-none rounded-l-lg border-r border-border shrink-0",
            "hover:bg-primary/10 hover:text-primary active:bg-primary/20",
            "transition-colors duration-100"
          )}
          onClick={handleDown}
          tabIndex={-1}
        >
          <ChevronDown className="h-3.5 w-3.5" />
        </Button>

        {/* Input */}
        <input
          ref={ref}
          type="text"
          inputMode="numeric"
          value={value}
          onChange={handleInputChange}
          disabled={disabled}
          className={cn(
            "h-8 flex-1 min-w-[3rem] bg-transparent text-center text-sm tabular-nums",
            "outline-none border-none px-1",
            "text-foreground placeholder:text-muted-foreground",
            "disabled:cursor-not-allowed"
          )}
          {...props}
        />

        {/* Increase button */}
        <Button
          type="button"
          variant="ghost"
          size="icon"
          disabled={disabled || (max !== undefined && value >= max)}
          className={cn(
            "h-8 w-8 rounded-none rounded-r-lg border-l border-border shrink-0",
            "hover:bg-primary/10 hover:text-primary active:bg-primary/20",
            "transition-colors duration-100"
          )}
          onClick={handleUp}
          tabIndex={-1}
        >
          <ChevronUp className="h-3.5 w-3.5" />
        </Button>
      </div>
    );
  }
);
NumberInput.displayName = "NumberInput";

export { NumberInput };
