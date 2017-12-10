#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <String.h>
#include <servo.h>

int set_servo(Servo, int);
int read_analog();
int get_temp(int);
void handleWebReq_RootPath();

// pio run -e nodemcuv2 -t upload --upload-port 192.168.1.113 
// pio run -e nodemcuv2 -t upload --upload-port monEsp

const char* ssid = "Apt. #3";
const char* password = "Chowfornow3344";
IPAddress ip(192, 168, 1, 113);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
ESP8266WebServer server(80);

int servo_pin = D4;
int ADC_Counts = 0;

int desired_temp = 70;
int current_temp = 70;
int last_temp = current_temp;
Servo control_servo;
void setup() {
  /* code */

  control_servo.attach(servo_pin);
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(3000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("monEsp"); // give an name to our module
  ArduinoOTA.begin(); // OTA initialization

  server.on("/", handleWebReq_RootPath);
  server.begin();
}

void loop() {

  ArduinoOTA.handle();
  server.handleClient();
  // Wait a bit before scanning again
  delay(1000);
  /*for (size_t i = 0; i < 180; i++) {
    set_servo(control_servo,i);
    delay(20);
  }*/
  last_temp     = current_temp;
  current_temp  = get_temp(read_analog());
  current_temp  = (current_temp + last_temp)/2;
  Serial.print("ip: ");
  Serial.println(ip);
  Serial.print("Temp: ");
  Serial.println(current_temp);
  }


int set_servo(Servo servo, int pos)
{
  servo.write(pos);                  // tell servo to go to position in variable 'pos'
  return pos;
}

int read_analog()
{
  int count = 20;
  int val1;
  int val2 = analogRead(A0);
  for (size_t i = 0; i < count; i++) {
    val1 = val2;
    delay(20);
    val2 = analogRead(A0);
    ADC_Counts = (val1 + val2)/2;
  }
  return ADC_Counts+10;
}

int get_temp(int input)
{
  float vAtAnalog = input * 3.3;
  float mv = vAtAnalog /1024;
   float temperatureC = (mv - 0.5) * 100;
 int temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return temperatureF;
}

void handleWebReq_RootPath(){
  char webreply[3];
  sprintf(webreply, "%d",current_temp);
  server.send(200, "text/plain", String(current_temp));
}
