#include <Arduino.h>

#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid     = "wifi_ssid";                          //WiFi Name
const char* password = "";                   					// WiFi Password

#define uS_TO_S_FACTOR 1000000  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  900        //Time ESP32 will go to sleep (in seconds): 900 sec = 15 min

Adafruit_AM2320 am2320 = Adafruit_AM2320(); //declaration of am2320 sensor

float temperature = 0.0;
float humidity = 0.0;

const String baseUrl = "example.com/collector_page?";
String url = "";

//functions
void print_wakeup_reason();


void setup(){
	Serial.begin(115200);
	delay(1000); //Take some time to open up the Serial Monitor
	
	am2320.begin(); //open connection with am2320 sensor


	//Print the wakeup reason for ESP32
	print_wakeup_reason();

	//Set timer
	esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
	Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

	//connect to wifi
	WiFi.begin(ssid, password);
	int wifiCounter = 0;
	while (WiFi.status() != WL_CONNECTED || wifiCounter < 30){
		delay(500);
		Serial.print(".");
		wifiCounter++;
	}

	//read values from sensor
 	temperature = am2320.readTemperature();
	humidity = am2320.readHumidity();
	url = "";
	url = baseUrl + "t=" + String(temperature) + "&h=" + String(humidity); //GET HTTP string creation

	//send data via HTTP GET
	HTTPClient http;
	http.begin(url);
	int httpCode = http.GET();
    if (httpCode > 0) { //Check for the returning code
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
    }
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources

	//Go to sleep now
	esp_deep_sleep_start();
}



void loop(){}



//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
	esp_sleep_wakeup_cause_t wakeup_reason;
	wakeup_reason = esp_sleep_get_wakeup_cause();
	switch(wakeup_reason)
	{
		case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
		case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
		case 3  : Serial.println("Wakeup caused by timer"); break;
		case 4  : Serial.println("Wakeup caused by touchpad"); break;
		case 5  : Serial.println("Wakeup caused by ULP program"); break;
		default : Serial.println("Wakeup was not caused by deep sleep"); break;
	}
}
