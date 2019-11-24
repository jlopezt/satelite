move Config.json Config.json.sav
move MQTTConfig.json MQTTConfig.json.sav
move WiFiConfig.json WiFiConfig.json.sav
move SensoresConfig.json SensoresConfig.json.sav

copy Config.json.produccion Config.json
copy SensoresConfig.json.produccion SensoresConfig.json
copy WiFiConfig.json.produccion WiFiConfig.json
copy MQTTConfig.json.produccion MQTTConfig.json

del Config.json.sav
del WiFiConfig.json.sav
del MQTTConfig.json.sav
del SensoresConfig.json.sav
 
date /T>AA_PRODUCCION
time /t>>AA_PRODUCCION
del AA_DESARROLLO 