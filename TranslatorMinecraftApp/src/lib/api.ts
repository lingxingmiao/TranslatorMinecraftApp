import { API_BASE } from "./constants";
import type {
  TaskCreateResponse,
  TaskStatusResponse,
  TaskLogsResponse,
  TaskType,
} from "./types";

async function request<T>(
  endpoint: string,
  options: RequestInit = {}
): Promise<T> {
  const url = `${API_BASE}${endpoint}`;
  const res = await fetch(url, {
    headers: {
      "Content-Type": "application/json",
      ...options.headers,
    },
    ...options,
  });
  if (!res.ok) {
    const text = await res.text();
    throw new Error(`API error ${res.status}: ${text}`);
  }
  return res.json();
}

export async function submitTranslateTask(
  file0: File,
  file1: File | null,
  allMode: boolean,
  exportInspection: boolean
): Promise<TaskCreateResponse> {
  const formData = new FormData();
  formData.append("file0", file0);
  if (file1) formData.append("file1", file1);
  formData.append("allMode", String(allMode));
  formData.append("exportInspection", String(exportInspection));

  const res = await fetch(`${API_BASE}/translate`, {
    method: "POST",
    body: formData,
  });
  if (!res.ok) throw new Error(`Submit failed: ${await res.text()}`);
  return res.json();
}

export async function submitSeparateTask(
  file0: File,
  file1: File
): Promise<TaskCreateResponse> {
  const formData = new FormData();
  formData.append("file0", file0);
  formData.append("file1", file1);
  const res = await fetch(`${API_BASE}/separatelangupdate`, {
    method: "POST",
    body: formData,
  });
  if (!res.ok) throw new Error(`Submit failed: ${await res.text()}`);
  return res.json();
}

export async function submitMergeTask(
  file0: File,
  notlangFile: File,
  file1: File
): Promise<TaskCreateResponse> {
  const formData = new FormData();
  formData.append("file0", file0);
  formData.append("notlang", notlangFile);
  formData.append("file1", file1);
  const res = await fetch(`${API_BASE}/mergelangupdate`, {
    method: "POST",
    body: formData,
  });
  if (!res.ok) throw new Error(`Submit failed: ${await res.text()}`);
  return res.json();
}

export async function getTaskStatus(
  taskId: string
): Promise<TaskStatusResponse> {
  return request<TaskStatusResponse>(`/task/status/${taskId}`);
}

export async function getTaskLogs(
  taskId: string
): Promise<TaskLogsResponse> {
  return request<TaskLogsResponse>(`/task/logs/${taskId}/json`);
}

export async function cancelTask(taskId: string): Promise<void> {
  await request(`/task/cancel/${taskId}`, { method: "POST" });
}

export async function downloadTaskResult(
  taskId: string,
  filename: string
): Promise<void> {
  const url = `${API_BASE}/task/download/${taskId}/${filename}`;
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.click();
}
