/*
 * ordenes.ino
 *
 * Comandos para la actualizacion de la hora del reloj
 *
 * Permite la puesta en hora del reloj a traves de comandos enviados por el puesto
 * serie desde el PC.
 *
 * Para actualizar la hora <comado> <valor>; Ejemplo: "hora 3;"
 * Se pueden anidar: "hora 2;minuto 33;"
 *
 */

#define LONG_COMANDO 40
#define LONG_PARAMETRO 30
#define LONG_ORDEN 22 //Comando (espacio) Parametros (fin de cadena)
#define MAX_COMANDOS   35

#include <Time.h>

char ordenRecibida[LONG_ORDEN]="";
int lonOrden=0;

typedef struct 
  {
  String comando;
  String descripcion;
  void (*p_func_comando) (int, char*, float)=NULL;
  }tipo_comando;
tipo_comando comandos[MAX_COMANDOS];

int HayOrdenes(int debug)
  {
  char inChar=0;
  
  while (Serial.available())
    {
    inChar=(char)Serial.read(); 
    switch (inChar)
      {
      case ';':
        //Recibido final de orden
        if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
        return(1);
        break;
      default:
        //Nuevo caracter recibido. Añado y sigo esperando
        ordenRecibida[lonOrden++]=inChar;
        ordenRecibida[lonOrden]=0;
        break;
      }
    }  
  return(0); //No ha llegado el final de orden
  }

int EjecutaOrdenes(int debug){
  String comando="";
  String parametros="";
  int iParametro=0;
  char sParametro[LONG_PARAMETRO]="";//LONG_PARAMETRO longitud maxmima del parametro
  float fParametro;
  int inicioParametro=0;

  if (debug) Serial.printf("Orden recibida: %s\n",ordenRecibida);
  
  for(int i=0;i<LONG_COMANDO;i++)
    {
    switch (ordenRecibida[i])
      {
      case ' ':
        //fin del comando, hay parametro
        inicioParametro=i+1;
        
        //Leo el parametro
        for (int j=0;j<LONG_ORDEN;j++)
          {  //Parsea la orden      
          if(ordenRecibida[j+inicioParametro]==0) 
            {
            strncpy(sParametro,ordenRecibida+inicioParametro,j+1);//copio el parametro como texto
            break;//j=LONG_ORDEN;//Cuando encuentro el final de la cadena
            }
          else iParametro=(iParametro*10)+(int)ordenRecibida[j+inicioParametro]-48; //hay que convertir de ASCII a decimal
          }
        fParametro=String(sParametro).toFloat();
        
        i=LONG_COMANDO;
        break;
      case 0:
        //fin de la orden. No hay parametro
        i=LONG_COMANDO;
        break;
      default:
        comando+=ordenRecibida[i];
        break;
      }
    }

  //Limpia las variables que3 recogen la orden
  lonOrden=0;
  ordenRecibida[0]=0;

  if (debug) Serial.printf("comando: %s\niParametro: %i\nsParametro: %s\nfParametro: %f\n",comando.c_str(),iParametro,sParametro,fParametro);
    
/**************Nueva funcion ***************************/
  int8_t indice=0;
  for(indice=0;indice<MAX_COMANDOS;indice++)
    {
    if (debug) Serial.printf("Comando[%i]: {%s} - {%s}\n",indice,comando.c_str(),comandos[indice].comando.c_str());

    if (comandos[indice].comando==comando) 
      {
      //Ejecuta la funcion asociada
      comandos[indice].p_func_comando(iParametro, sParametro, fParametro);
      return(0);
      }    
    }

  //Si llega aqui es que no ha encontrado el comando
  Serial.println("Comando no encontrado");
  return(-1);//Comando no encontrado  
/*******************************************************/
}

void limpiaOrden(void)
  {
  lonOrden=0;
  ordenRecibida[0]=0;
  }
  
void inicializaOrden(void)
  { 
  int i =0;  

  limpiaOrden();
  
  comandos[i].comando="help";
  comandos[i].descripcion="Listado de comandos";
  comandos[i++].p_func_comando=func_comando_help;
  
  comandos[i].comando="IP";
  comandos[i].descripcion="Direccion IP";
  comandos[i++].p_func_comando=func_comando_IP;
  
  comandos[i].comando="direccion";
  comandos[i].descripcion="Devuelve la direccion del satelite";
  comandos[i++].p_func_comando=func_comando_direccion;
  
  comandos[i].comando="restart";
  comandos[i].descripcion="Reinicia el modulo";
  comandos[i++].p_func_comando=func_comando_restart;
  
  comandos[i].comando="info";
  comandos[i].descripcion="Devuelve informacion del hardware";
  comandos[i++].p_func_comando=func_comando_info;
  
  comandos[i].comando="fexist";
  comandos[i].descripcion="Indica si existe un fichero en el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_fexist;
  
  comandos[i].comando="finfo";
  comandos[i].descripcion="Devuelve informacion del sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_finfo;
  
  comandos[i].comando="fopen";
  comandos[i].descripcion="Devuelve el contenido del fichero especificado";
  comandos[i++].p_func_comando=func_comando_fopen;
  
  comandos[i].comando="fremove";
  comandos[i].descripcion="Borra el fichero especificado";
  comandos[i++].p_func_comando=func_comando_fremove;
  
  comandos[i].comando="format";
  comandos[i].descripcion="Formatea el sistema de ficheros";
  comandos[i++].p_func_comando=func_comando_format;
  
  comandos[i].comando="tem";
  comandos[i].descripcion="Devuelve la temperatura medida";
  comandos[i++].p_func_comando=func_comando_tem;
  
  comandos[i].comando="hum";
  comandos[i].descripcion="Devuelve la humedad medida";
  comandos[i++].p_func_comando=func_comando_hum;
    
  comandos[i].comando="luz";
  comandos[i].descripcion="Devuelve la luz medida";
  comandos[i++].p_func_comando=func_comando_luz;
      
  comandos[i].comando="hora";
  comandos[i].descripcion="Fija/consulta la hora del sistema";
  comandos[i++].p_func_comando=func_comando_hora;
      
  comandos[i].comando="minuto";
  comandos[i].descripcion="Fija/consulta los minutos del sistema";
  comandos[i++].p_func_comando=func_comando_minuto;
       
  comandos[i].comando="segundo";
  comandos[i].descripcion="Fija/consulta los segundos del sistema";
  comandos[i++].p_func_comando=func_comando_segundo;
  
  comandos[i].comando="echo";
  comandos[i].descripcion="Devuelve el eco del sistema";
  comandos[i++].p_func_comando=func_comando_echo;
   
  comandos[i].comando="debug";
  comandos[i].descripcion="Activa/desactiva el modo debug";
  comandos[i++].p_func_comando=func_comando_debug;

  comandos[i].comando="SSID";
  comandos[i].descripcion="Nombre de la base WiFi conectada";
  comandos[i++].p_func_comando=func_comando_getSSID;

  comandos[i].comando="contadores";
  comandos[i].descripcion="Numero de vueltas del bucle principal";
  comandos[i++].p_func_comando=func_comando_contadores;

  comandos[i].comando="mqtt";
  comandos[i].descripcion="Configuración de MQTT";
  comandos[i++].p_func_comando=func_comando_mqtt;

  comandos[i].comando="eraseConfig";
  comandos[i].descripcion="Borrado de configuracion en flash";
  comandos[i++].p_func_comando=func_comando_eraseConfig;

  //resto
  for(;i<MAX_COMANDOS;)
    {
    comandos[i].comando="vacio";
    comandos[i].descripcion="Comando vacio";
    comandos[i++].p_func_comando=func_comando_vacio;  
    }

  func_comando_help(0,"",0.0);
  //for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!=String("vacio")) Serial.printf("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  }


/*********************************************************************/
/*  Funciones para los comandos                                      */
/*  void (*p_func_comando) (int, char*, float)                       */
/*********************************************************************/  
void func_comando_vacio(int iParametro, char* sParametro, float fParametro) //"vacio"
{}
 
void func_comando_help(int iParametro, char* sParametro, float fParametro) //"help"
  {
  Serial.printf("\n\nComandos:");  
  for(int8_t i=0;i<MAX_COMANDOS;i++) if (comandos[i].comando!=String("vacio")) Serial.printf("Comando %i: [%s]\n",i, comandos[i].comando.c_str());
  Serial.printf("\n------------------------------------------------------------------------------\n");
  }

void func_comando_IP(int iParametro, char* sParametro, float fParametro) //"IP"
  {
  boolean debug=false;
  Serial.println(getIP(debug));  
  }  
  
void func_comando_direccion(int iParametro, char* sParametro, float fParametro) //"direccion"
  {
  if (sParametro[0]!=0)
    {
    String cad="";
    direccion=iParametro;
    Serial.printf("Nueva direccion %i.\n",direccion);
    if(!leeFichero(GLOBAL_CONFIG_FILE, cad)) Serial.println("No se pudo leer el fichero");
    cad=generaJsonConfiguracionId(cad, String(direccion));
    if(!salvaFichero(GLOBAL_CONFIG_FILE, GLOBAL_CONFIG_BAK_FILE, cad)) Serial.println("No se pudo salvar el fichero");  
    }
  else Serial.printf("La direccion del satelite es %i\n", direccion);
  }    

void func_comando_restart(int iParametro, char* sParametro, float fParametro)//"restart")
  {
  ESP.restart();
  }

void func_comando_info(int iParametro, char* sParametro, float fParametro)//"info")
  {
  Serial.printf("\n-----------------Hardware info-----------------\n");
  Serial.printf("Vcc: %i\n",ESP.getVcc());
  Serial.printf("FreeHeap: %i\n",ESP.getFreeHeap());
  Serial.printf("ChipId: %i\n",ESP.getChipId());
  Serial.printf("SdkVersion: %s\n",ESP.getSdkVersion());
  Serial.printf("CoreVersion: %s\n",ESP.getCoreVersion().c_str());
  Serial.printf("FullVersion: %s\n",ESP.getFullVersion().c_str());
  Serial.printf("BootVersion: %i\n",ESP.getBootVersion());
  Serial.printf("BootMode: %i\n",ESP.getBootMode());
  Serial.printf("CpuFreqMHz: %i\n",ESP.getCpuFreqMHz());
  Serial.printf("FlashChipId: %i\n",ESP.getFlashChipId());
      //gets the actual chip size based on the flash id
  Serial.printf("FlashChipRealSize: %i\n",ESP.getFlashChipRealSize());
      //gets the size of the flash as set by the compiler
  Serial.printf("FlashChipSize: %i\n",ESP.getFlashChipSize());
  Serial.printf("FlashChipSpeed: %i\n",ESP.getFlashChipSpeed());
      //FlashMode_t ESP.getFlashChipMode());
  Serial.printf("FlashChipSizeByChipId: %i\n",ESP.getFlashChipSizeByChipId());  
  Serial.printf("-----------------------------------------------\n");

  Serial.printf("\n-----------------Config info-----------------\n");
  Serial.printf("anchoLoop: %i\n",anchoLoop);
  Serial.printf("ahorroEnergia: %i\n",ahorroEnergia);
  Serial.printf("-----------------------------------------------\n");

  Serial.printf("\n-------------Config contadores---------------\n");
  Serial.printf("multiplicadorAnchoIntervalo= %i\n",multiplicadorAnchoIntervalo);
  Serial.printf("anchoIntervalo= %i\n",anchoIntervalo);
  Serial.printf("frecuenciaOTA= %i\n",frecuenciaOTA);
  Serial.printf("frecuenciaLeeSensores= %i\n",frecuenciaLeeSensores);
  Serial.printf("frecuenciaServidorWeb= %i\n",frecuenciaServidorWeb);
  Serial.printf("frecuenciaOrdenes= %i\n",frecuenciaOrdenes);
  Serial.printf("frecuenciaMQTT= %i\n",frecuenciaMQTT);
  Serial.printf("frecuenciaWifiWatchdog= %i\n",frecuenciaWifiWatchdog);
  Serial.printf("-----------------------------------------------\n");  
  }  

void func_comando_fexist(int iParametro, char* sParametro, float fParametro)//"fexist")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    if(SPIFFS.exists(sParametro)) Serial.printf("El fichero %s existe.\n",sParametro);
    else Serial.printf("NO existe el fichero %s.\n",sParametro);
    }
  }

void func_comando_finfo(int iParametro, char* sParametro, float fParametro)//"finfo")
  {
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
    Serial.printf("totalBytes: %i\nusedBytes: %i\nblockSize: %i\npageSize: %i\nmaxOpenFiles: %i\nmaxPathLength: %i\n",fs_info.totalBytes, fs_info.usedBytes ,fs_info.blockSize ,fs_info.pageSize ,fs_info.maxOpenFiles ,fs_info.maxPathLength);
    }
  else Serial.println("Error al leer info");  
  }

void func_comando_fopen(int iParametro, char* sParametro, float fParametro)//"fopen")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    {
    File f = SPIFFS.open(sParametro, "r");
    if (f)
      { 
      Serial.println("Fichero abierto");
      size_t tamano_fichero=f.size();
      Serial.printf("El fichero tiene un tamaño de %i bytes.\n",tamano_fichero);
      char buff[tamano_fichero+1];
      f.readBytes(buff,tamano_fichero);
      buff[tamano_fichero+1]=0;
      Serial.printf("El contenido del fichero es:\n******************************************\n%s\n******************************************\n",buff);
      f.close();
      }
    else Serial.printf("Error al abrir el fichero %s.\n", sParametro);
    } 
  } 

void func_comando_fremove(int iParametro, char* sParametro, float fParametro)//"fremove")
  {
  if (sParametro=="") Serial.println("Es necesario indicar un nombre de fichero");
  else
    { 
    if (SPIFFS.remove(sParametro)) Serial.printf("Fichero %s borrado\n",sParametro);
    else Serial.printf("Error al borrar el fichero%s\n",sParametro);
    } 
 }

void func_comando_format(int iParametro, char* sParametro, float fParametro)//"format")
  {     
  if (SPIFFS.format()) Serial.println("Sistema de ficheros formateado");
  else Serial.println("Error al formatear el sistema de ficheros");
  } 


void func_comando_tem(int iParametro, char* sParametro, float fParametro)//"tem"
    {
    Serial.println(String(getTemperatura(),1));
    }

void func_comando_hum(int iParametro, char* sParametro, float fParametro)//"hum"  
    {
    Serial.println(String(getHumedad(),1));
    }
  
void func_comando_luz(int iParametro, char* sParametro, float fParametro)//"luz"
    {
    Serial.println(String(getLuz(),1));
    }        

void func_comando_hora(int iParametro, char* sParametro, float fParametro)//"hora"    
  {
  //Tomo la hora actual
  time_t ahora=now();
  int hora=hour(ahora); 
  int minuto=minute(ahora);
  int segundo=second(ahora);
  int dia=day(ahora);
  int mes=month(ahora);
  int anno=year(ahora);

  if (sParametro[0]!=0) setTime(iParametro, minuto,segundo,dia, mes, anno); //Aplico la nueva hora
  else Serial.printf("La hora es %i\n",hora);
  }
  
void func_comando_minuto(int iParametro, char* sParametro, float fParametro)//"minuto"    
  {
  //Tomo la hora actual
  time_t ahora=now();
  int hora=hour(ahora); 
  int minuto=minute(ahora);
  int segundo=second(ahora);
  int dia=day(ahora);
  int mes=month(ahora);
  int anno=year(ahora);

  if (sParametro[0]!=0) setTime(hora, iParametro,segundo,dia, mes, anno); //Aplico la nueva hora
  else Serial.printf("Los minutos son %i\n",minuto);
  }
  
void func_comando_segundo(int iParametro, char* sParametro, float fParametro)//"segundo"
  {
  //Tomo la hora actual
  time_t ahora=now();
  int hora=hour(ahora); 
  int minuto=minute(ahora);
  int segundo=second(ahora);
  int dia=day(ahora);
  int mes=month(ahora);
  int anno=year(ahora);

  if (sParametro[0]!=0) setTime(hora, minuto,iParametro,dia, mes, anno); //Aplico la nueva hora
  else Serial.printf("Los segundos soh %i\n",segundo);
  }

void func_comando_echo(int iParametro, char* sParametro, float fParametro)//"echo") 
  {
  Serial.printf("echo; %s\n",sParametro);
  }

void func_comando_debug(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  ++debugGlobal=debugGlobal % 2;
  if (debugGlobal) Serial.println("debugGlobal esta on");
  else Serial.println("debugGlobal esta off");
  }

void func_comando_getSSID(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Red WiFi: %s\nPotencia: %i\n", nombreSSID().c_str(),WiFi.RSSI());  
  }

void func_comando_contadores(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("vueltas= %i\n",vuelta);  
  Serial.printf("anchoLoop= %i\n",anchoLoop);
  Serial.printf("multiplicadorAnchoIntervalo= %i\n",multiplicadorAnchoIntervalo);
  Serial.printf("anchoIntervalo= %i\n",anchoIntervalo);
  Serial.printf("frecuenciaOTA= %i | %i\n",frecuenciaOTA,vuelta%frecuenciaOTA);
  Serial.printf("frecuenciaLeeSensores= %i | %i\n",frecuenciaLeeSensores,vuelta%frecuenciaLeeSensores);
  Serial.printf("frecuenciaServidorWeb= %i | %i\n",frecuenciaServidorWeb,vuelta%frecuenciaServidorWeb);
  Serial.printf("frecuenciaOrdenes= %i | %i\n",frecuenciaOrdenes,vuelta%frecuenciaOrdenes);
  Serial.printf("frecuenciaMQTT= %i | %i\n",frecuenciaMQTT,vuelta%frecuenciaMQTT);
  Serial.printf("frecuenciaEnviaDatos= %i | %i\n",frecuenciaEnviaDatos,vuelta%frecuenciaEnviaDatos);  
  Serial.printf("frecuenciaWifiWatchdog= %i | %i\n",frecuenciaWifiWatchdog,vuelta%frecuenciaWifiWatchdog);
  } 

void func_comando_mqtt(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  Serial.printf("Configuracion leida:\nID MQTT: %s\nIP broker: %s\nIP Puerto del broker: %i\ntimeReconnectMQTT: %i\nUsuario: %s\nPassword: %s\nTopic root: %s\nWill topic: %s\nWill msg: %s\nClean session: %i\n", ID_MQTT.c_str(), IPBroker.toString().c_str(), puertoBroker, timeReconnectMQTT, usuarioMQTT.c_str(),passwordMQTT.c_str(),topicRoot.c_str(),(topicRoot+"/"+String(WILL_TOPIC)).c_str(),String(WILL_MSG).c_str(), CLEAN_SESSION);
  }  

void func_comando_eraseConfig(int iParametro, char* sParametro, float fParametro)//"debug")
  {
  if(ESP.eraseConfig()) Serial.printf("Configuracion borrada correctamente.\n");
  else Serial.printf("Error al borrar la configuracion.\n");
  }  
  
/**********************************************************************/
/* Salva la configuracion general en formato json                     */
/**********************************************************************/  
String generaJsonConfiguracionId(String configActual, String id)
  {
  boolean nuevo=true;
  String salida="";

  if(configActual=="") 
    {
    Serial.println("No existe el fichero. Se genera uno nuevo");
    return "{\"id\": \"" + id + "\", \"ahorroEnergia\": 0,\"multiplicadorAnchoIntervalo\": 5,\"anchoIntervalo\": 100,\"frecuenciaOTA\": 5,\"frecuenciaLeeSensores\": 50,\"frecuenciaServidorWeb\": 1,\"frecuenciaOrdenes\": 2,\"frecuenciaMQTT\": 50,\"frecuenciaEnviaDatos\": 100, \"frecuenciaWifiWatchdog\": 100 }";
    }
    
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configActual.c_str());
  json.printTo(Serial);
  if (json.success()) 
    {
    Serial.println("parsed json");          

//******************************Parte especifica del json a leer********************************
    json["id"]=direccion;
//************************************************************************************************

    json.printTo(salida);//pinto el json que he creado
    Serial.printf("json creado:\n#%s#\n",salida.c_str());
    }//la de parsear el json

  return salida;  
  }  
  
