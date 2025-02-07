#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <AceButton.h>
 
#define BLYNK_TEMPLATE_ID "TMPLXYZ12345"
#define BLYNK_TEMPLATE_NAME "Water"
#define BLYNK_AUTH_TOKEN "abcd1234efgh5678ijkl9012"


char ssid[] = "MONGOwifi";  
char pass[] = "qwer1234";  
int emptyTankDistance = 160;
int fullTankDistance = 20;
int triggerPer = 20;
 
using namespace ace_button;
 
#define TRIG 12                //D6
#define ECHO 13                //D7
#define Relay 14           //D5
#define BP1 2              //D0
#define BP2 13             //D3
#define BP3 15             //D4
 
#define V_B_1 V1
#define V_B_3 V3
#define V_B_4 V4
 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
 
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
float duration;
float distance;
int waterLevelPer;
 
bool toggleRelay = false;
bool modeFlag = true;
String currMode;
 
char auth[] = BLYNK_AUTH_TOKEN;
 
ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
 
void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
 
BlynkTimer timer;
 
void checkBlynkStatus() {
 
  bool isconnected = Blynk.connected();
  if (isconnected == false) {
  }
  if (isconnected == true) {
  }
}
 
BLYNK_WRITE(VPIN_BUTTON_3) {
  modeFlag = param.asInt();
  if (!modeFlag && toggleRelay) {
    digitalWrite(Relay, LOW);
    toggleRelay = false;
  }
  currMode = modeFlag ? "AUTO" : "MANUAL";
}
 
BLYNK_WRITE(VPIN_BUTTON_4) {
  if (!modeFlag) {
    toggleRelay = param.asInt();
    digitalWrite(Relay, toggleRelay);
  } else {
    Blynk.virtualWrite(V_B_4, toggleRelay);
  }
}
 
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V_B_1);
  Blynk.virtualWrite(V_B_3, modeFlag);
  Blynk.virtualWrite(V_B_4, toggleRelay);
}
 
void displayData() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(30, 0);
  display.print(waterLevelPer);
  display.print(" ");
  display.print("%");
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.print(currMode);
  display.setCursor(95, 25);
  display.print(toggleRelay ? "ON" : "OFF");
  display.display();
}
 
void measureDistance() {
 
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = ((duration / 2) * 0.343) / 10;
  if (distance > (fullTankDistance - 15) && distance < emptyTankDistance) {
    waterLevelPer = map((int)distance, emptyTankDistance, fullTankDistance, 0, 100);
    Blynk.virtualWrite(V_B_1, waterLevelPer);
    if (waterLevelPer < triggerPer) {
      if (modeFlag) {
        if (!toggleRelay) {
          digitalWrite(Relay, HIGH);
          toggleRelay = true;
          Blynk.virtualWrite(V_B_4, toggleRelay);
        }
      }
    }
    if (distance < fullTankDistance) {
      if (modeFlag) {
        if (toggleRelay) {
          digitalWrite(Relay, LOW);
          toggleRelay = false;
          Blynk.virtualWrite(V_B_4, toggleRelay);
        }
      }
    }
  }
  displayData();
  delay(100);
}
 
void setup() {
  Serial.begin(9600);
  pinMode(ECHO, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(Relay, OUTPUT);
 
  pinMode(BP1, INPUT_PULLUP);
  pinMode(BP2, INPUT_PULLUP);
  pinMode(BP3, INPUT_PULLUP);
 
  digitalWrite(Relay, HIGH);
 
  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
 
  button1.init(BP1);
  button2.init(BP2);
  button3.init(BP3);
 
  currMode = modeFlag ? "AUTO" : "MANUAL";
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(1000);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
 
  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus);
  timer.setInterval(1000L, measureDistance);
  Blynk.config(auth);
  delay(1000);
 
  Blynk.virtualWrite(V_B_3, modeFlag);
  Blynk.virtualWrite(V_B_4, toggleRelay);
 
  delay(500);
}
 
void loop() {
  Blynk.run();
  timer.run();
  button1.check();
  button3.check();
 
  if (!modeFlag) {
    button2.check();
  }
}
 
void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT1");
  switch (eventType) {
    case AceButton::kEventReleased:
      if (modeFlag && toggleRelay) {
        digitalWrite(Relay, LOW);
        toggleRelay = false;
      }
      modeFlag = !modeFlag;
      currMode = modeFlag ? "AUTO" : "MANUAL";
      Blynk.virtualWrite(V_B_3, modeFlag);
      break;
  }
}
 
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT2");
  switch (eventType) {
    case AceButton::kEventReleased:
      if (toggleRelay) {
        digitalWrite(Relay, LOW);
        toggleRelay = false;
      } else {
        digitalWrite(Relay, HIGH);
        toggleRelay = true;
      }
      Blynk.virtualWrite(V_B_4, toggleRelay);
      delay(1000);
      break;
  }
}
 
void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT3");
  switch (eventType) {
    case AceButton::kEventReleased:
      break;
  }
}
