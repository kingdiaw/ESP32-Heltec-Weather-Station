//Include Related Library
#include <WiFi.h>
#include <ThingSpeak.h>           // ThingSpeak by MathWorks version 2.0.1
#include <heltec.h>               // Heltec Library
#include <Adafruit_BMP085.h>      // Adafruit BMP085 by Adafruit version 1.2.2
#include <BH1750.h>               // BH1750 by Christopher Laws version 1.3.0

#define SECRET_SSID "tomato"      // replace with your WiFi network name
#define SECRET_PASS "king@535383" // replace with your WiFi password

#define SECRET_CH_ID 952592       // replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "FC78KESIUHFA9LO3"   // replace XYZ with your channel write API Key

// Calibrate the altitude by changing this default pressure value for your location
#define MyAltitude 101300  // 101300 Pascal is a Pressure at altitude approx. 20m above sea level(Padang Besar, Perlis)

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

WiFiClient  client;
Adafruit_BMP085 bmp;
BH1750 lightMeter;
TwoWire I2Cone = TwoWire(0);


// struct definition for storing results
struct bmpResults {
  float temperature;
  int pressure;
  float altitude;
  int seaLevelPressure;
};

//User Define Function-readBMP180()
//==============================================================
void readBMP180(bmpResults *allSensorResults) {
  //Update data from sensor
  allSensorResults->temperature = bmp.readTemperature();
  allSensorResults->pressure = bmp.readPressure();
  allSensorResults->altitude = bmp.readAltitude(MyAltitude);
  allSensorResults->seaLevelPressure = bmp.readSealevelPressure();

  // output all results to serial monitor first
  Serial.print("Temperature = ");
  Serial.print(allSensorResults->temperature);
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(allSensorResults->pressure);
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  Serial.print("Altitude = ");
  Serial.print(allSensorResults->altitude);
  Serial.println(" meters");

  Serial.print("Pressure at sealevel (calculated) = ");
  Serial.print(allSensorResults->seaLevelPressure);
  Serial.println(" Pa");

  Serial.print("Real altitude = ");
  Serial.print(bmp.readAltitude(MyAltitude));
  Serial.println(" meters");

  Serial.println();
  Heltec.display->clear();
  // output all data to OLED display
  String Pres = "Pressure: " + (String)(float(allSensorResults->pressure) / 100) + " hPa";
  Heltec.display->drawString(0, 0, Pres);

  String Temp = "Temperature: " + (String)allSensorResults->temperature + " °C";
  Heltec.display->drawString(0, 15, Temp);

  String Alt = "Altitude: " + (String)allSensorResults->altitude + " m";
  Heltec.display->drawString(0, 30, Alt);
  Heltec.display->display();
}

//User Define Function-readBH1750()
//==============================================================
void readBH1750(float* lux) {
  *lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(*lux);
  Serial.println(" lx");
  Serial.println();

  // output all data to OLED display
  String Light = "Light: " + (String)lightMeter.readLightLevel() + " lx";
  Heltec.display->drawString(0, 45, Light);
  Heltec.display->display();
}
//=============================================================

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  Heltec.display->resetOrientation();
  //Heltec.display->flipScreenVertically();  // flip display if needed
  Heltec.display->clear();

  if (!bmp.begin())  // checking if sensor is available
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");

  if (!lightMeter.begin())  // checking if sensor is available
    Serial.println("Could not find a valid sensor, check wiring!");

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  bmpResults sensorResults;
  float luxResult;

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this  line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  //Update data from sensors
  readBMP180(&sensorResults);
  readBH1750(&luxResult);

  // Write to ThingSpeak.
  ThingSpeak.setField(1, sensorResults.temperature);
  ThingSpeak.setField(2, (float)sensorResults.pressure / 100);
  ThingSpeak.setField(3, sensorResults.altitude);
  ThingSpeak.setField(4, luxResult);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200)
    Serial.println("Channel update successful.");

  else
    Serial.println("Problem updating channel. HTTP error code " + String(x));

  //Wait here for 15 seconds
  delay(15000); 

}
