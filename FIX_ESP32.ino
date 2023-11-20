#include <Wire.h>
#include <WiFiClient.h> 
#include <WiFi.h>
#include <HTTPClient.h>
WiFiClient client;

#define LED 2
#define vibSensorPin 34
#define TRIG_PIN 5
#define ECHO_PIN 18
#define SOUND_SPEED 0.034

const char *ssid = "Taman Firdaus 2.4GHz";  //ENTER YOUR WIFI SETTINGS
const char *password = "orangSuci666";

//Web/Server address to read/write from 
const char *host = "192.168.100.11";   //https://circuits4you.com website or IP address of server
//192.168.100.14 --> Tiga Putra Wifi

//Inisialisasi Sensor Ultrasonik
float duration_us, distance_cm;
int count, count2= 0;
int personCount = 0;
static bool someoneInside = false;
static bool someoneInsideNow = someoneInside;
int activateCountNoPerson, activateCountPerson = 1;
int temp[1];
int validateBreak = 0;

//Inisialisasi Sensor Vibration
bool value;

TaskHandle_t task1;
TaskHandle_t task2;
TaskHandle_t task3;

void updateData();

void updateData(){
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration_us = pulseIn(ECHO_PIN, HIGH);
  // calculate the distance
  distance_cm = 0.017 * duration_us;
  // print the value to Serial Monitor
  Serial.print("distance ketika loop: ");
  Serial.print(distance_cm); 
  Serial.println(" cm");
}

String get_wifi_status(int status){
  switch(status){
    case WL_IDLE_STATUS:
    return "WL_IDLE_STATUS";
    case WL_SCAN_COMPLETED:
    return "WL_SCAN_COMPLETED";
    case WL_NO_SSID_AVAIL:
    return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:
    return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
    return "WL_CONNECTION_LOST";
    case WL_CONNECTED:
    return "WL_CONNECTED";
    case WL_DISCONNECTED:
    return "WL_DISCONNECTED";
  }
}

void postData(void * parameter){
   for (;;) {
    HTTPClient http;    //Declare object of class HTTPClient
    String postData, vibration, person_count, link;

    vibration = String(value);
    person_count = String(personCount);
    
     //Post Data
    postData = "&vibration=" + vibration + "&personCount=" + person_count;
    link = "http://fluzzteam.tech/post-esp-data.php";
    http.begin(client,link);              //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  
    int httpCode = http.POST(postData);   //Send the request
    String payload = http.getString();    //Get the response payload
  
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload
  
    http.end();  //Close connection
    
    delay(300);  //Post Data at every 10 seconds
   }
  }

void ultrasonic(void * parameter) {
  for (;;) { // infinite loop
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);
    
      // measure duration of pulse from ECHO pin
      duration_us = pulseIn(ECHO_PIN, HIGH);
    
      // calculate the distance
      distance_cm = 0.017 * duration_us;
    
      // print the value to Serial Monitor
      Serial.print("distance: ");
      Serial.print(distance_cm); 
      Serial.println(" cm");
    
      if (distance_cm < 30){
        //flag1 = 1;
              if(activateCountPerson == 1){
                for(count2 =0;count2<= 10;count2++){
                  Serial.println("Count= " +String(count2));
                  //Serial.println("jarak= "+ String(distance_cm));
                  updateData();
                  if(distance_cm > 30){
                  //validateBreak = 1;
                  return;}
                  delay(500);
                }
                //if(validateBreak = 0){
                  someoneInside = true;
                  if (someoneInsideNow != someoneInside){
                    Serial.print (" --- Ada Orang");
                    personCount++;
                    personCount += temp[0];
                    temp[0] = personCount;
                    someoneInsideNow = someoneInside; 
                  }
                  activateCountNoPerson = 1;
                  activateCountPerson = 0;
                //}
              }
              else {
                //validateBreak = 0;
                someoneInside = true;
                  if (someoneInsideNow != someoneInside){
                    Serial.print (" --- Ada Orang");
                    personCount++;
                    personCount += temp[0];
                    temp[0] = personCount;
                    someoneInsideNow = someoneInside; 
                    activateCountNoPerson = 1;
                  }
              }
      }   
       else {
          if(activateCountNoPerson == 1){
            someoneInside = false;
            for(count =0;count<= 10;count++){
              Serial.println("Count= " +String(count));
              //Serial.println("jarak= "+ String(distance_cm));
              //delay(500);
              updateData();
              if(distance_cm < 30){;
                return;}
              // }
              delay(500);
            }
            //if(validateBreak = 0){
            someoneInsideNow = someoneInside;
            if(count == 10){
              someoneInside = false; 
              Serial.print (" --- Tidak ada Orang");
              count = 0;
            }
            personCount = 0;
            activateCountNoPerson = 0;
            activateCountPerson = 1;
            //}
          }else{
            //validateBreak = 0;
            someoneInside = false;
            someoneInsideNow = someoneInside;
            someoneInside = false;
            personCount = 0;
            activateCountPerson = 1;
            Serial.print("Count= " + String(count));
            Serial.print (" --- Tidak ada Orang");
          }
        }
      
      Serial.print("  Person Count = ");
      Serial.println(personCount);
      delay(2000);
      
    // Pause the task for 500 ms
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void vibration(void * parameter) {
  for (;;) { // infinite loop
    value = digitalRead(vibSensorPin);
    Serial.print("Vibration status: ");
    Serial.println(value);
    if (value == 1) {
      digitalWrite(LED, HIGH);
    }
      else if(value == 0){
      digitalWrite(LED, LOW);
    }

    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(vibSensorPin, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  int status = WL_IDLE_STATUS;
  Serial.println("\nConnecting...");
  Serial.println(get_wifi_status(status));
  WiFi.begin(ssid, password);

  while(status != WL_CONNECTED){
      delay(500);
      status = WiFi.status();
      Serial.println(get_wifi_status(status));
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  xTaskCreatePinnedToCore(
    ultrasonic,    // Function that should be called
    "Ultrasonic Sensor",   // Name of the task (for debugging)
    2048,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &task1,             // Task handle
    0          // Core you want to run the task on (0 or 1)
  );

  xTaskCreatePinnedToCore(
    vibration,    // Function that should be called
    "Vibration Sensor",   // Name of the task (for debugging)
    2048,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &task2,             // Task handle
    0          // Core you want to run the task on (0 or 1)
  );

  xTaskCreatePinnedToCore(
    postData,    // Function that should be called
    "PostData",   // Name of the task (for debugging)
    2048,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &task3,             // Task handle
    0          // Core you want to run the task on (0 or 1)
  );

}

void loop() {
}
