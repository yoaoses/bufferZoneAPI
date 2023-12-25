#include <ESP8266WiFi.h>

//control temperatura interno
#include <OneWire.h>
#include <DallasTemperature.h>
//---------------------------

//constantes wi-fi
const char* ssid = "BufferZone";  // Nombre de tu red Wi-Fi
const char* password = "BufferZone_prototype01";  // Clave de tu red Wi-Fi
const char* deviceName = "placaPrototipo_01";  // Nombre del dispositivo para identificarlo en el router
const int maxAttempts = 10;  // Número máximo de intentos de conexión
//----------------

//constantes control interno
#define SENSOR_T_INTERNO D4 // pin D0 digital
OneWire myWire(SENSOR_T_INTERNO);
DallasTemperature sensor(&myWire);
const int internalFan=D1;
//--------------------------------------
//sonda
const int pinSonda = A0; // Pin analógico al que está conectada la sonda (cambia según tu configuración)
//-----
//globales
unsigned long interval = 300000;//tiempo
float lastMesureC=0;
float lastMesureF=0;
float internalTemp=0;
float lastInternalTemp=-1000;
//--------
void setup() {  
  pinMode(internalFan,OUTPUT);
  pinMode(pinSonda,INPUT);
  Serial.begin(115200);
  delay(100);

  connectToWiFi();
  
  // Otras inicializaciones o configuraciones pueden ir aquí
}

void loop() {
  // Ejecutar funciónSonda() cada 5 a 10 minutos
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  internalTemp=tempControll();
  Serial.println("");
  Serial.print("internal t|°C=>");
  Serial.print(internalTemp,1);
  if(internalTemp<30){
    digitalWrite(internalFan,LOW);
  }else{
    digitalWrite(internalFan,HIGH);
  }
  Serial.println("");
  if(WiFi.status() != WL_CONNECTED){
      Serial.println("Reconectando");
      connectToWiFi(); // Reconectar en cada ejecución del loop
  }

  if(round(internalTemp)!=round(lastInternalTemp)){
    updateNodeState();
  }
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    funcionSonda();
    delay(1000);
    uploadLatestSensorData();
  }
  
  delay(30000);
}

void connectToWiFi() {
  int attempts = 0;
  
  Serial.println("");
  Serial.println("Conectando a la red Wi-Fi...");
  WiFi.mode(WIFI_STA);

  // Inicia la conexión a la red Wi-Fi con el nombre (SSID) y la contraseña proporcionados
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Conexión exitosa");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Dirección MAC: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("");
    Serial.println("No se pudo conectar a la red Wi-Fi");
  }
}

float tempControll(){
  float temp;
  sensor.requestTemperatures();
  temp=sensor.getTempCByIndex(0);
  return temp;
}

void uploadLatestSensorData(){
  Serial.println("subiendo datos de control");
  Serial.println("  <--Sonda-->");
  Serial.print("      Temperatura ºC: ");
  Serial.println(lastMesureC);
  Serial.print("      Temperatura ºF: ");
  Serial.println(lastMesureF);
  Serial.println("  <--  ...  -->");
}

void updateNodeState(){
  Serial.println("subiendo estado del Nodo");
  Serial.println("  <--Controlador-->");
  Serial.print("      Id Nodo: ");
  Serial.println(deviceName);
  Serial.print("      Temperatura interna: ");
  Serial.println(internalTemp);
  Serial.print("      Estado ventilador: ");
  Serial.println(digitalRead(internalFan)==1?true:false);
  Serial.println("  <--  ...  -->");
  lastInternalTemp=internalTemp;
}

void disconnectFromWiFi() {
  Serial.println("Desconectando de la red Wi-Fi...");
  WiFi.disconnect(true); // true forzará la desconexión y la limpieza de la configuración
}

void funcionSonda() {
  const int muestras = 17; // Número de muestras a promediar +2(primeras 2 pasadas son para calentar y estabilizar la sonda)
  float sumaLecturas = 0.0; // Variable para almacenar la suma de la muestra

  for (int i = 0; i < muestras; i++) {
    int lectura = analogRead(pinSonda);//Activa la sonda
    if(i>1){//Primeras 2 pasadas son para estabilizar temperatura de la sonda 
      Serial.print(lectura);
      Serial.print(", ");
      sumaLecturas += lectura; //Suma voltaje obtenido
    }else{
      Serial.println("calentando sonda");
    }
    delay(1000); //Pausa entre lecturas
  }

  float promedioLecturas = sumaLecturas / muestras; // Calcula el promedio de los voltajes
  lastMesureC = promedioLecturas * 0.05545460981;//ponderacion calculada
  lastMesureF = (lastMesureC * 9.0 / 5.0) + 32.0;
  /*
  Serial.println("");
  Serial.print("Temperatura (C): ");
  Serial.println(lastMesureC,2); // Muestra la temperatura promedio en grados Celsius con 2 decimales
  Serial.print("Temperatura (F): ");
  Serial.println(lastMesureF,2); // Muestra la temperatura promedio en grados Fahrenheit con 2 decimales
  */
}




  // Otras funciones y lógica de tu programa van aquí

  // Ejemplo: Desconexión después de cierta lógica
  // if (alguna_condicion) {
  //   disconnectFromWiFi();
  // }

