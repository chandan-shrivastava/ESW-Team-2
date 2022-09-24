#include <NewPing.h>
#include <Servo.h>
#include <MotorDriver.h>
#include "ThingSpeak.h"
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

MotorDriver m;
bool on = false;

#define TRIG_PIN A0
#define ECHO_PIN A1
#define irRight A2
#define irLeft A3

#define MAX_DISTANCE 200
#define MAX_SPEED 190

unsigned long myChannelNumber = 1845434;
const char *myWriteAPIKey = "L2NP3A6OPNQPVWNU";

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

Servo myservo;

boolean goesForward = false;
boolean object;
int distance = 100;
int speedSet = 0;
float angle = 0;
float x = 0;
float y = 0;
float pi = 3.14;

char ssid[] = "Chandan Shrivastava";
char pass[] = "1234567890";
int status = WL_IDLE_STATUS;
WiFiServer server(80);
char cse_ip[] = "192.168.3.210";
int cse_port = 8080;
WiFiClient wifi;
HttpClient Hclient = HttpClient(wifi, cse_ip, cse_port);
WiFiClient client = server.available();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  connect_WiFi();
  server.begin();
  printWifiStatus();
  pinMode(irLeft, INPUT);
  pinMode(irRight, INPUT);
  myservo.attach(10);
  myservo.write(115);
  delay(2000);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);

  ThingSpeak.begin(client);
}

void loop()
{
  WiFiClient client = server.available();

  if (client)
  {
    on = !on;
    client.stop();
  }
  if (on)
  {
    int xd = digitalRead(irLeft);
    int yd = digitalRead(irRight);
    if (xd == 0 && yd == 0)
    {
      objectAvoid();
    }
    else if (xd == 0 && yd == 1)
    {
      // objectAvoid();
      moveStop();
      delay(100);
      moveBackward();
      delay(300);
      turnRight();
    }
    else if (xd == 1 && yd == 0)
    {
      // objectAvoid();
      moveStop();
      delay(100);
      moveBackward();
      delay(300);
      turnLeft();
    }
    else if (xd == 1 && yd == 1)
    {
      moveStop();
      delay(100);
      moveBackward();
      delay(300);
      turnRight();
    }
  }
  else
  {
    moveStop();
  }
}
void objectAvoid()
{
  distance = readPing();

  int distanceR = 0;
  int distanceL = 0;
  if (distance <= 25)
  {
    distance = 250;
    moveStop();
    delay(100);
    moveBackward();
    delay(300);
    moveStop();
    delay(200);
    unsigned long t = millis();
    x=x*0.5;
    y=y*0.5;
    t=t*0.5;
    String val = String(x) + " " + String(y) + " " + String(t);



    createCI(val);
    ThingSpeak.setField(1, x);
    ThingSpeak.setField(2, y);
    int response_thingspeak = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    distanceR = lookRight();
    delay(200);
    distanceL = lookLeft();
    delay(200);
    Serial.println(distanceL);
    Serial.println(distanceR);
    Serial.println(distance);
    if (distanceR >= distanceL)
    {
      turnRight();
      moveStop();
    }
    else
    {
      turnLeft();
      moveStop();
    }
  }
  else
  {
    moveForward();
  }
  distance = readPing();
}

int lookRight()
{

  myservo.write(50);
  delay(500);
  int distance = readPing();
  delay(100);
  myservo.write(115);
  return distance;
}

int lookLeft()
{
  myservo.write(170);
  delay(500);
  int distance = readPing();
  delay(100);
  myservo.write(115);
  return distance;
  delay(100);
}

int readPing()
{
  delay(70);
  int cm = int(sonar.ping_median() / 57);
  if (cm == 0)
  {
    cm = 250;
  }
  return cm;
}

void moveStop()
{
  m.motor(1, FORWARD, 0);
  m.motor(2, FORWARD, 0);
  m.motor(3, FORWARD, 0);
  m.motor(4, FORWARD, 0);
}

void moveForward()
{

  goesForward = true;
  m.motor(1, FORWARD, speedSet);
  m.motor(2, FORWARD, speedSet);
  m.motor(3, FORWARD, speedSet);
  m.motor(4, FORWARD, speedSet);
  x = x + speedSet * cos(angle * pi / 180);
  y = y + speedSet * sin(angle * pi / 180);
  // Serial.println("in move forward");
  for (speedSet = speedSet; speedSet < MAX_SPEED; speedSet += 2)
  {
    m.motor(1, FORWARD, speedSet);
    m.motor(2, FORWARD, speedSet);
    m.motor(3, FORWARD, speedSet);
    m.motor(4, FORWARD, speedSet);
    x = x + speedSet * cos(angle * pi / 180);
    y = y + speedSet * sin(angle * pi / 180);
    delay(5);
  }
}

void moveBackward()
{
  goesForward = false;
  m.motor(1, BACKWARD, speedSet);
  m.motor(2, BACKWARD, speedSet);
  m.motor(3, BACKWARD, speedSet);
  m.motor(4, BACKWARD, speedSet);
  x = x - speedSet * cos(angle * pi / 180);
  y = y - speedSet * sin(angle * pi / 180);
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 2)
  {
    m.motor(1, BACKWARD, speedSet);
    m.motor(2, BACKWARD, speedSet);
    m.motor(3, BACKWARD, speedSet);
    m.motor(4, BACKWARD, speedSet);
    x = x - speedSet * cos(angle * pi / 180);
    y = y - speedSet * sin(angle * pi / 180);
    delay(5);
  }
}

void turnRight()
{
  m.motor(1, FORWARD, speedSet);
  m.motor(2, BACKWARD, speedSet);
  m.motor(3, BACKWARD, speedSet);
  m.motor(4, FORWARD, speedSet);
  angle -= 15;
  delay(1000);
  m.motor(1, FORWARD, speedSet);
  m.motor(2, FORWARD, speedSet);
  m.motor(3, FORWARD, speedSet);
  m.motor(4, FORWARD, speedSet);
  x = x + speedSet * cos(angle * pi / 180);
  y = y + speedSet * sin(angle * pi / 180);
  loop();
}

void turnLeft()
{
  m.motor(1, BACKWARD, speedSet);
  m.motor(2, FORWARD, speedSet);
  m.motor(3, FORWARD, speedSet);
  m.motor(4, BACKWARD, speedSet);
  angle += 15;
  delay(1000);
  m.motor(1, FORWARD, speedSet);
  m.motor(2, FORWARD, speedSet);
  m.motor(3, FORWARD, speedSet);
  m.motor(4, FORWARD, speedSet);
  x = x + speedSet * cos(angle * pi / 180);
  y = y + speedSet * sin(angle * pi / 180);
  loop();
}

void createCI(String val)
{
  String body = "{\"m2m:cin\": {\"lbl\": \"Team-2\",\"con\": \"" + String(val) + "\"}}";
  Hclient.beginRequest();
  Hclient.post("/~/in-cse/in-name/Team-2/Node-1/Data/");
  Hclient.sendHeader("Content-Length", body.length());
  Hclient.sendHeader("X-M2M-Origin", "admin:admin");
  Hclient.sendHeader("Content-Type", "application/json;ty=4");
  Hclient.sendHeader("Connection", "keep-alive");
  Hclient.beginBody();
  Hclient.print(body);
  Hclient.endRequest();
  int status = Hclient.responseStatusCode();
  String responseBody = Hclient.responseBody();
  delay(1000);
}

void printWifiStatus()
{
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.print(ip);
}

void connect_WiFi()
{
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
}
