/********
 * Mbasa Mguguma
 * Webserver for system data representation
 **/

//required libraries
#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
#include <StringArray.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include "FS.h"
#include <DHT.h>
#include "SimpleKalmanFilter.h"
#include <SPI.h>

//function declaration here:
//int myFunction(int, int);

// Network credentials
const char* ssid="ESP32Cam";
const char* password="EEE4113_18";

//for authentication
const char* http_username= "dddd";
const char* http_password= "adminnnn";
int log_count=0;

const char* http_user_salt= "";

// set web server port
AsyncWebServer server(80);

//set web socket
//AsyncWebSocket ws("/ws");

//set event source
AsyncEventSource events("/events");

// Define the pin for the DHT22 sensor
#define DHTPIN1 13
#define DHTPIN2 15
const int analogPin = 14; 
// Define the sensor type
#define DHTTYPE DHT22

// Initialize the DHT22 sensor
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// Define the Kalman filter parameters
float e_mea = 0.1; // Measurement Uncertainty
float e_est = 0.1; // Estimation Uncertainty
float q = 0.01;    // Process Variance

// Initialize the Kalman filter
SimpleKalmanFilter kf_temp(e_mea, e_est, q);
SimpleKalmanFilter kf_hum(e_mea, e_est, q);
SimpleKalmanFilter kf_volt(e_mea, e_est, q);
 

//Json Variable to store sensor readings
JSONVar readings;
String fileData;
//var for sensor data
float tData, hData;

#define FILE_PHOTO "/picture.jpg"
boolean newPhotoTaken = false;
//Camera pins
#define PWDN_GPIO     32
#define RESET_GPIO    -1
#define XCLK_GPIO     0
#define SIOD_GPIO     26
#define SIOC_GPIO     27
#define Y9_GPIO       35
#define Y8_GPIO       34
#define Y7_GPIO       39
#define Y6_GPIO       36
#define Y5_GPIO       21
#define Y4_GPIO       19
#define Y3_GPIO       18
#define Y2_GPIO       5
#define VSYNC_GPIO    25
#define HREF_GPIO     23
#define PCLK_GPIO     22


//GPIO pins
//#define tempPin 12
//#define SD_CS 5

//Current time
unsigned long currentTime = millis();
//Previous time
unsigned long prevTime = 0;
//timeout time in milliseconds
const long timeOutTime=30000;

void initWiFi(){
  //Connect to the WiFi network
  Serial.print("Setting up Access Point :");
  Serial.println(ssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  // long int sTime= millis();
  // while (WiFi.status() != WL_CONNECTED){
  //   delay(500);
  //   Serial.print(".");
  //   if ((sTime+1000000)<millis()){
  //     break;
  //   }
  // }
  //print out local IP address
  Serial.println("");
  Serial.println("WiFi Access point setup succesfully!");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

//initialize camera
void initCamera(){
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    camera_config_t config;
    config.ledc_channel= LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO;
    config.pin_d1 = Y3_GPIO;
    config.pin_d2 = Y4_GPIO;
    config.pin_d3 = Y5_GPIO;
    config.pin_d4 = Y6_GPIO;
    config.pin_d5 = Y7_GPIO;
    config.pin_d6 = Y8_GPIO;
    config.pin_d7 = Y9_GPIO;
    config.pin_xclk = XCLK_GPIO;
    config.pin_pclk = PCLK_GPIO;
    config.pin_vsync = VSYNC_GPIO;
    config.pin_href = HREF_GPIO;
    config.pin_sccb_scl= SIOC_GPIO;
    config.pin_sccb_sda = SIOD_GPIO;
    config.pin_pwdn = PWDN_GPIO;
    config.pin_reset = RESET_GPIO;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_LATEST;

    if (psramFound()){
      config.frame_size= FRAMESIZE_UXGA;
      config.jpeg_quality = 10;
      config.fb_count=1;
    }
    esp_err_t err= esp_camera_init(&config);
    if(err !=ESP_OK){
      Serial.printf("Camera init failed with error 0x%x", err);
      ESP.restart();
    }
  }
//initialize SPIFFS
void initSPIFFS(){
  if(!SPIFFS.begin(true)){
    Serial.println("mounting SPIFFS error");
    return;
  }
  Serial.println("SPIFFS mounted succesfully.");
  
}

void formatSPIFFS(){
  Serial.println("formatting SPIFFS.");
  if(SPIFFS.format()){
    Serial.println("SPIFFS formated successfully.");
  }else {
    Serial.println("Error formatting SPIFFS.");
  }
}

String getSensorReadings(){
  float v=(rand()%(144-143+1)+143)/(12);
  int t= rand()%(21-18+1)+ 18;
  int h=rand()%(91-88+1)+ 88;
  Serial.println("getting sensor readings");
  readings["temperature"] = String(t);
  readings["humidity"] = String(h);
  readings["voltage"] = String(v);
  File file = SPIFFS.open("/data.txt", "a");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return "--";
  }
  else{
    file.print(t);
    file.print(",");
    file.print(h);
    file.print(",");
    file.print(v);
    file.println("");
    //file.close();
  }
   String sData = JSON.stringify(readings);
   Serial.println(sData);
   return sData;
}

// String getSensorReadings(){
//   float h1 = dht1.readHumidity();
//   float t1=dht1.readTemperature();
//   float h2 =dht2.readHumidity() ;
//   float t2 = dht2.readTemperature();
//   float avgTemp = (t1 + t2)/2;
//   float avgHum = (h1 + h2)/2;
//   int analogValue = analogRead(14);
//   float voltage = (float)analogValue * (10.74/4095) + 1.27;
//   // Apply Kalman filter to the sensor readings
//   float filteredTemperature = kf_temp.updateEstimate(avgTemp);
//   float filteredHumidity = kf_hum.updateEstimate(avgHum);
//   float filteredVoltage = kf_volt.updateEstimate(voltage);
//   float temp = filteredTemperature*100;
//   float hum = filteredHumidity*100;
//   float volt = filteredVoltage*100;
//   readings["temperature"] = String(temp);
//   readings["humidity"] = String(hum);
//   readings["voltage"] = String(volt);
//   //readings["current"] = String(c);
//   int temperatureInteger = (int)temp;
//   int humidityInteger = (int)hum;
//   int voltageInteger = (int)volt;
//   File file = SPIFFS.open("/data.txt", "w");
//   if (!file) {
//     Serial.println("Failed to open file for writing");
//     return "--";
//   }
//   else{
//     file.print(temperatureInteger);
//     file.print(",");
//     file.print(humidityInteger);
//     file.print(",");
//     file.print(voltageInteger);
//     file.println("");
//     file.close();
//   }
//   String sData = JSON.stringify(readings);
//   return sData;
// }

void readFromFile(fs::FS &fs,const char* path){
  //read file from SPIFFS
  File file= SPIFFS.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("failed to open file");
    return;
  }
  Serial.println("reading from the file");
  while(file.available()){
    fileData = file.readString();
    Serial.print(fileData);
  }
}
//check if saved succesfully
bool checkPhoto(fs::FS &fs){
  File f_pic = fs.open(FILE_PHOTO);
  unsigned int pic_size = f_pic.size();
  return(pic_size>100);
}
//take picture and save to spiffs
void captureAndSave(void){
  camera_fb_t* fb= NULL;
  bool ok= 0;

  do{
    //take picture
    Serial.println("Taking a photo");
    fb= esp_camera_fb_get();
    if(!fb){
      Serial.println("Camera capture failed");
      return;
    }
    Serial.printf("Picture file name: %s \n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);

    if(!file){
      Serial.println("Failed to open file in writing");
    }
    else{
      file.write(fb->buf, fb->len);
      Serial.print("the picture has been saved in ");
      Serial.println(FILE_PHOTO);      
    }
    file.close();
    esp_camera_fb_return(fb);

    //check if saved properly
    ok =checkPhoto(SPIFFS);
  }while(!ok);
}

String readTemperature(){
  //for testing purposes, random generated temperature values
  int t = rand() % (20-18+1)+ 18;
  Serial.println(t);
  return String(t);
}
String getTemperature(){
  float t1=dht1.readTemperature();
  float t2 = dht2.readTemperature();
  float avgTemp = (t1 + t2)/2;
  float filteredTemperature = kf_temp.updateEstimate(avgTemp);
  float temp = filteredTemperature*100;
  Serial.println(temp);
  return String(temp);
}

String readHumidty(){
  //for testing purposes, random generated himidity values
  int h= rand()%(93-88+1)+ 88;
  Serial.println(h);
  return String(h);
}

String getHumidty(){
  float h1 = dht1.readHumidity();
  float h2 =dht2.readHumidity() ;
  float avgHum = (h1 + h2)/2;
  float filteredHumidity = kf_hum.updateEstimate(avgHum);
  float hum = filteredHumidity*100;
  Serial.println(hum);
  return String(hum);
}

String readVoltage(){
  //for testing purposes, random generated himidity values
  float v= (rand()%(144-143+1)+143)/(12);
  Serial.println(v);
  return String(v);
}
String getVoltage(){
  int analogValue = analogRead(14);
  float voltage = (float)analogValue * (10.74/4095) + 1.27;
  float filteredVoltage = kf_volt.updateEstimate(voltage);
  float volt = filteredVoltage*100;
  Serial.println(volt);
  return String(volt);
}

void setup() {
  // Setup code
  Serial.begin(115200);
  //initialize inputs
  //pinMode(tempPin, INPUT);

  // dht1.begin();
  // dht2.begin();
  // format SPIFFS
  //formatSPIFFS();
  //initialize SPIFFS
  initSPIFFS();
  
  //initialize WiFi
  initWiFi();
  //initialize Camera
  initCamera();
  server.serveStatic("/", SPIFFS, "/");
  //Route to home page upon request
  server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!request-> authenticate(http_username, http_password)){
      return request->requestAuthentication();
    }
    log_count=log_count+1;
    request->send(SPIFFS, "/index.html", "text/html");

  });
  //

  server.on("/images", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(log_count<1){
      return request-> requestAuthentication();
    }
    else{
      request->send(SPIFFS, "/image.html", "text/html");
    }
  });


  //Load css file
  // server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/style.css", "text/css");
  // });

  //Route to sensor page
  server.on("/data",HTTP_GET, [](AsyncWebServerRequest *request){
    if(log_count<1){
      return request->requestAuthentication();
    }
    else{
      request->send(SPIFFS, "/data.html", "text/html");
    } 
  });
  // send logo
  server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });
  server.on("/uctLogo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/uctLogo.png", "image/png");
  });
  //logout page on logout
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if(log_count==0){
      request->send(SPIFFS, "/login.html", "text/html");
    }else{
      log_count=log_count-1;
      request->send(SPIFFS, "/login.html", "text/html");
    }
    
    
  });
  //take a picture on capture request
  server.on("/capture", HTTP_GET, [](AsyncWebServerRequest *request){
    newPhotoTaken= true;
    request->send_P(200, "text/plain", "Taking Photo");
  });

  //send image upon request for the user to view on the web App
  server.on("/captured-image", HTTP_GET,[](AsyncWebServerRequest *request){
    request-> send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });
  //send image upon request for download
  server.on("/picture-download", HTTP_GET, [](AsyncWebServerRequest *request){
    request-> send(SPIFFS, FILE_PHOTO, "image/jpg", true);
  });
  //send voltage data upon request on the /voltage url for the graphs
  server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readVoltage().c_str());
  });
  //send temperature data upon request on the /tempearture url for the graph
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemperature().c_str());
  });

  //send humidity data upon request on the /humidity url for the graphs
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumidty().c_str());
  });

  //request latest sensor readings for the gauges
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    String json_data = getSensorReadings();
    request->send(200, "application/json", json_data);
    Serial.print("Sending data :");
    Serial.println(json_data);
    json_data= String();
  });

  // //load javascript file
  // server.on("/layout.js", HTTP_GET, [](AsyncWebServerRequest *request){
  //   request->send(SPIFFS, "/layout.js", "text/js");
  // });
  //download data in the txt file
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
    //
    request->send(SPIFFS, "/data.txt", "application/txt", true);
  });

  //send event when there's one
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client connected! with ID: %u\n",client->lastId());
    }
    //send event with message
    client->send("welcome to the ESP32 webserver!", NULL, millis(),10000);
  });
  server.addHandler(&events);
  
  //start server
  server.begin();
}

void loop() {
  //send events to the client every 30 seconds
  currentTime = millis();
  if((currentTime-prevTime)>timeOutTime) {
    events.send("ping",NULL,millis());
    // getSensorReadings();
    // events.send((JSON.stringify(readings["temperature"])).c_str(), "temperature", millis());
    // events.send((JSON.stringify(readings["humidity"])).c_str(), "humidity", millis());
    // events.send((JSON.stringify(readings["voltage"])).c_str(), "voltage", millis());
    events.send(getSensorReadings().c_str(), "new_readings", millis());
    prevTime= millis();
  }
  //take picture upon button press
  if(newPhotoTaken){
    captureAndSave();
    newPhotoTaken=false;
  }
  delay(1);
  
}

