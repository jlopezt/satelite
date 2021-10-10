del D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data\*.* /q /s

xcopy .\produccion\*.* D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data /S /Y

date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
xcopy AA_PRODUCCION D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data /Y
del AA_DESARROLLO 