import type { ThemeConfig } from "@/lib/theme";

// ==================== Task Types ====================

export type TaskType = "translate" | "separate" | "merge";

export type TaskStatus = "queued" | "processing" | "completed" | "failed" | "cancelled";

export interface TaskInfo {
  taskId: string;
  displayName: string;
  typeName: TaskType;
  status: TaskStatus;
  progress: number;
  logs: LogEntry[];
  filename?: string;
  downloadUrl?: string;
  createdAt?: string;
}

export interface LogEntry {
  level: "INFO" | "WARNING" | "ERROR" | "DEBUG";
  message: string;
  timestamp?: string;
}

// ==================== API Types ====================

export interface TaskCreateResponse {
  taskId: string;
  typeName: TaskType;
  status: TaskStatus;
}

export interface TaskStatusResponse {
  taskId: string;
  status: TaskStatus;
  progress: number;
  error?: string;
  filename?: string;
  downloadUrl?: string;
}

export interface TaskLogsResponse {
  logs: LogEntry[];
}

// ==================== Settings Types ====================

export interface LlmEndpointConfig {
  id: string;
  name: string;
  url: string;
  key: string;
  model: string;
  temperature: number;
  topP: number;
  topK: number;
  presencePenalty: number;
  frequencyPenalty: number;
  seed: number;
  maxRetries: number;
  retryDelay: number;
  maxConcurrency: number;
  weight: number;
  activeHours: string;
  enabled: boolean;
}

export interface AppSettings {
  apiUrl: string;
  apiKey: string;
  savePath: string;
  maxLogLines: number;
  pollInterval: number;
  autoScroll: boolean;
  endpoints: LlmEndpointConfig[];
  theme: ThemeConfig;
}

// ==================== Lang File Types ====================

export interface LangEntry {
  key: string;
  value: string;
}

export interface TooltipEntry {
  key: string;
  value: string;
}
