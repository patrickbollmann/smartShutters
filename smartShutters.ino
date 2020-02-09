/*********
author = "Patrick Bollmann"
email = "pbollman@mail.upb.de"
date = "09.02.2020"
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "Password";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String currentState = "up";

// Assign output variables to GPIO pins
const int output5 = D5; //up
const int output4 = D4; //down

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void up(){
  digitalWrite(output4, HIGH);
  delay(200);
  digitalWrite(output5, LOW);
  
}
void down(){
  digitalWrite(output5, HIGH);
  delay(200);
  digitalWrite(output4, LOW);
  
}
void stop(){
  digitalWrite(output5, HIGH);
  digitalWrite(output4, HIGH);
  
}
void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to HIGH (high means disconnected for my relay module)
  digitalWrite(output5, HIGH);
  digitalWrite(output4, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
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
  up();
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
            if (header.indexOf("GET /goup") >= 0) {
              Serial.println("going up");
              currentState="up";
              up();
            } else if (header.indexOf("GET /godown") >= 0) {
              Serial.println("going down");
              currentState="down";
              down();
            } else if (header.indexOf("GET /stop") >= 0) {
              Serial.println("stop");
              stop();
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}");
            client.println(".button3 {background-color: #FF6A6A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Smart Rollladen</h1>");
            
            // Display current state, up down button depending on state     
            if (currentState=="up") {
              client.println("<p><a href=\"/godown\"><button class=\"button\">Herunterfahren</button></a></p>");
            } else {
              client.println("<p><a href=\"/goup\"><button class=\"button button2\">Hochfahren</button></a></p>");
            }
            // Display the stop button
            client.println("<p><a href=\"/stop\"><button class=\"button button3\">Stopp</button></a></p>");
               
            client.println("</body></html>");
            
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
}
