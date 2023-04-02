// Assumes Lolin D32 ESP32 development board
// https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/

#define STOPWATCH_START 0

#define BUTTON_PIN 32 // GIOP21 pin connected to button
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

const static double fullBattery = 0.61;
const static double emptyBattery = 0.36;
#define ESP32_BATTERY_PIN 35

#if !(STOPWATCH_START)
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// display refresh in ms
#define DISPLAY_REFRESH 100

// battery status at STOPWATCH_START
int startBattery = 0;

int lastDisplay = millis();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#endif


// REPLACE WITH THE MAC Address of your receiver 
#if !(STOPWATCH_START)
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0xA4, 0x83, 0x0C};
#else
uint8_t broadcastAddress[] = {0x24, 0x6F, 0x28, 0x1F, 0x3C, 0x70};
#endif

// Variable to store if sending data was successful
String success;

esp_now_peer_info_t peerInfo;

#if STOPWATCH_START

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  } else{
    success = "Delivery Fail :(";
  }
}

#else
int startTime = 0, stopTime = 0;
boolean running = false;  

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t* incoming, int len) {
  memcpy(&startBattery, incoming, sizeof(startBattery));
  Serial.print("Bytes received: ");
  Serial.println(len);

  // reset and start clock
  startTime = millis();
  running = true;
}

#endif

int getBatteryPercentage() {
  double adcVoltage;
  adcVoltage = analogRead(ESP32_BATTERY_PIN) / 4095.0;
  int percentage = 100 * (adcVoltage - emptyBattery) / (fullBattery - emptyBattery); 

  return percentage;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

#if !(STOPWATCH_START)
  // Init OLED display
  startTime = millis();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
#endif

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

#if STOPWATCH_START
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
#endif
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

#if !(STOPWATCH_START)
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
#endif

}
 
void loop() {
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH) {
    // button pressed
#if STOPWATCH_START
    // Send message via ESP-NOW
    int percentage = getBatteryPercentage();
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&percentage, sizeof(percentage));
     
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
#else    
    stopTime = millis();
    running = false;
#endif
  
  }

  lastState = currentState;

#if !(STOPWATCH_START)
  if (millis() - lastDisplay > DISPLAY_REFRESH) {
    lastDisplay = millis();
    updateDisplay();
  }
#endif
}

#if !(STOPWATCH_START)
void updateDisplay(){

  int percentage = getBatteryPercentage();
  Serial.print("Battery: ");
  Serial.print(percentage);
  Serial.println("%");
  
  int elapsed;
  if (running) {
    elapsed = (millis() - startTime);
  } else {
    elapsed = (stopTime - startTime);
  }

  // Display Readings on OLED Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // start battery
  display.print("SB: ");
  display.print(startBattery);
  display.print("% ");

  // finish battery
  display.print("FB: ");
  display.print(percentage);
  display.println("%");

  display.setCursor(0, 15);
  display.setTextSize(5);
  display.print(elapsed / 1000.0, 1);
  display.display();
  
  // Display Readings in Serial Monitor
  Serial.print("Clock: ");
  Serial.println(elapsed / 1000.0, 1);
  Serial.print("Running: ");
  Serial.println(running ? "true" : "false");
}
#endif
