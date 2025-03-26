#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "Jarvis";
const char *password = "Boyz3034";
const char *apiKey = "NNH5AZD2ERO6D1UH";

WiFiServer server(80);

String tempData = "0";
String rainData = "Unknown";

// Timing variables
unsigned long previousThingSpeak = 0;
unsigned long previousSerialCheck = 0;
const long tsInterval = 15000;  // 15 seconds for ThingSpeak
const long serialInterval = 200; // Check serial every 200ms

void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop() {
    // Check for serial data from Arduino
    if (millis() - previousSerialCheck >= serialInterval) {
        previousSerialCheck = millis();
        while (Serial.available()) {
            String input = Serial.readStringUntil('\n');
            input.trim();  // Clean trailing spaces and new lines
            Serial.println("Received from Arduino: " + input);
            
            int commaIndex = input.indexOf(',');
            if (commaIndex != -1) {
                tempData = input.substring(0, commaIndex);
                rainData = input.substring(commaIndex + 1);
                rainData.trim();
                Serial.println("Parsed Temp: " + tempData);
                Serial.println("Parsed Rain: " + rainData);
            }
        }
    }

    // Send data to ThingSpeak periodically
    if (millis() - previousThingSpeak >= tsInterval) {
        previousThingSpeak = millis();
        sendToThingSpeak(tempData, rainData);
    }

    // Web server handling
    WiFiClient client = server.available();
    if (client) {
        Serial.println("New client connected");
        handleClient(client);
    }
}

void handleClient(WiFiClient &client) {
    unsigned long timeout = millis() + 250; 
    String request;

    while (client.connected() && millis() < timeout) {
        if (client.available()) {
            request = client.readStringUntil('\r');
            client.flush();
            break;
        }
    }

    if (request.indexOf("GET / HTTP") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println("Cache-Control: no-cache, no-store, must-revalidate");
    client.println("Pragma: no-cache");
    client.println("Expires: 0");
    client.println();
    
    client.println("<!DOCTYPE html><html><head>");
    client.println("<title>SOLAR MONITOR</title>");
    client.println("<meta http-equiv=\"refresh\" content=\"3\">");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
    client.println("<link href=\"https://fonts.googleapis.com/css2?family=Merriweather&display=swap\" rel=\"stylesheet\">");
    client.println("<style>");
    client.println("body { font-family: 'Merriweather', serif; text-align: center; margin-top: 30px; background: #dbdbff; }");
    client.println("h1 { color: #444; font-size: 36px; margin-bottom: 20px; }");
    client.println(".data { font-size: 24px; margin: 20px; padding: 20px; background: #f14a4a; display: inline-block; border-radius: 10px; border: 3px solid white; color: white; }");
    client.println(".github-logo { position: fixed; top: 50%; left: 50%; transform: translate(-50%, -50%); border-radius: 15px; padding: 8px; background: white; }");
    client.println(".github-logo img { width: 60px; height: 60px; border-radius: 15px; }");
    client.println("@media (max-width: 600px) {");
    client.println("body { margin-top: 15px; }");
    client.println("h1 { font-size: 28px; margin-bottom: 15px; }");
    client.println(".data { font-size: 18px; margin: 10px; padding: 12px; }");
    client.println(".github-logo { top: 45%; border-radius: 20px; }"); // Adjust position for mobile
    client.println(".github-logo img { width: 50px; height: 50px; border-radius: 20px; }");
    client.println("}");
    client.println("</style></head><body>");
    client.println("<h1>SOLAR MONITOR</h1>");
    client.println("<div class=\"data\">");
    client.println("<p><strong>Temperature:</strong> " + tempData + " °C</p>");
    client.println("<p><strong>Rain Status:</strong> " + rainData + "</p>");
    client.println("</div>");
    client.println("<a href=\"https://github.com/AyushChandekar/solar_monitoring_system\" target=\"_blank\" class=\"github-logo\">");
    client.println("<img src=\"https://github.githubassets.com/images/modules/logos_page/GitHub-Mark.png\" alt=\"GitHub Logo\">");
    client.println("</a>");
    client.println("</body></html>");
}




    client.stop();
    Serial.println("Client disconnected");
}

void sendToThingSpeak(String temp, String rain) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        const char *host = "api.thingspeak.com";
        
        String rainValue = (rain == "Raining") ? "1" : "0";

        if (client.connect(host, 80)) {
            String url = "/update?api_key=" + String(apiKey) + "&field1=" + temp + "&field2=" + rainValue;

            Serial.println("Sending data to ThingSpeak...");
            Serial.println(url);

            client.println("GET " + url + " HTTP/1.1");
            client.println("Host: " + String(host));
            client.println("Connection: close");
            client.println();

            delay(10);
            while (client.available()) {
                String line = client.readStringUntil('\r');
                Serial.print(line);
            }
            client.stop();
            Serial.println("\nData sent to ThingSpeak successfully!");
        } else {
            Serial.println("Failed to connect to ThingSpeak");
        }
    } else {
        Serial.println("WiFi Disconnected");
    }
}
