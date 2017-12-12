#include <Arduino.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <string>
#include <servo.h>
#define TEXT_BUFF_SIZE 512

int set_servo(Servo, int);
int read_analog();
int get_temp(int);
void handleWebReq_RootPath();
void handleWebReq_Received();
void page_print(String);
bool handle_control();
void handle_temp();
bool boot_wifi(bool on);

// pio run -e nodemcuv2 -t upload --upload-port 192.168.1.113 
// pio run -e nodemcuv2 -t upload --upload-port monEsp

const char* ssid = "Apt. #3";
const char* password = "Chowfornow3344";
IPAddress ip(192, 168, 1, 113);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer web_server(80);
WiFiServer control_server(1234);
WiFiClient control_handler;

Servo control_servo;

int   servo_pin = D4;
int   ADC_Counts = 0;

int   desired_temp = 70;
int   current_temp = 70;
int   last_temp = current_temp;

int   servo_command = 0,last_command = servo_command;

String webpage_output;


void setup() {

  boot_wifi(true);

  ArduinoOTA.setHostname("monEsp"); // give an name to our module
  ArduinoOTA.begin(); // OTA initialization

  web_server.on("/", handleWebReq_RootPath);
  web_server.on("/received", handleWebReq_Received);
  web_server.begin();

  control_server.begin();
  control_handler =control_server.available();

  control_servo.attach(servo_pin);
  set_servo(control_servo,0);
}

void loop() {
  ArduinoOTA.handle();
  web_server.handleClient();
  if(handle_control() && last_command != servo_command)
  {

    set_servo(control_servo, servo_command);
    last_command = servo_command;
  }
  handle_temp();
  delay(1000);

}

bool boot_wifi(bool on)
{
  if(on)
  {
    WiFi.begin(ssid, password);
    WiFi.config(ip, gateway, subnet);

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      delay(3000);
      ESP.restart();
    }
    return true;
  }else{
    WiFi.disconnect();
  }
}

void handle_temp()
{
  last_temp     = current_temp;
  current_temp  = get_temp(read_analog());
  current_temp  = (current_temp + last_temp)/2;
}

bool handle_control()
{
  WiFiClient client;
  String receive_string;
  char last_char = 0;
  int i = 0;
  int receive_size = 0;

  if(!client.connected())
  {client = control_server.available(); }
  if(client.connected() && (receive_size = client.available()) > 0)
  {
  char  receive_buffer[TEXT_BUFF_SIZE];
  char  send_buffer[TEXT_BUFF_SIZE];
    while (i<receive_size && i<TEXT_BUFF_SIZE)
    {
      last_char = client.read();
      receive_buffer[i] = last_char;
      i++;
    }

    client.print(receive_buffer);
    client.println();
    client.stopAll();

    receive_string = String(receive_buffer);
    receive_string.trim();
    i = 0;
    while((i = receive_string.indexOf('{'))!= -1)
    {
      if(receive_string.substring(i+1,receive_string.indexOf(':')) == "set_servo")
      {
        servo_command = receive_string.substring(receive_string.indexOf(':')+1,receive_string.indexOf('}')).toInt();
      }
      receive_string = receive_string.substring(receive_string.indexOf('}'));
    }
    return true;
  }

  return false;
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

int set_servo(Servo servo, int pos)
{
  pos = pos * 1.8;
  page_print("\n Set Servo: "+String(pos));
  servo.write(180-pos);
  return pos;
}

int get_temp(int input)
{
  float vAtAnalog = input * 3.3;
  float mv = vAtAnalog /1024;
   float temperatureC = (mv - 0.5) * 100;
 int temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return temperatureF;
}

void page_print(String publish_string)
{
  webpage_output.concat(publish_string);
}

void handleWebReq_RootPath(){
  char webreply[3];
  sprintf(webreply, "%d",current_temp);
  web_server.send(200, "text/plain", webreply);
}

void handleWebReq_Received(){
  web_server.send(200, "text/plain", webpage_output);
}
