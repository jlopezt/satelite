del D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data\*.* /q /s

xcopy .\desarrollo\*.* D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data /S /Y

date /T>AA_DESARROLLO
time /T>>AA_DESARROLLO 
xcopy AA_DESARROLLO D:\arduino\desarrollos\Sketchs\Termostato\Codigo\Satelite\data /Y
del AA_PRODUCCION
