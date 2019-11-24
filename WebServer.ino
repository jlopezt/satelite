/************************************************************************************************
Servicio                                    URL                            Formato entrada Formato salida                       Comentario                                                                                       Ejemplo peticion                    Ejemplo respuesta
Servicio de informacion                     http://IPSatelite              N/A             N/A                                  Nombre del modulo y version                                                  
Servicio de consulta de valores temperatura http://IPSatelite/temperatura  id=<id>         <temperatura>,<id2>                  Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/temperatura?id=59 23.5|43682
Servicio de consulta de valores humedad     http://IPSatelite/humedad      id=<id>         <humedad>,<id2>                      Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/humedad?id=59     46.2|43682
Servicio de consulta de valores luz         http://IPSatelite/luz          id=<id>         <luz>,<id2>                          Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/luz?id=59         97.3|43682
Servicio de consulta de valores medidos     http://IPSatelite/medidas      id=<id>         <temperatura>|<humedad>|<luz>|<id2>  Devuelve la temperatura y el id_2 en funcion del id de entrada (por seguridad)                   http://IPSatelite/medidas?id=59     23.5|46.2|97.3|43682
Web de valores medidos                      http://IPSatelite/web          N/A             HTML                                 Devuelve una pagina web para la consulta desde navegador                                         http://IPSatelite/web  
Servicio de test                            http://IPSatelite/test         N/A             HTML                                 Devuelve un texto html para verificar conectividad desde navegador                               http://IPSatelite/test 
************************************************************************************************/
//enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE, HTTP_OPTIONS };
#define IDENTIFICACION "Modulo " + String(direccion) + " Habitacion= " + nombres[direccion] + "<BR>"

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer server(PUERTO_WEBSERVER);

String cabeceraHTML="<HTML><HEAD><TITLE>" + nombre_dispositivo + " </TITLE></HEAD><BODY><h1><a href=\"../\" target=\"_self\">" + nombre_dispositivo + "</a><br></h1>";
String enlaces="<TABLE>\n<CAPTION>Enlaces</CAPTION>\n<TR><TD><a href=\"info\" target=\"_self\">Info</a></TD></TR>\n<TR><TD><a href=\"test\" target=\"_self\">Test</a></TD></TR>\n<TR><TD><a href=\"restart\" target=\"_self\">Restart</a></TD></TR>\n<TR><TD><a href=\"listaFicheros\" target=\"_self\">Lista ficheros</a></TD></TR>\n</TABLE>"; 
String pieHTML="</BODY></HTML>";

void inicializaWebServer(void)
  {
  //decalra las URIs a las que va a responder
  server.on("/", handleRoot); //web de temperatura
  server.on("/temperatura", handleTemperatura); //Servicio de temperatura
  server.on("/humedad", handleHumedad); //Servicio de temperatura
  server.on("/luz", handleLuz); //Servicio de temperatura    
  server.on("/medidas", handleMedidas); //Servicio de temperatura
  server.on("/medida", handleMedida); //Servicio de temperatura con respuesta Json
  server.on("/web", handleWeb); //Servicio de temperatura 
   
  server.on("/test", handleTest);  //URI de test
  server.on("/reset", handleReset);  //URI de test  
  server.on("/restart", handleRestart);  //URI de test
  server.on("/info", handleInfo);  //URI de test
  
  server.on("/listaFicheros", HTTP_ANY, handleListaFicheros);  //URI de leer fichero
  server.on("/creaFichero", handleCreaFichero);  //URI de crear fichero
  server.on("/borraFichero", handleBorraFichero);  //URI de borrar fichero
  server.on("/leeFichero", handleLeeFichero);  //URI de leer fichero
  server.on("/manageFichero", HTTP_ANY, handleManageFichero);  //URI de leer fichero  
  server.on("/infoFS", handleInfoFS);  //URI de info del FS

  server.onNotFound(handleNotFound);//pagina no encontrada

  server.begin();
  Serial.println("HTTP server started");
  }

void webServer(int debug)
  {
  server.handleClient();
  }

void handleRoot() 
  {
  String cad="";

  cad += cabeceraHTML;
  //genero la respuesta por defecto
  cad += "Modulo satelite. Version ";
  cad += VERSION;
  cad += "<BR>";
  cad += IDENTIFICACION;

/***********************************/

  cad += "Valores medidos: ";
  cad += generaJson();

/***********************************/
  cad +="<BR><BR>";
  cad += "<table>";  
  cad += "<tr><td>Servicio</td><td>URL</td><td>Formato entrada</td><td>Formato salida</td><td>Comentario</td><td>Ejemplo peticion</td><td>Ejemplo respuesta</td></tr>";
  cad += "<tr><td>Informacion</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/"">http://" + String(getIP(debugGlobal)) + "</a></td><td>N/A</td><td>N/A</td><td>Nombre del modulo y version</td><td>http://" + String(getIP(debugGlobal)) + "</td><td>N/A</td></tr>";
  cad += "<tr><td>Consulta de valores temperatura</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/temperatura"">http://" + String(getIP(debugGlobal)) + "/temperatura</a></td><td>N/A</td><td>{""Temperatura"": $valor$}</td><td>json con la temperatura leida</td><td>http://" + String(getIP(debugGlobal)) + "/temperatura</td><td>{""Temperatura"": 24.3}</td></tr>";
  cad += "<tr><td>Consulta de valores humedad</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/humedad"">http://" + String(getIP(debugGlobal)) + "/humedad</a></td><td>N/A</td><td>{""Hunmedad"": $valor$}</td><td>json con la humedad leida</td><td>http://" + String(getIP(debugGlobal)) + "/humedad</td><td>{""Humedad"": 24.3}</td></tr>";
  cad += "<tr><td>Consulta de valores luz</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/luz"">http://" + String(getIP(debugGlobal)) + "/luz</a></td><td>N/A</td><td>{""Luz"": $valor$}</td><td>json con la luz leida</td><td>http://" + String(getIP(debugGlobal)) + "/luz</td><td>{""Luz"": 24.3}</td></tr>";

  cad += "<tr><td>Consulta de valores medidos</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/medidas"">http://" + String(getIP(debugGlobal)) + "/medidas</a></td><td>N/A</td><td>$temp$|$hum$|$luz$|</td><td>Devuelve los valores medidos en formato texto</td><td>http://" + String(getIP(debugGlobal)) + "/medidas</td><td>0|0</td></tr>";    
  cad += "<tr><td>Consulta de valores medidos (json)</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/medida""> http://" + String(getIP(debugGlobal)) + "/medida </a></td><td>N/A</td><td>{""Temperatura"": $valor$,""Humedad"": $valor$,""Luz"": $valor$,""id"": 0}</td><td>Devuelve los valores medidos en formato json</td><td>http://" + String(getIP(debugGlobal)) + "/medida</td><td>{""Temperatura"": 24.4,""Humedad"": 34.9,""Luz"": 37.0,""id"": 0}</td></tr>";    
  
  cad += "<tr><td>Web de valores medidos</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/web"">http://" + String(getIP(debugGlobal)) + "/web</a></td><td>N/A</td><td>HTML</td><td>Devuelve una pagina web para la consulta desde navegador</td><td>http://" + String(getIP(debugGlobal)) + "/web</td><td></td></tr>";    
  cad += "<tr><td>Test</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/test"">http://" + String(getIP(debugGlobal)) + "/test</a></td><td>N/A</td><td>HTML</td><td>Verifica el estado del Actuador</td></tr>";   
  cad += "<tr><td>Reinicia el controlador</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/restart"">http://" + String(getIP(debugGlobal)) + "/restart</a></td><td>N/A</td><td>N/A</td><td>Reinicia el modulo</td></tr>";
  
  cad += "<tr><td>Informacion del Hw del sistema</td><td><a href=""http://" + String(getIP(debugGlobal)) + "/info"">http://" + String(getIP(debugGlobal)) + "/info</a></td><td>N/A</td><td>HTML</td><td>Informacion del Hw del modulo</td></tr>";
  cad += "</table>";
  cad +="<BR><BR>";
/***********************************/
  
  cad += enlaces;
  cad += "<BR>";
  cad += "vueltas= " + String(vuelta) + " / " + String(UINT16_MAX);
  cad += pieHTML;
  
  server.send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Servicio de consulta de valores medidos  */
/*                                           */
/*********************************************/
void handleMedidas() 
  {
  String cad="";

  cad += cabeceraHTML;  
  //genero la respuesta OK
  cad = String(getTemperatura(),1);
  cad += SEPARADOR;
  cad += String(getHumedad(),1);
  cad += SEPARADOR;
  cad += String(getLuz(),1);
  cad += SEPARADOR;

  cad += pieHTML;
  
  server.send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Medidas                                  */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Temperatura\":23.2,\"Humedad\":45.5,\"Luz\":14}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Temperatura"]; // 23.2
 *  float Humedad = root["Humedad"]; // 45.5
 *  int Luz = root["Luz"]; // 14
 */
/*********************************************/
void handleMedida() 
  {
  String cad=generaJson();
  
  server.send(200, "application/json", cad);   
   
  //Serial.println("Medidas requeridas ok");
  }

/*********************************************/
/*                                           */
/*  Medida de temperatura                    */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *  const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Temperatura\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Temperatura"]; // 23.1
*/    
/*********************************************/
void handleTemperatura() 
  {
  String cad="";

  //genero la respuesta OK --> {"Temperatura": 22.5}
  cad  = "{\"Temperatura\": ";
  cad += String(getTemperatura(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*  Medida de humedad                    */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Humedad\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Humedad"]; // 23.1
*/  
/*********************************************/
void handleHumedad() 
  {
  String cad="";

  //genero la respuesta OK --> {"Humedad": 22.5}
  cad  = "{\"Humedad\": ";
  cad += String(getHumedad(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*  Medida de Luz                            */
/*  Servicio de consulta de valores medidos  */
/*  Devuelve un formato json                 */
/*                                           */
/*  Codigo para leer el JSON:
 *   const size_t bufferSize = JSON_OBJECT_SIZE(1) + 20;
 *  DynamicJsonBuffer jsonBuffer(bufferSize);
 *
 *  const char* json = "{\"Luz\":23.1}";
 *
 *  JsonObject& root = jsonBuffer.parseObject(json);
 *
 *  float Temperatura = root["Luz"]; // 23.1
*/  
/*********************************************/
void handleLuz() 
  {
  String cad="";
  
  //genero la respuesta OK --> {"Luz": 22.5}
  cad  = "{\"Luz\": ";
  cad += String(getLuz(),1);
  cad += "}";
  
  server.send(200, "application/json", cad);    
  }

/*********************************************/
/*                                           */
/*    Web de consulta de valores medidos     */
/*                                           */
/*********************************************/
void handleWeb() 
  {
  String cad="";

  cad  = cabeceraHTML;
  cad += IDENTIFICACION; //"Modulo " + String(direccion) + " Habitacion= " + nombres[direccion];
  cad += "<h1>Temperatura: ";
  cad += String(getTemperatura(),1);
  cad += "&deg; </h1>";
  cad += "<h1>Humedad: ";
  cad += String(getHumedad(),1);
  cad += "% </h1>";
  cad += "<h1>Luz: ";
  cad += String(getLuz(),1);
  cad += "</h1>";
  
  cad += pieHTML;
  
  server.send(200, "text/html", cad);
  }

/*********************************************/
/*                                           */
/*  Servicio de test                         */
/*                                           */
/*********************************************/  
void handleTest(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  cad += "Test OK<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Resetea el dispositivo mediante          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleReset(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  cad += "Reseteando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);
  delay(100);     
  ESP.reset();
  }
  
/*********************************************/
/*                                           */
/*  Reinicia el dispositivo mediante         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleRestart(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  cad += "Reiniciando...<br>";
  cad += pieHTML;
    
  server.send(200, "text/html", cad);     
  delay(100);
  ESP.restart();
  }

/*********************************************/
/*                                           */
/*  Lee info del chipset mediante            */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfo(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;

  cad += "<BR>-----------------info logica-----------------<BR>";
  cad += "IP: " + String(getIP(debugGlobal));
  cad += "<BR>";  

  cad += "id: ";
  cad += String(direccion);
  cad += "<BR>";   
  
  cad += "Temperatura: ";
  cad += String(getTemperatura(),1);
  cad += "<BR>";     
  cad += "Humedad: ";
  cad += String(getHumedad(),1);
  cad += "<BR>";     
  cad += "Luz: ";
  cad += String(getLuz(),1);
  cad += "<BR>";     
  cad += "-----------------------------------------------<BR>"; 
  
  cad += "<BR>-----------------Contadores info-----------------<BR>";
  cad += "multiplicadorAnchoIntervalo: ";
  cad += String(multiplicadorAnchoIntervalo);
  cad += "<BR>";     
  cad += "anchoIntervalo: ";
  cad += String(anchoIntervalo);
  cad += "<BR>";   
  cad += "frecuenciaOTA: ";
  cad += String(frecuenciaOTA);
  cad += "<BR>";   
  cad += "frecuenciaLeeSensores: ";
  cad += String(frecuenciaLeeSensores);
  cad += "<BR>";   
  cad += "frecuenciaServidorWeb: ";
  cad += String(frecuenciaServidorWeb);
  cad += "<BR>";   
  cad += "frecuenciaOrdenes: ";
  cad += String(frecuenciaOrdenes);
  cad += "<BR>"; 
  cad += "frecuenciaMQTT: ";
  cad += String(frecuenciaMQTT);
  cad += "<BR>";
  cad += "frecuenciaWifiWatchdog: ";
  cad += String(frecuenciaWifiWatchdog); 
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>"; 
  
  cad += "<BR>-----------------WiFi info-----------------<BR>";
  cad += "SSID: " + nombreSSID();
  cad += "<BR>";    
  cad += "IP: " + WiFi.localIP().toString();
  cad += "<BR>";    
  cad += "Potencia: " + String(WiFi.RSSI());
  cad += "<BR>";    
  cad += "-----------------------------------------------<BR>";  
/*
  cad += "<BR>-----------------MQTT info-----------------<BR>";
  cad += "IP broker: " + IPBroker.toString();
  cad += "<BR>";
  cad += "Puerto broker: " +   puertoBroker=0;
  cad += "<BR>";  
  cad += "Usuario: " + usuarioMQTT="";
  cad += "<BR>";  
  cad += "Password: " + passwordMQTT="";
  cad += "<BR>";  
  cad += "Topic root: " + topicRoot="";
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  
*/    
  cad += "<BR>-----------------Hardware info-----------------<BR>";
  cad += "Vcc: " + String(ESP.getVcc());
  cad += "<BR>";  
  cad += "FreeHeap: " + String(ESP.getFreeHeap());
  cad += "<BR>";
  cad += "ChipId: " + String(ESP.getChipId());
  cad += "<BR>";  
  cad += "SdkVersion: " + String(ESP.getSdkVersion());
  cad += "<BR>";  
  cad += "CoreVersion: " + ESP.getCoreVersion();
  cad += "<BR>";  
  cad += "FullVersion: " + ESP.getFullVersion();
  cad += "<BR>";  
  cad += "BootVersion: " + String(ESP.getBootVersion());
  cad += "<BR>";  
  cad += "BootMode: " + String(ESP.getBootMode());
  cad += "<BR>";  
  cad += "CpuFreqMHz: " + String(ESP.getCpuFreqMHz());
  cad += "<BR>";  
  cad += "FlashChipId: " + String(ESP.getFlashChipId());
  cad += "<BR>";  
     //gets the actual chip size based on the flash id
  cad += "FlashChipRealSize: " + String(ESP.getFlashChipRealSize());
  cad += "<BR>";  
     //gets the size of the flash as set by the compiler
  cad += "FlashChipSize: " + String(ESP.getFlashChipSize());
  cad += "<BR>";  
  cad += "FlashChipSpeed: " + String(ESP.getFlashChipSpeed());
  cad += "<BR>";  
     //FlashMode_t ESP.getFlashChipMode());
  cad += "FlashChipSizeByChipId: " + String(ESP.getFlashChipSizeByChipId());  
  cad += "<BR>";  
  cad += "-----------------------------------------------<BR>";  

  cad += "<BR>-----------------info fileSystem-----------------<BR>";   
  FSInfo fs_info;
  if(SPIFFS.info(fs_info)) 
    {
    /*        
     struct FSInfo {
        size_t totalBytes;
        size_t usedBytes;
        size_t blockSize;
        size_t pageSize;
        size_t maxOpenFiles;
        size_t maxPathLength;
    };
     */
    cad += "totalBytes: ";
    cad += fs_info.totalBytes;
    cad += "<BR>usedBytes: ";
    cad += fs_info.usedBytes;
    cad += "<BR>blockSize: ";
    cad += fs_info.blockSize;
    cad += "<BR>pageSize: ";
    cad += fs_info.pageSize;    
    cad += "<BR>maxOpenFiles: ";
    cad += fs_info.maxOpenFiles;
    cad += "<BR>maxPathLength: ";
    cad += fs_info.maxPathLength;
    }
  else cad += "Error al leer info";
  cad += "<BR>-----------------------------------------------<BR>"; 
  
  cad += pieHTML;
  server.send(200, "text/html", cad);     
  }

/*********************************************/
/*                                           */
/*  Crea un fichero a traves de una          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleCreaFichero(void)
  {
  String cad="";
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;

  cad += cabeceraHTML;
  cad += IDENTIFICACION;

  if(server.hasArg("nombre") && server.hasArg("contenido")) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    contenidoFichero=server.arg("contenido");

    if(salvaFichero( nombreFichero, nombreFichero+".bak", contenidoFichero)) cad += "Fichero salvado con exito<br>";
    else cad += "No se pudo salvar el fichero<br>"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Borra un fichero a traves de una         */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleBorraFichero(void)
  {
  String nombreFichero="";
  String contenidoFichero="";
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(borraFichero(nombreFichero)) cad += "El fichero " + nombreFichero + " ha sido borrado.\n";
    else cad += "No sepudo borrar el fichero " + nombreFichero + ".\n"; 
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee un fichero a traves de una           */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleLeeFichero(void)
  {
  String cad=cabeceraHTML;
  String nombreFichero="";
  String contenido="";
  
  cad += "<h1>" + String(NOMBRE_FAMILIA) + "</h1>";
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";           
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";   
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lee info del FS                          */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/  
void handleInfoFS(void)
  {
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("---------------------------------------------------------------\nmounted file system");  
    FSInfo fs_info;
    if(SPIFFS.info(fs_info)) 
      {
      /*        
       struct FSInfo {
          size_t totalBytes;
          size_t usedBytes;
          size_t blockSize;
          size_t pageSize;
          size_t maxOpenFiles;
          size_t maxPathLength;
      };
       */
      cad += "totalBytes: ";
      cad += fs_info.totalBytes;
      cad += "<BR>usedBytes: ";
      cad += fs_info.usedBytes;
      cad += "<BR>blockSize: ";
      cad += fs_info.blockSize;
      cad += "<BR>pageSize: ";
      cad += fs_info.pageSize;    
      cad += "<BR>maxOpenFiles: ";
      cad += fs_info.maxOpenFiles;
      cad += "<BR>maxPathLength: ";
      cad += fs_info.maxPathLength;
      }
    else cad += "Error al leer info";

    Serial.println("unmounted file system\n---------------------------------------------------------------");
    }//La de abrir el sistema de ficheros

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Pagina no encontrada                     */
/*                                           */
/*********************************************/
void handleNotFound()
  {
  String message = "";//"<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";

  message = "<h1>" + String(NOMBRE_FAMILIA) + "<br></h1>";
  message += "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i=0; i<server.args(); i++)
    {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    
  server.send(404, "text/html", message);
  }

/*********************************************/
/*                                           */
/*  Habilita la edicion y borrado de los     */
/*  ficheros en el sistema a traves de una   */
/*  peticion HTTP                            */ 
/*                                           */
/*********************************************/ 
void handleManageFichero(void)
  {
  String nombreFichero="";
  String contenido="";
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  if(server.hasArg("nombre") ) //si existen esos argumentos
    {
    nombreFichero=server.arg("nombre");
    cad += "<h2>Fichero: " + nombreFichero + "</h2><BR>";  

    if(leeFichero(nombreFichero, contenido))
      {
      cad += "El fichero tiene un tama&ntilde;o de ";
      cad += contenido.length();
      cad += " bytes.<BR>";
      cad += "El contenido del fichero es:<BR>";
      cad += "<textarea readonly=true cols=75 rows=20 name=\"contenido\">";
      cad += contenido;
      cad += "</textarea>";
      cad += "<BR>";

      cad += "<table><tr><td>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "  </p>";
      cad += "</form>";
      cad += "</td></tr></table>";
      
      cad += "<table>Modificar fichero<tr><td>";      
      cad += "<form action=\"creaFichero\" target=\"_self\">";
      cad += "  <p>";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + nombreFichero + "\">";
      cad += "    contenido del fichero: <br><textarea cols=75 rows=20 name=\"contenido\">" + contenido + "</textarea>";
      cad += "    <BR>";
      cad += "    <input type=\"submit\" value=\"salvar\">";
      cad += "  </p>";
      cad += "</td></tr></table>";
      }
    else cad += "Error al abrir el fichero " + nombreFichero + "<BR>";
    }
  else cad += "Falta el argumento <nombre de fichero>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }

/*********************************************/
/*                                           */
/*  Lista los ficheros en el sistema a       */
/*  traves de una peticion HTTP              */ 
/*                                           */
/*********************************************/  
void handleListaFicheros(void)
  {
  String nombreFichero="";
  String contenidoFichero="";
  boolean salvado=false;
  String cad="";

  cad += cabeceraHTML;
  cad += IDENTIFICACION;
  
  //Variables para manejar la lista de ficheros
  String contenido="";
  String fichero="";  
  int16_t to=0;
  
  if(listaFicheros(contenido)) 
    {
    Serial.printf("contenido inicial= %s\n",contenido.c_str());      
    //busco el primer separador
    to=contenido.indexOf(SEPARADOR); 

    cad +="<style> table{border-collapse: collapse;} th, td{border: 1px solid black; padding: 10px; text-align: left;}</style>";
    cad += "<TABLE>";
    while(to!=-1)
      {
      fichero=contenido.substring(0, to);//cojo el principio como el fichero
      contenido=contenido.substring(to+1); //la cadena ahora es desde el separador al final del fichero anterior
      to=contenido.indexOf(SEPARADOR); //busco el siguiente separador

      cad += "<TR><TD>" + fichero + "</TD>";           
      cad += "<TD>";
      cad += "<form action=\"manageFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"editar\">";
      cad += "</form>";
      cad += "</TD><TD>";
      cad += "<form action=\"borraFichero\" target=\"_self\">";
      cad += "    <input type=\"hidden\" name=\"nombre\" value=\"" + fichero + "\">";
      cad += "    <input type=\"submit\" value=\"borrar\">";
      cad += "</form>";
      cad += "</TD></TR>";
      }
    cad += "</TABLE>\n";
    cad += "<BR>";
    
    //Para crear un fichero nuevo
    cad += "<h2>Crear un fichero nuevo:</h2>";
    cad += "<table><tr><td>";      
    cad += "<form action=\"creaFichero\" target=\"_self\">";
    cad += "  <p>";
    cad += "    Nombre:<input type=\"text\" name=\"nombre\" value=\"\">";
    cad += "    <BR>";
    cad += "    Contenido:<br><textarea cols=75 rows=20 name=\"contenido\"></textarea>";
    cad += "    <BR>";
    cad += "    <input type=\"submit\" value=\"salvar\">";
    cad += "  </p>";
    cad += "</td></tr></table>";      
    }
  else cad += "<TR><TD>No se pudo recuperar la lista de ficheros</TD></TR>"; 

  cad += pieHTML;
  server.send(200, "text/html", cad); 
  }
 
