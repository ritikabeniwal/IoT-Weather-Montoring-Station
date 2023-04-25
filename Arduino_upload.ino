#include <LiquidCrystal.h> 
#include <SoftwareSerial.h>
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>

// initialize LCD interface pin with the arduino pin number it is connected to
const int rs = 13, en = 12, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define ESP_RX_PIN 10
#define ESP_TX_PIN 11

SoftwareSerial espSerial(ESP_RX_PIN, ESP_TX_PIN);  //Rx, Tx

#define DHTTYPE DHT11 
#define DHTPIN 7   // Pin connected to the DHT sensor
#define MQ135PIN A0   // Air Quality sensor pin
#define LDRpin A1    // Analog pin connected to the LDR  
#define seaLevelPressure_hPa 1013.25

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;

//initial values = 0
float humidity = 0;
float temperature = 0;
float pressure = 0;
float altitude = 0;
float seaLevelPressure = 0;
float realAltitude = 0;
float airqlty = 0;
float LDRValue = 0;

//functions
void getDHTvalues();
void getPressure();
void getAirQualityVal();
void getLDRValue();
void send_data(void);

bool BMP_flag = false;
bool DHT_flag = false;

void setup() {
  espSerial.begin(115200);
  Serial.begin(9600);
  pinMode(MQ135PIN, INPUT);
  pinMode(LDRpin, INPUT);
  lcd.begin(16, 2);
  dht.begin();
  //column, row
  lcd.setCursor(0, 0);
  lcd.print("  IOT  WEATHER  ");
  lcd.setCursor(0, 1);
  lcd.print("MONITOR SYSTEM");
  delay(1500);
}

void loop() 
{
  getDHTvalues();
  getPressure();
  getAirQualityVal();
  getLDRValue();
  send_data();
} 

void getDHTvalues()
{
  lcd.clear();
  // Read humidity as %age
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperature = dht.readTemperature();
  if (isnan(humidity)|| isnan(temperature)) 
  {
  lcd.setCursor(0, 0);
  lcd.print("DHT11 MISSING");
  Serial.println(F("Failed to read humidity from DHT sensor!"));
  DHT_flag = false;
  return;
  }
  else
  {
    DHT_flag = true;
    lcd.setCursor(2, 0);
    lcd.print("TEMP:");
    lcd.print(temperature);
    lcd.print(" *C ");
    lcd.setCursor(2, 1);
    lcd.print("HUM:");
    lcd.print(humidity);
    lcd.print(" %");

    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C "));
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.println(F("% "));
  }
  delay(2000);
}


void getPressure()
{
  lcd.clear();
  if (!bmp.begin()) 
  {
    lcd.setCursor(0, 0);
    lcd.print("BMP180 MISSING");
    BMP_flag = false;
    delay(2000);
    return;
  }
  else
  {
    BMP_flag = true;
    lcd.setCursor(0, 0);
    lcd.print("PA(GND):");
    pressure = bmp.readPressure() / 100.0F; // convert to hPa and store as float
    lcd.print(pressure); // print pressure with 1 decimal point
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print("PA(sea):");
    seaLevelPressure = bmp.readSealevelPressure() / 100.0F; // convert to hPa and store as float
    lcd.print(seaLevelPressure); // print pressure with 1 decimal point
    altitude = bmp.readAltitude(seaLevelPressure); // calculate altitude based on sea level pressure

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");
    Serial.print("Pressure at sealevel (calculated) = ");
    Serial.print(bmp.readSealevelPressure());
    Serial.println(" Pa");
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(seaLevelPressure_hPa * 100));
    Serial.println(" meters");
  }
  delay(2000);
}

void getAirQualityVal()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AIR QUALITY: ");
  airqlty = analogRead(MQ135PIN);
  int mappedValue = map(airqlty, 0, 1023, 0, 100);
  lcd.print(mappedValue);
  lcd.setCursor(2, 1);
  if (mappedValue <= 60)
    lcd.print("GOOD!");
  else if (mappedValue > 60 && mappedValue <= 80)
    lcd.print("FAIR");
  else if (mappedValue > 80 && mappedValue <= 90)
    lcd.print("POOR");
  else
    lcd.print("BAD");
  delay(2000);

  Serial.print("AirQuality ");
  Serial.println(airqlty);
}


void getLDRValue()
{
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("LIGHT: ");
  int LDRValue = analogRead(LDRpin); 
  int y = map(LDRValue, 0, 1023, 0, 100);
  lcd.print(y);
  lcd.print(" %");
  lcd.setCursor(0, 1);
  if (y <= 20)
    lcd.print("POOR INTENSITY!");
  else if (y > 20 && y <= 60)
    lcd.print("LOW INTENSITY!");
  else if (y > 60 && y <= 100)
    lcd.print("HIGH INTENSITY!");
  else
    lcd.print(" ");

  delay(2000);
  Serial.print("Light Intensity: ");
  Serial.print(y);
  Serial.println(" %");
}

void send_data()
{
  const char START_CHAR = '*';
  const char END_CHAR = '#';
  Serial.println("Sending Data... ");
  espSerial.write(START_CHAR); // Starting char
  if(DHT_flag == true)
  {
  Serial.println("DHT_flag = true, sending actual temp & hum ");
  //send on esp serial to esp8266
  espSerial.print(dht.readHumidity(), 0);  
  espSerial.print(dht.readTemperature(), 0); 
  }
  else 
  {
  Serial.println("DHT_flag = false, sending dummy temp & hum ");
  espSerial.print("00.00"); //send dummy data on esp serial to esp8266
  }

  espSerial.print(map(analogRead(LDRpin), 0, 1023, 0, 100)); 
  espSerial.print(map(analogRead(MQ135PIN), 0, 1023, 0, 100)); 

  if(BMP_flag == true)
  {
  Serial.println("BMP_flag = true, sending actual pressure ");
  espSerial.print(bmp.readPressure()); //send on esp serial to esp8266
  }
  else 
  {
  Serial.println("BMP_flag = false, sending dummy pressure "); // print on serial monitor
  espSerial.print("00.00"); //send dummy data on esp serial to esp8266
  }

  espSerial.write(END_CHAR); // Ending char
  Serial.println("Sent Data ");
}
