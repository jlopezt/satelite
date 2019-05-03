/************************************************/
/*                                              */
/* Funciones para la gestion de ficheros en     */
/* memoria del modulo esp6288                   */
/*                                              */
/************************************************/


/************************************************/
/* Recupera los datos de configuracion          */
/* de un archivo cualquiera                     */
/************************************************/
boolean leeFichero(String nombre, String &contenido)
  {
  boolean leido=false;
  
  Serial.println("Inicio de lectura de fichero " + nombre);

  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("------------------------------mounted file system---------------------------------");
    if (SPIFFS.exists(nombre)) 
      {
      //file exists, reading and loading
      Serial.printf("Encontrado fichero de configuracion %s.\n",nombre.c_str());
      File configFile = SPIFFS.open(nombre, "r");
      if (configFile) 
        {
        Serial.printf("Abierto fichero de configuracion %s.\n",configFile.name());
        size_t size = configFile.size();

        // Allocate a buffer to store contents of the file.
        char *buff=NULL;
        buff=(char *)malloc(size+1);

        configFile.readBytes(buff, size);
        buff[size]=0;//pongo el fin de cadena
        
        contenido=String(buff);
        Serial.printf("Contenido del fichero: #%s#\n",contenido.c_str());
        free(buff);
        leido=true;
        
        configFile.close();//cierro el fichero
        Serial.println("Cierro el fichero");
        }//la de abrir el fichero de configuracion del WiFi
        else Serial.println("Fichero no encontrado");
      }//la de existe fichero
    Serial.println("---------------------------------------unmounted file system------------------------");
    }//La de abrir el sistema de ficheros

  return leido;
  }
/**********************************************************************/
/* Salva la cadena pasada al fichero especificado                     */
/* Si ya existe lo sobreescribe                                       */
/**********************************************************************/  
boolean salvaFichero(String nombreFichero, String nombreFicheroBak, String contenidoFichero)
  {
  boolean salvado=false;
    
  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("-------------------------------mounted file system--------------------------------");
  
    //file exists, reading and loading
    if(SPIFFS.exists(nombreFichero)) 
      {
      if(nombreFicheroBak!="")
        {
        Serial.printf("El fichero %s ya existe, sera copiara con el nombre %s.\n",nombreFichero.c_str(),nombreFicheroBak.c_str());
          
        if(SPIFFS.exists(nombreFicheroBak.c_str())) SPIFFS.remove(nombreFicheroBak.c_str());  
        SPIFFS.rename(nombreFichero.c_str(),nombreFicheroBak.c_str());  
        }
      else Serial.printf("El fichero %s ya existe, sera sobreescrito.\n",nombreFichero.c_str());
      }
    
    File newFile = SPIFFS.open(nombreFichero, "w");//abro el fichero, si existe lo borra
    if (newFile) 
      {
      Serial.printf("Abierto fichero %s.\nGuardo contenido:\n#%s#\n",newFile.name(),contenidoFichero.c_str());
  
      newFile.print(contenidoFichero);
      newFile.close();//cierro el fichero
      Serial.println("Cierro el fichero");
      salvado=true;
      }
    else Serial.println("El fichero no se pudo abrir para escritura.\n");
  
    Serial.println("------------------------------unmounted file system---------------------------------");
    }//La de abrir el sistema de ficheros
  return salvado;
  }


/****************************************************/
/* Borra el fichero especificado                    */
/****************************************************/  
boolean borraFichero(String nombreFichero)
  {
  boolean borrado=false;
  
  //inicializo el sistema de ficheros
  if (SPIFFS.begin()) 
    {
    Serial.println("--------------------------------------mounted file system-------------------------\n");  
    //file exists, reading and loading
    if(!SPIFFS.exists(nombreFichero)) Serial.println("El fichero " + nombreFichero + " no existe.");
    else
      {
      if (SPIFFS.remove(nombreFichero)) 
        {
        borrado=true;
        Serial.println("El fichero " + nombreFichero + " ha sido borrado.");
        }
      else Serial.println("No se pudo borrar el fichero " + nombreFichero + ".");
      }  
    Serial.println("--------------------------------unmounted file system-------------------------------");
    }//La de abrir el sistema de ficheros  
  return borrado;
  }  
