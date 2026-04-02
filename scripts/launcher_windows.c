/*
 * Prestige AI — Windows Launcher
 * Copyright (c) 2024-2026 Prestige Technologie Company
 * All rights reserved.
 *
 * Architecture: 2 processes (no Python)
 *   1. Vision Engine (video + AI + compositor + output — background)
 *   2. Control Room (user interface — foreground)
 *
 * AI runs natively via ONNX Runtime in C++.
 * When Control Room closes, Vision Engine is terminated.
 *
 * Compile: cl /Fe:PrestigeAI.exe launcher_windows.c scripts/launcher.rc /link /SUBSYSTEM:WINDOWS
 */

#include <windows.h>
#include <stdio.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    STARTUPINFOA si;
    PROCESS_INFORMATION piVision = {0};
    PROCESS_INFORMATION piControl = {0};

    /* Get directory of this executable */
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    char visionCmd[MAX_PATH * 2];
    char controlCmd[MAX_PATH * 2];
    sprintf(visionCmd, "\"%sprestige_vision.exe\"", exePath);
    sprintf(controlCmd, "\"%sprestige_control.exe\"", exePath);

    /* ── 1. Start Vision Engine (background — hidden window) ──
     * Includes: video capture, AI detection (ONNX), face tracking,
     * compositor, whisper subtitles, social chat, output routing */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (!CreateProcessA(NULL, visionCmd, NULL, NULL, FALSE,
                        CREATE_NO_WINDOW, NULL, exePath, &si, &piVision))
    {
        MessageBoxA(NULL,
            "Impossible de lancer le Vision Engine.\n"
            "Verifiez que prestige_vision.exe est present.",
            "Prestige AI", MB_OK | MB_ICONERROR);
        return 1;
    }

    /* Wait for Vision Engine to initialize (load ONNX models, open camera) */
    Sleep(2000);

    /* ── 2. Start Control Room (foreground — main window) ── */
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;

    if (!CreateProcessA(NULL, controlCmd, NULL, NULL, FALSE,
                        0, NULL, exePath, &si, &piControl))
    {
        MessageBoxA(NULL,
            "Impossible de lancer le Control Room.\n"
            "Verifiez que prestige_control.exe est present.",
            "Prestige AI", MB_OK | MB_ICONERROR);
        TerminateProcess(piVision.hProcess, 0);
        CloseHandle(piVision.hProcess);
        CloseHandle(piVision.hThread);
        return 1;
    }

    /* Wait for Control Room to close (user exits the application) */
    WaitForSingleObject(piControl.hProcess, INFINITE);

    /* ── 3. Cleanup: terminate Vision Engine ── */
    TerminateProcess(piVision.hProcess, 0);
    WaitForSingleObject(piVision.hProcess, 3000);

    CloseHandle(piVision.hProcess);
    CloseHandle(piVision.hThread);
    CloseHandle(piControl.hProcess);
    CloseHandle(piControl.hThread);

    return 0;
}
