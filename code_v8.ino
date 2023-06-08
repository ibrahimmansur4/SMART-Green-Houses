#include <ESP8266WiFi.h>
#include <DHT.h>    //humidity and temperature sensor
#include <Servo.h>
#include <ThingSpeak.h>

//defining pins & values for soil sensor
#define d_soilMositure D1  //powering the sensor
#define a_soilMositure A0 //analogue reading
#define soilWet 500   // Define max value we consider soil 'wet'
#define soilDry 750   // Define min value we consider soil 'dry'



//humidifier pins and values
#define humidifier D3 //relay

//defining pins for outputs
#define r_waterpump D8 // for water pump
#define bulb D5 //relay for light bulb 

//fans & hair dryer
#define outletFan D7
#define inletFan D6
#define hairDryer D4

//humidity and temp
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;

long myChannelNumber = 2137729;
const char myWriteAPIKey[] = "D7TM5296HKQQ8GZO";
long myChannelNumber2 = 2148454;
const char myWriteAPIKey2[] = "HR8GXDJQOTW300N5";
long myChannelNumber3 = 2148478;
const char myWriteAPIKey3[] = "32Q4P9UPTAFRGQH9";


int temp = 33;
int hum = 26;
float humedad;
float temperatura;
int moisture;
bool action_r1;
bool outlet;
bool inlet;
bool dryer;
bool light;
bool humidity;

bool soilMoisture();
int readSensor();
void waterPump();
void fans();

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(d_soilMositure, OUTPUT);
  pinMode(a_soilMositure, INPUT);
  pinMode(r_waterpump, OUTPUT);
  pinMode(humidifier, OUTPUT);
  pinMode(bulb, OUTPUT);
  pinMode(outletFan, OUTPUT);
  pinMode(inletFan, OUTPUT);
  pinMode(hairDryer, OUTPUT);

  WiFi.begin("HUAWEI P20 lite", "12345678");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print("..");
  }
  Serial.println();
  Serial.println("NodeMCU is connected!");
  Serial.println(WiFi.localIP());
  dht.begin();
  ThingSpeak.begin(client);

  // Initially keep the soil moisture sensor OFF
  digitalWrite(d_soilMositure, LOW);

  digitalWrite(bulb, HIGH); //initially light off

  digitalWrite(humidifier, LOW); //all time on

  //fans
  digitalWrite(outletFan, HIGH);    //off
  digitalWrite(inletFan, HIGH);    //off
  digitalWrite(hairDryer, HIGH);    //off

}

void loop() {
  // put your main code here, to run repeatedly:
  waterPump();
  fans();

}


//  This function returns the analog soil moisture measurement

int readSensor() {
  const unsigned long interval = 10;
  static unsigned long previousMillis = 0;

  digitalWrite(d_soilMositure, HIGH);  // Turn the sensor ON
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    int val = analogRead(a_soilMositure);  // Read the analog value from sensor
    digitalWrite(d_soilMositure, LOW);   // Turn the sensor OFF
    return val;             // Return analog moisture value
  }
  else {
    return -1; // return -1 if not enough time has passed
  }
}


bool soilMoisture() {
  unsigned long previousMillis = 0;  // variable to store the previous millis()
  const long interval = 1000;  // interval at which to take readings (in milliseconds)
  unsigned long currentMillis = millis();  // get the current millis()

  if (currentMillis - previousMillis >= interval) {  // check if it's time to take a reading
    previousMillis = currentMillis;  // save the current millis() for next comparison

    // get the reading from the function below and print it
    moisture = readSensor();
    ThingSpeak.writeField(myChannelNumber, 3, moisture, myWriteAPIKey);
    Serial.println("Moisture: ");
    Serial.println(moisture);
    // Determine status of our soil
    if (moisture < soilWet) {
      Serial.println("Status: Soil is too wet");
      return false;
    } else if (moisture >= soilWet && moisture < soilDry) {
      Serial.println("Status: Soil moisture is perfect");
      return false;
    } else {
      Serial.println("Status: Soil is too dry - time to water!");
      return true;
    }
  }
}

void waterPump() {
  action_r1 = soilMoisture();
  ThingSpeak.writeField(myChannelNumber, 7, action_r1, myWriteAPIKey);
  Serial.println("Water Pump ON/OFF: ");
  Serial.println(action_r1);
  if (action_r1 == false) {
    //off
    digitalWrite(r_waterpump, HIGH); //inverted logic for  relays
  }
  else {
    //on
    digitalWrite(r_waterpump, LOW);
  }
}


void fans() {
  // DHT sensor code start
  static unsigned long lastDHTreadTime = 0; // initialize last DHT read time to 0
  if (millis() - lastDHTreadTime > 500) { // if it's time to read DHT sensor
    lastDHTreadTime = millis(); // update last DHT read time
    humedad = dht.readHumidity();
    temperatura = dht.readTemperature();
    Serial.println("Temperature: ");
    Serial.println(temperatura);
    Serial.println("Humidity: ");
    Serial.println(humedad);
    if (isnan(humedad) || isnan(temperatura)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    ThingSpeak.writeField(myChannelNumber2, 1, temperatura, myWriteAPIKey2);
    ThingSpeak.writeField(myChannelNumber3, 1, humedad, myWriteAPIKey3);
  }



  if (humedad < 15) {
    digitalWrite(humidifier, LOW);
    digitalWrite(outletFan, HIGH); //off
    digitalWrite(inletFan, HIGH);
    digitalWrite(bulb, HIGH);
    digitalWrite(hairDryer, HIGH); //Off
    outlet = false;
    inlet = false;
    dryer = false;
    light = false;
    humidity = true;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
  }

  if (temperatura < 29) {
    digitalWrite(humidifier, HIGH);
    digitalWrite(outletFan, HIGH); //off
    digitalWrite(inletFan, HIGH);
    digitalWrite(bulb, HIGH);
    digitalWrite(hairDryer, LOW); //On
    outlet = false;
    inlet = false;
    dryer = true;
    light = false;
    humidity = false;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
  }


  if ((temperatura > 33) & (humedad > 20)) {
    digitalWrite(humidifier, LOW);
    digitalWrite(outletFan, HIGH); //off
    digitalWrite(inletFan, HIGH);
    digitalWrite(bulb, HIGH);
    digitalWrite(hairDryer, HIGH); //Off
    outlet = false;
    inlet = false;
    dryer = false;
    light = false;
    humidity = true;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
  }

  if ((temperatura > 33) & (humedad >= 33)) {
    digitalWrite(humidifier, HIGH);
    digitalWrite(outletFan, LOW); //off
    digitalWrite(inletFan, LOW);
    digitalWrite(bulb, HIGH);
    digitalWrite(hairDryer, HIGH); //Off
    outlet = true;
    inlet = true;
    dryer = false;
    light = false;
    humidity = false;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
  }


  if ((temperatura > 33) & (humedad > 35)) {
    digitalWrite(humidifier, HIGH);
    digitalWrite(outletFan, LOW); //on
    digitalWrite(inletFan, LOW);
    digitalWrite(bulb, HIGH);
    outlet = true;
    inlet = true;
    dryer = true;
    light = false;
    humidity = false;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off for half second: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
    digitalWrite(hairDryer, LOW); //On
    delay(500);
    digitalWrite(hairDryer, HIGH); //Off
  }

  if ((temperatura <= 33) & (humedad > 20)) {
    digitalWrite(humidifier, LOW);
    digitalWrite(outletFan, HIGH); //off
    digitalWrite(inletFan, HIGH);
    digitalWrite(bulb, LOW);
    digitalWrite(hairDryer, HIGH); //Off
    outlet = false;
    inlet = false;
    dryer = false;
    light = true;
    humidity = true;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
  }



  if ((temperatura <= 33) & (humedad >= 40)) {

    digitalWrite(humidifier, LOW);
    digitalWrite(outletFan, LOW); //on
    digitalWrite(inletFan, LOW);
    digitalWrite(bulb, HIGH);
    digitalWrite(hairDryer, LOW);
    outlet = true;
    inlet = true;
    dryer = true;
    light = false;
    humidity = true;
    ThingSpeak.writeField(myChannelNumber, 4, outlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 5, inlet, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 6, dryer, myWriteAPIKey);
    ThingSpeak.writeField(myChannelNumber, 8, light, myWriteAPIKey);
    Serial.println("Outlet Fan On/Off: ");
    Serial.println(outlet);
    Serial.println("Inlet Fan On/off: ");
    Serial.println(inlet);
    Serial.println("Dryer On/Off: ");
    Serial.println(dryer);
    Serial.println("Bulb On/Off: ");
    Serial.println(light);
    Serial.println("Humidifier On/Off: ");
    Serial.println(humidity);
    delay(3000);
    digitalWrite(hairDryer, HIGH);

  }

}
