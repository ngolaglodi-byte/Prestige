/*
 * Prestige AI — Windows Launcher
 * Copyright (c) 2024-2026 Prestige Technologie Company
 * All rights reserved.
 *
 * This launcher starts all 3 Prestige AI processes silently:
 * 1. prestige_vision.exe (Vision Engine)
 * 2. python ai_engine/main.py (AI Engine)
 * 3. prestige_control.exe (Control Room — main window)
 *
 * When the Control Room closes, it terminates the other processes.
 * Compiled with: cl /Fe:PrestigeAI.exe launcher_windows.c /link /SUBSYSTEM:WINDOWS
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>

static PROCESS_INFORMATION g_vision = {0};
static PROCESS_INFORMATION g_ai = {0};

void LaunchHidden(const char* cmd, PROCESS_INFORMATION* pi) {
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    char cmdBuf[2048];
    strncpy(cmdBuf, cmd, sizeof(cmdBuf) - 1);
    cmdBuf[sizeof(cmdBuf) - 1] = '\0';

    CreateProcessA(NULL, cmdBuf, NULL, NULL, FALSE,
                   CREATE_NO_WINDOW | DETACHED_PROCESS,
                   NULL, NULL, &si, pi);
}

void Cleanup() {
    if (g_vision.hProcess) {
        TerminateProcess(g_vision.hProcess, 0);
        CloseHandle(g_vision.hProcess);
        CloseHandle(g_vision.hThread);
    }
    if (g_ai.hProcess) {
        TerminateProcess(g_ai.hProcess, 0);
        CloseHandle(g_ai.hProcess);
        CloseHandle(g_ai.hThread);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    /* Get directory of this exe */
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) *(lastSlash + 1) = '\0';

    char visionCmd[2048];
    char aiCmd[2048];
    char controlCmd[2048];

    snprintf(visionCmd, sizeof(visionCmd), "\"%sprestige_vision.exe\"", exePath);
    snprintf(aiCmd, sizeof(aiCmd), "\"%sai_engine\\venv\\Scripts\\pythonw.exe\" \"%sai_engine\\main.py\"", exePath, exePath);
    snprintf(controlCmd, sizeof(controlCmd), "\"%sprestige_control.exe\"", exePath);

    /* Start Vision Engine (hidden) */
    LaunchHidden(visionCmd, &g_vision);
    Sleep(2000);

    /* Start AI Engine (hidden) */
    LaunchHidden(aiCmd, &g_ai);
    Sleep(2000);

    /* Start Control Room (visible — main window) */
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char ctrlBuf[2048];
    strncpy(ctrlBuf, controlCmd, sizeof(ctrlBuf) - 1);
    ctrlBuf[sizeof(ctrlBuf) - 1] = '\0';

    CreateProcessA(NULL, ctrlBuf, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    /* Wait for Control Room to exit */
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    /* Cleanup — kill background processes */
    Cleanup();

    return 0;
}
