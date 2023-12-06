//Para uso de JSON
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
// Incluimos librerías DHT11 
#include <DHT.h>
#include <DHT_U.h>
// modem:
#define TINY_GSM_MODEM_SIM900

// Configurar serial para la consola de debug
#define SerialMon Serial

//Para emular la comunicación serial en otros pines digitales
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3); // RX, TX

// Configuracion de configuracion para Internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// Configuracion del GSM PIN
#define GSM_PIN "1234"

//Pin del LED integrado de Arduino
#define LED_PIN 13
//Para controlar el estado del LED
int ledStatus = LOW;

//Configuracion de APN (Access Point Name) de la red de servicio movil(En este caso, Tuenti)
const char apn[] = "internet.movil";
const char gprsUser[] = "internet";
const char gprsPass[] = "internet";

// Direccion del servidor MQTT
const char* broker = "163.10.3.73";
const char* topicTemp = "Arduino/temp";
const char* topicControl = "Arduino/control";

#include <TinyGsmClient.h>
#include <PubSubClient.h>

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

// Definimos el pin digital donde se conecta el sensor "DHT11"
#define DHTPIN 5
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); // Inicializamos el sensor DHT11

unsigned long lastReconnectAttempt = 0;
unsigned long lastPing = 0;
unsigned long t;

// Maneja los mensajes MQTT recibidos aquí
void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Mensaje recibido [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Solo procede si el el mensaje recibido corresponde a un topic en particular
  if (String(topic) == topicControl) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
  }
}

boolean mqttConnect() {
  SerialMon.print("Conectando con ");
  SerialMon.print(broker);

  // Conexion al MQTT Broker
  boolean status = mqtt.connect("arduinoClient", "taller2g2", "taller2g2");

  if (status == false) {
    SerialMon.println(" error");
    return false;
  }
  SerialMon.println(" exito");
  mqtt.subscribe(topicControl);
  return mqtt.connected();
}


void setup() {
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  pinMode(LED_PIN, OUTPUT);

  SerialMon.println("Iniciando sensor...");
  //Inicializacion sensor de temperatura 
  dht.begin();

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif

  // Restart toma algo de tiempo, para evitarlo, utilizar init() en vez de restart()
  SerialMon.println("Inicializando modem...");
  //modem.restart();
  modem.init();

#if TINY_GSM_USE_GPRS
  // Unlock your SIM card with a PIN if needed
  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
#endif

  SerialMon.print("Conectando a la red...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" error");
    delay(10000);
    return;
  }
  SerialMon.println(" exito");

#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    SerialMon.print("Conectando a ");
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      SerialMon.println(" error");
      delay(10000);
      return;
    }
    SerialMon.println(" exito");

#endif

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
}

void loop() {
  t = millis();
  // Nos aseguramos que seguimos registrados en la red
  if (!modem.isNetworkConnected()) {
    SerialMon.println("Desconectado de la red.");
    if (!modem.waitForNetwork(180000L, true)) {
      SerialMon.println("Error en la conexion.");
      delay(10000);
      return;
    }
    if (modem.isNetworkConnected()) {
      SerialMon.println("Conexion reestablecida.");
    }

#if TINY_GSM_USE_GPRS
    // Nos asguramos que seguimos conectados a la red GPRS
    if (!modem.isGprsConnected()) {
      SerialMon.println("Desconectado de red GPRS");
      SerialMon.print(F("Conectando a "));
      SerialMon.print(apn);
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" error");
        delay(10000);
        return;
      }
      if (modem.isGprsConnected()) { SerialMon.println("Conexion GPRS reestablecida."); }
    }
#endif
  }
  if (!mqtt.connected()) {
    SerialMon.println("No conectado al Broker");
    // Reconectar cada 10 segundos
    if (t - lastReconnectAttempt > 10000L) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  } else if (t - lastPing > 10000L) {
    lastPing = t;
    //Crear un objeto JSON
    DynamicJsonDocument jsonDoc(32);
    float temp = dht.readTemperature(); // Leemos la temperatura en grados centigrados (por defecto)
    if (!(isnan(temp))) { //Si no hubo error al obtener los datos del sensor DHT11.
      jsonDoc["temp"] = temp;
    }
    else{
      jsonDoc["temp"] = "error";
    }
    char jsonBuffer[32];
    serializeJson(jsonDoc, jsonBuffer);
    mqtt.publish(topicTemp, jsonBuffer);
    t = millis();
  }
  mqtt.loop();
}