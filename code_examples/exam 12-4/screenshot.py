import os
import time
import ctypes
from ctypes import wintypes
from PIL import ImageGrab

WORKSPACE = r"D:\Mine\School\EmbeddedNew\WatchDoorDog"
OUT_PATH = os.path.join(WORKSPACE, "images", "picsimlab_running_full.png")

user32 = ctypes.WinDLL("user32", use_last_error=True)

EnumWindows = user32.EnumWindows
EnumWindowsProc = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
GetWindowTextLengthW = user32.GetWindowTextLengthW
GetWindowTextW = user32.GetWindowTextW
IsWindowVisible = user32.IsWindowVisible
ShowWindow = user32.ShowWindow
SetForegroundWindow = user32.SetForegroundWindow
GetWindowRect = user32.GetWindowRect

SW_RESTORE = 9


def get_window_title(hwnd):
    length = GetWindowTextLengthW(hwnd)
    if length <= 0:
        return ""
    buf = ctypes.create_unicode_buffer(length + 1)
    GetWindowTextW(hwnd, buf, length + 1)
    return buf.value.strip()


def find_picsimlab_window():
    matches = []

    @EnumWindowsProc
    def _enum(hwnd, _lparam):
        if not IsWindowVisible(hwnd):
            return True
        title = get_window_title(hwnd)
        if not title:
            return True
        lower = title.lower()
        if "picsimlab" in lower or "lxrad" in lower or "blue pill" in lower:
            matches.append((hwnd, title))
        return True

    EnumWindows(_enum, 0)

    for hwnd, title in matches:
        if "picsimlab" in title.lower():
            return hwnd
    return matches[0][0] if matches else None


def capture_window(hwnd, out_file):
    ShowWindow(hwnd, SW_RESTORE)
    time.sleep(0.3)
    SetForegroundWindow(hwnd)
    time.sleep(1.0)

    rect = wintypes.RECT()
    if not GetWindowRect(hwnd, ctypes.byref(rect)):
        raise RuntimeError("无法读取窗口坐标")

    if rect.right - rect.left < 200 or rect.bottom - rect.top < 200:
        raise RuntimeError("窗口尺寸异常，可能未正常打开")

    bbox = (rect.left, rect.top, rect.right, rect.bottom)
    img = ImageGrab.grab(bbox=bbox, all_screens=True)
    img.save(out_file)


if __name__ == "__main__":
    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    time.sleep(2)
    hwnd = find_picsimlab_window()
    if not hwnd:
        raise RuntimeError("未找到 PICSimLab 窗口，请先启动仿真")
    capture_window(hwnd, OUT_PATH)
    print(f"Screenshot saved to {OUT_PATH}")
