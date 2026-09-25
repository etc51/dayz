' Запуск autosync-run.cmd без мигающего окна консоли.
Set fso = CreateObject("Scripting.FileSystemObject")
dir = fso.GetParentFolderName(WScript.ScriptFullName)
CreateObject("WScript.Shell").Run """" & dir & "\autosync-run.cmd""", 0, True
