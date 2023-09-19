//Establece el modelo del modulo GPRS a utilizar
#define TINY_GSM_MODEM_SIM900
// Incluimos librerías DHT11 
#include <DHT.h>
#include <DHT_U.h>
//Para emular la comunicación serial en otros pines digitales
#include <SoftwareSerial.h>
//Para uso de JSON
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
//Para facilitar uso de GPRS
#include <TinyGsmClient.h>
#include <PubSubClient.h> 

// Definimos el pin digital donde se conecta el sensor "DHT11"
#define DHTPIN 5
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11

//Variables globales

DHT dht(DHTPIN, DHTTYPE); // Inicializamos el sensor DHT11

//Se crea objeto de comunicacion serial para comunicarse con SIM800L
//SIM800L TX y RX estan conectados a los pines 3 y 2 de Arduino Uno
SoftwareSerial serialSIM(3, 2);

//Configuracion de APN (Access Point Name) de la red de servicio movil(En este caso, Tuenti)
const char* apn = "internet.movil"; // Configura el APN de tu proveedor de servicios móviles
const char* gprsUser = "internet"; // Usuario del APN por defecto de Tuenti
const char* gprsPass = "internet"; // Contraseña del APN por defecto de Tuenti
const char* gsmPin = "TuPIN"; // Si tu tarjeta SIM tiene un PIN, configúralo aquí

const char* mqttServer = "163.10.3.73";
const int mqttPort = 8083;
const char* mqttUser = "taller2g2";
const char* mqttPassword = "taller2g2";

TinyGsm modem(serialSIM);
TinyGsmClient client(modem);

PubSubClient mqtt(client);

void setupDHT11(){
  // Comenzamos el sensor DHT
  dht.begin();
}
 
void setup() {
 // Inicializamos comunicación serie
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  serialSIM.begin(9600);
  Serial.println("Inicializando sensor y módulo GSM"); //Println para debug
  //Inicializacion sensor de temperatura 
  setupDHT11();

  // Conecta al módulo SIM800L
  if (!modem.init()) {
    Serial.println("Error al iniciar el módulo GSM");
    while (true);
  }

  // Desbloquea la tarjeta SIM con PIN si es necesario
  // if (gsmPin && modem.simUnlock(gsmPin) != GSM_SUCCESS) {
  //   Serial.println("Error al desbloquear la tarjeta SIM");
  //   while (true);
  // }

  // Conéctate a la red GPRS utilizando el APN
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("Error al conectarse a la red GPRS");
    while (true);
  }

  // Configura el servidor MQTT
  mqtt.setServer(mqttServer, mqttPort);
  Serial.println("El programa se inicializo correctamente"); //Println para debug
}

void loop() {
  if (!mqtt.connected()) {
    reconnect(); //Reconecto si se pierde la señal
  }
   //Leemos los datos del sensor DHT11.
  float temp = dht.readTemperature(); // Leemos la temperatura en grados centigrados (por defecto)
  // Comprobamos si ha habido algun error en la lectura
  if (!(isnan(temp))) { //Si no hubo error al obtener los datos del sensor DHT11.
    // Serial.print("Temperatura: ");
    // Serial.print(temp);
    // Serial.print(" *C ");
    // Serial.print("\n");

     // Crear un objeto JSON
    DynamicJsonDocument jsonDoc(200);
    jsonDoc["temperatura"] = temp;
    char jsonBuffer[200];
    serializeJson(jsonDoc, jsonBuffer);

    // Publicar el JSON como mensaje MQTT
    mqtt.publish("sensor/temperatura", jsonBuffer);
  }
  else{
    // Si hay un error al leer la temperatura, enviar un mensaje de error en formato JSON.
    DynamicJsonDocument errorDoc(200);
    errorDoc["error"] = "Error al leer temperatura";
    char errorBuffer[200];
    serializeJson(errorDoc, errorBuffer);
    mqtt.publish("sensor/error", errorBuffer);
  }
  delay(5000); // Esperamos 5 segundos entre medidas
}

void reconnect() { //Modulo para reconexion
  while (!mqtt.connected()) {
    Serial.println("Conectando al servidor MQTT...");
    if (mqtt.connect("arduinoClient", mqttUser, mqttPassword)) {
      Serial.println("Conectado al servidor MQTT");
    } else {
      Serial.print("Error de conexión MQTT, rc=");
      Serial.print(mqtt.state());
      Serial.println(" Intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}