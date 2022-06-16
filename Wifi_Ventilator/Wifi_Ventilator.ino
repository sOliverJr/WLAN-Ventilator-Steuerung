// Load Wi-Fi library
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
int state = 0;

// Assign output variables to GPIO pins
// Reversed functionality (low -> on, high -> off)
// Connect Realay to NO & COM
// That way, if the arduino is off/not connected to power, the ventilator is on
const int relay = 5;          
const int lever = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Define NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Vars for ventilator-timer
unsigned long ventStartTime = 0;
unsigned long ventCurrentTime = 0;

// Get_Unix_Time() Function that gets current epoch time
unsigned long Get_Unix_Time() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void Check_For_State() {
  // turns the GPIOs on and off
  if (state == 0) {
    digitalWrite(relay, HIGH);
    ventCurrentTime = 0;
    ventStartTime = 0;
    
  } else if (state == 1) {
    digitalWrite(relay, LOW);
    ventCurrentTime = 0;
    ventStartTime = 0;
    
  } else if (state == 2) {
    if (ventStartTime == 0){
      ventStartTime = Get_Unix_Time();
      digitalWrite(relay, LOW);
      };
    ventCurrentTime = Get_Unix_Time();
    if((ventCurrentTime - ventStartTime) > (60*15)){
      digitalWrite(relay, HIGH);
      state = 0;
    };
    
  } else if (state == 3) {
    if (ventStartTime == 0){
      ventStartTime = Get_Unix_Time();
      digitalWrite(relay, LOW);
      };
    ventCurrentTime = Get_Unix_Time();
    if((ventCurrentTime - ventStartTime) > (60*30)){
      digitalWrite(relay, HIGH);
      state = 0;
    };
    
  } else if (state == 4) {
    if (ventStartTime == 0){
      ventStartTime = Get_Unix_Time();
      digitalWrite(relay, LOW);
      };
    ventCurrentTime = Get_Unix_Time();
    if((ventCurrentTime - ventStartTime) > (60*60)){
      digitalWrite(relay, HIGH);
      state = 0;
    };
    
  } else if (state == 5) {
    if (ventStartTime == 0){
      ventStartTime = Get_Unix_Time();
      digitalWrite(relay, LOW);
      };
    ventCurrentTime = Get_Unix_Time();
    if((ventCurrentTime - ventStartTime) > (60*1)){
      digitalWrite(relay, HIGH);
      state = 0;
    };
  }
}


void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(relay, OUTPUT);
  pinMode(lever, OUTPUT);
  // Set outputs
  digitalWrite(relay, HIGH);
  digitalWrite(lever, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /0") >= 0) {
              Serial.println("Ventilator aus");
              state = 0;
            } else if (header.indexOf("GET /1") >= 0) {
              Serial.println("Ventilator an");
              state = 1;
            } else if (header.indexOf("GET /2") >= 0) {
              Serial.println("Ventilator für 15min an");
              state = 2;
            } else if (header.indexOf("GET /3") >= 0) {
              Serial.println("Ventilator für 30min an");
              state = 3;
            } else if (header.indexOf("GET /4") >= 0) {
              Serial.println("Ventilator für 60min an");
              state = 4;
            } else if (header.indexOf("GET /5") >= 0) {
              Serial.println("Ventilator für 1min an");
              state = 5;
            }
            
                // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><title>Ventilator</title> <meta name=\"viewport\" content=\"width=device-width, initial-scale=0.75\"></head>");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            
                  // CSS to style the on/off buttons 
            client.println("<style>");
            client.println("@import url('https://fonts.googleapis.com/css2?family=Slabo+27px&display=swap');");
            client.println("body{margin: 0em; padding: 0em; border: 0em;background-color: #2b2d42;min-width:350px; width: auto !important; width:350px; min-height:650px;height: auto !important; height:650px;}");
            client.println("#inhalt{ width: 100%; height: 100%; position: relative;}");
            client.println("button{ width: 90%; top: 0px; color: white;  padding: 14px; text-decoration: none;  font-size: 30px;  margin: 2px;  cursor: pointer; border-radius: 12px; margin: 0; margin-top: 18px; position: relative; top: 50%; left: 50%; -ms-transform: translate(-50%, -50%); transform: translate(-50%, -50%); border: none;}");
            client.println(".selectable {  background-color: #003566;}");
            client.println(".unselectable { background-color: #8d99ae; }");
            client.println(".selected{ background-color: #e07a5f;}");
            client.println(".title { font-family: 'Slabo', serif; font-size: 3em; color: rgb(235, 235, 235); text-align: center; margin: 5%; margin-bottom: 12%;}");
            client.println(".untertitel { font-family: 'Slabo', serif; font-size: 2em; color: #dee2e6; text-align: center; margin: 10%; margin-bottom: 8%;}");
            client.println("#copyright {position: absolute; margin: 0em; font-size: 1.25em; font-style: italic;  color: rgb(235, 235, 235);  left: 50%;  bottom: 2px;  transform: translate(-50%, -50%);}");
            client.println("</style>");
            
            // Web Page Heading
            client.println("<body>");
            client.println("<div id=\"inhalt\">");
            client.println("<p class=\"title\">Ventilator Interface</p>");
            client.println("<div class=\"box\">");
            client.println("<p class=\"untertitel\">Toggle</p>");
                 
            if (state == 0) {
              client.println("<a href= \"/1\"><button class=\"selectable\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable selected\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"/2\"><button class=\"selectable\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"/3\"><button class=\"selectable\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"/4\"><button class=\"selectable\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"/5\"><button class=\"selectable\" id=\"min1\">1 Min</button></a>");
            } else if (state == 1){
              client.println("<a href= \"/1\"><button class=\"selectable selected\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"/0\"><button class=\"selectable\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"/2\"><button class=\"selectable\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"/3\"><button class=\"selectable\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"/4\"><button class=\"selectable\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"/5\"><button class=\"selectable\" id=\"min1\">1 Min</button></a>");
            } else if (state == 2){
              client.println("<a href= \"/1\"><button class=\"selectable\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"/0\"><button class=\"selectable\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"\"><button class=\"unselectable selected\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min1\">1 Min</button></a>");
            } else if (state == 3){
              client.println("<a href= \"/1\"><button class=\"selectable\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"/0\"><button class=\"selectable\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable selected\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min1\">1 Min</button></a>");
            } else if (state == 4){
              client.println("<a href= \"/1\"><button class=\"selectable\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"/0\"><button class=\"selectable\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable selected\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min1\">1 Min</button></a>");
            } else if (state == 5){
              client.println("<a href= \"/1\"><button class=\"selectable\" id=\"on\">On</button></a></br>");
              client.println("<a href= \"/0\"><button class=\"selectable\" id=\"off\">Off</button></a>");
              client.println("</div>");
              client.println("<div class=\"box\"><p class=\"untertitel\">Timer</p>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min15\">15 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min30\">30 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable\" id=\"min60\">60 Min</button></a></br>");
              client.println("<a href= \"\"><button class=\"unselectable selected\" id=\"min1\">1 Min</button></a>");
            }

            client.println("</div>");
            client.println("</div>");
            client.println("<p id=\"copyright\">Oliver Seider Jr</p>");
            client.println("</body>");
               
            Check_For_State();
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  Check_For_State();
}
