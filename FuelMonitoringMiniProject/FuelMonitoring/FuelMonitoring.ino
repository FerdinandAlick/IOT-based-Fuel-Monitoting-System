#include <Blynk.h> //Blynk App Library
#include <SoftwareSerial.h>
#include <BlynkSimpleEsp8266.h>
#define BLYNK_PRINT Serial
#include <ThingSpeak.h> //Thingspeak Library
#include <ESPEssentials.h> //ESP8266NodeMCU Library
#include <OTA.h>
#include <SerialOut.h>
#include <WebServer.h>
#include <Wifi.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
char auth[] = " 4aegzUl0a5CisgDulVDVNb3sdOwDUjX8"; //Your Blynk Auth Code
String apiKey = "UZOVX01FUFZ9N99A"; // Enter your Write API key from ThingSpeak
const char ssid[] = "realme";     // replace with your wifi ssid and wpa2 key
const char pass[] = "12345678";
const char *server = "api.thingspeak.com";
#define LED_BUILTIN 16
#define SENSOR  4
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned long flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
 
void ICACHE_RAM_ATTR pulseCounter()
{
  pulseCount++;
}
 
WiFiClient client;
void setup()
{
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);
  Blynk.begin(auth, ssid, pass); //To connect to Blynk App
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}
void loop()
{
  WiFi.begin("realme","12345678"); //To connect to WiFi
  while(WiFi.status() != WL_CONNECTED)
{
  Serial.print("....");
  delay(200);
}
Serial.println();
Serial.println("Node Connected");
Serial.println(WiFi.localIP());
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;
 
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
 
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
 
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(float(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

 
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalLitres);
    Serial.println("L");
    Blynk.virtualWrite(V0,String(totalLitres)); //Select Blynk Virtual Pins
    Blynk.virtualWrite(V1,totalLitres);
    Blynk.virtualWrite(V2,String(flowRate));
    Blynk.run();
  }
  
  if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
    String postStr = apiKey; //Thingspeak
      postStr += "&field1=";
      postStr += String(float(flowRate));
      postStr += "&field2=";
      postStr += String(totalLitres);
      postStr += "\r\n\r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
     client.stop();
}
