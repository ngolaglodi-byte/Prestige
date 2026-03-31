' Prestige AI Launcher
' Copyright (c) 2024-2026 Prestige Technologie Company

Set WshShell = CreateObject("WScript.Shell")
strPath = CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName)

' Start Vision Engine (hidden)
WshShell.Run """" & strPath & "\prestige_vision.exe""", 0, False

' Wait 2 seconds
WScript.Sleep 2000

' Start AI Engine (hidden)
WshShell.Run """" & strPath & "\ai_engine\venv\Scripts\pythonw.exe"" """ & strPath & "\ai_engine\main.py""", 0, False

' Wait 2 seconds
WScript.Sleep 2000

' Start Control Room (visible - main window)
WshShell.Run """" & strPath & "\prestige_control.exe""", 1, True

' When Control Room closes, kill Vision Engine
WshShell.Run "taskkill /f /im prestige_vision.exe", 0, True
