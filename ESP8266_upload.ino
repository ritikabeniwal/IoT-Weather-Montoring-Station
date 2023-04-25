#include "ThingSpeak.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ESP8266HTTPClient.h>


//------- WI-FI details ----------//
char ssid[] = "******"; //SSID here
char pass[] = "******"; // Password here
//--------------------------------//

//----------- Channel details ----------------//
unsigned long Channel_ID = *****; // Your Channel ID
const char * myWriteAPIKey = "******"; //Your write API key
//-------------------------------------------//

const int Field_Number_1 = 1;
const int Field_Number_2 = 2;
const int Field_Number_3 = 3;
const int Field_Number_4 = 4;
const int Field_Number_5 = 5;
String value = "";  //Define a string to store received data
int temp_value = 0, humidity_value = 0, air_quality_value = 0, pressure_value = 0, light_intensity_value = 0;

WiFiClient  client;
HTTPClient http;

#define ESP_RX_PIN 10 // RX
#define ESP_TX_PIN 11 // TX
SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN); // RX, TX

void setup()
{
  Serial.begin(9600);    // Serial monitor for debugging
  espSerial.begin(115200); // Serial communication with Arduino
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  internet();
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());
  String server = "api.thingspeak.com";
  String url = "/update?api_key=" + String(myWriteAPIKey) +
               "&field1=" + String(temp_value) +
               "&field2=" + String(humidity_value) +
               "&field3=" + String(air_quality_value) +
               "&field4=" + String(pressure_value) +
               "&field5=" + String(light_intensity_value);


  http.begin(client, server, 80, url);
  Serial.println("setup ");
}


void loop()
{
  internet();

  Serial.println("Start ");
  if (espSerial.available() > 0)
  {
    Serial.println("Data Available ");
    delay(100);
    
    while (espSerial.available() > 0)
    {
      value = espSerial.readString();
      Serial.print("value ");
      Serial.println(value);
      if (value[0] == '*')
      {
        Serial.println("value[0]: ");
        Serial.println(value[0]);
        if (value[15] == '#')
        {
          temp_value = ((value[1] - 0x30) * 10 + (value[2] - 0x30));
          Serial.print("Value 1: ");
          Serial.println(temp_value);
          humidity_value = ((value[3] - 0x30) * 10 + (value[4] - 0x30));
          Serial.print("Value 2: ");
          Serial.println(humidity_value);
          air_quality_value = ((value[5] - 0x30) * 10000 + (value[6] - 0x30) * 1000 + (value[7] - 0x30) * 100 + (value[8] - 0x30) * 10 + (value[9] - 0x30));
          Serial.print("Value 3: ");
          Serial.println(air_quality_value);
          light_intensity_value = ((value[12] - 0x30) * 10 + (value[13] - 0x30));
          Serial.print("Value 4: ");
          Serial.println(light_intensity_value);
          pressure_value = ((value[10] - 0x30) * 10 + (value[11] - 0x30));
          Serial.print("Value 5: ");
          Serial.println(pressure_value);
          
        }
      }
    }
  }
  int httpResponseCode = http.GET();
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  upload();
}

void internet()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      delay(5000);
    }
  }
}

void upload()
{
  ThingSpeak.writeField(Channel_ID, Field_Number_1, temp_value, myWriteAPIKey);
  delay(15000);
  ThingSpeak.writeField(Channel_ID, Field_Number_2, humidity_value, myWriteAPIKey);
  delay(15000);
  ThingSpeak.writeField(Channel_ID, Field_Number_3, air_quality_value, myWriteAPIKey);
  delay(15000);
  ThingSpeak.writeField(Channel_ID, Field_Number_4, pressure_value, myWriteAPIKey);
  delay(15000);
  ThingSpeak.writeField(Channel_ID, Field_Number_5, light_intensity_value, myWriteAPIKey);
  delay(15000);
  value = "";
  delay(15000);
}