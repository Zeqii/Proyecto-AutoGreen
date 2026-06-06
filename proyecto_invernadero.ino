/*
  Sistema de monotoreo de Invernadero
  Placa: ESP32-C3 Super Mini
  ALumnos: Brandon Ezequias Nij Lopez, 6590-23-19976 - bnijl@miumg.edu.gt
           Eddin Fernando Nij López, 6590-23-19988 - enijl1@miumg.edu.gt
  GPIO0 -> Humedad suelo
  GPIO1 -> LDR
  GPIO2 -> Relé ventilador
  GPIO3 -> Relé bomba
  GPIO4 -> DHT11

  GPIO5 -> LED Luz auxiliar
  GPIO6 -> LED que hace que simule calefccion

  GPIO8 -> SDA LCD
  GPIO9 -> SCL LCD

  IMPORTANTE:
  Tools -> para poder ver los datos en el monitor en serie se usa USB CDC On Boot -> Enabled
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

//=====================
// LCD
//=====================
LiquidCrystal_I2C LCD(0x27, 16, 2);

//=====================
// DHT11
//=====================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//=====================
// Sensores
//=====================
#define SOIL_PIN 0
#define LDR_PIN 1

//=====================
// Actuadores
//=====================
#define FAN_RELAY 2
#define PUMP_RELAY 3

#define LIGHT_LED 5
#define HEATER_LED 6

unsigned long previousMillis = 0;
const unsigned long interval = 5000;

bool pantallaSensores = true;

void setup()
{
  Serial.begin(115200);

  dht.begin();
  pinMode(LDR_PIN, INPUT);

  pinMode(FAN_RELAY, OUTPUT);
  pinMode(PUMP_RELAY, OUTPUT);

  pinMode(LIGHT_LED, OUTPUT);
  pinMode(HEATER_LED, OUTPUT);

  digitalWrite(FAN_RELAY, HIGH);
  digitalWrite(PUMP_RELAY, HIGH);

  digitalWrite(LIGHT_LED, LOW);
  digitalWrite(HEATER_LED, LOW);

  LCD.init();
  LCD.backlight();

  LCD.setCursor(0, 0);
  LCD.print("Invernadero");
  LCD.setCursor(0, 1);
  LCD.print("Iniciando...");

  delay(2000);
  LCD.clear();

  Serial.println("Sistema iniciado");
}

void loop()
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    //=====================
    // Lecturas
    //=====================

    float temperatura = dht.readTemperature();
    float humedad = dht.readHumidity();

    if (isnan(temperatura) || isnan(humedad))
    {
      Serial.println("Error leyendo DHT11");
      return;
    }

    int soilRaw = analogRead(SOIL_PIN);
    int ldrRaw = analogRead(LDR_PIN);

    // este sensor se  calibró de acuerdo a los datos obtenidos
    int humedadSuelo = map(soilRaw, 4095, 2400, 0, 100);

    // Aproximación de lux
    int lux = map(ldrRaw, 300, 3800, 0, 500);

    bool ventilador = false;
    bool bomba = false;
    bool luzAuxiliar = false;
    bool calefactor = false;

    //=====================
    // Ventilador
    //=====================

    if (temperatura > 30)
    {
      digitalWrite(FAN_RELAY, LOW);
      ventilador = true;
    }
    else
    {
      digitalWrite(FAN_RELAY, HIGH);
    }

    //=====================
    // Bomba
    //=====================

    if (humedadSuelo < 40)
    {
      digitalWrite(PUMP_RELAY, LOW);
      bomba = true;
    }
    else
    {
      digitalWrite(PUMP_RELAY, HIGH);
    }

    //=====================
    // Luz auxiliar
    //=====================

    if (lux < 200)
    {
      digitalWrite(LIGHT_LED, HIGH);
      luzAuxiliar = true;
    }
    else
    {
      digitalWrite(LIGHT_LED, LOW);
    }

    //=====================
    // Calefactor
    //=====================

    if (temperatura < 29)
    {
      digitalWrite(HEATER_LED, HIGH);
      calefactor = true;
    }
    else
    {
      digitalWrite(HEATER_LED, LOW);
    }

    //=====================
    // LCD
    //=====================

    LCD.clear();

    if (pantallaSensores)
    {
      LCD.setCursor(0, 0);
      LCD.print("T:");
      LCD.print(temperatura, 1);
      LCD.print(" H:");
      LCD.print((int)humedad);

      LCD.setCursor(0, 1);
      LCD.print("S:");
      LCD.print(humedadSuelo);
      LCD.print("% L:");
      LCD.print(lux);
    }
    else
    {
      LCD.setCursor(0, 0);
      LCD.print("Vent:");
      LCD.print(ventilador ? "ON " : "OFF");

      LCD.setCursor(0, 1);
      LCD.print("Riego:");
      LCD.print(bomba ? "ON " : "OFF");
    }

    pantallaSensores = !pantallaSensores;

    //=====================
    // Serial Monitor
    //=====================

    Serial.println("========== INVERNADERO ==========");

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Humedad Ambiente: ");
    Serial.print(humedad);
    Serial.println(" %");

    Serial.print("Humedad Suelo: ");
    Serial.print(humedadSuelo);
    Serial.println(" %");

    Serial.print("Luminosidad: ");
    Serial.print(lux);
    Serial.println(" lux");

    Serial.print("Ventilador: ");
    Serial.println(ventilador ? "ACTIVO" : "APAGADO");

    Serial.print("Bomba: ");
    Serial.println(bomba ? "ACTIVA" : "APAGADA");

    Serial.print("Luz Auxiliar: ");
    Serial.println(luzAuxiliar ? "ENCENDIDA" : "APAGADA");

    Serial.print("Calefactor: ");
    Serial.println(calefactor ? "ENCENDIDO" : "APAGADO");

    Serial.println("=================================");
    Serial.println();
  }
}
