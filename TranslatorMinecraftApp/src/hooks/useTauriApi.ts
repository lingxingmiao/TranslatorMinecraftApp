import { useState, useEffect } from "react";

export function useTauriApi() {
  const [isTauri, setIsTauri] = useState(false);

  useEffect(() => {
    setIsTauri(
      typeof window !== "undefined" &&
        window.__TAURI_INTERNALS__ !== undefined
    );
  }, []);

  return { isTauri };
}
