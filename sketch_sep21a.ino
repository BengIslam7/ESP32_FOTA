#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "*************";
const char* password = "**************";

const char* currentversion = "1.0.0";

// Change this URL for each new GitHub Release
const char* newfirmwareURL =
    "https://github.com/********/********/releases/download/*********/firmware.ino.bin";


void setup()
{
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");

    // HTTPS client
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    // GitHub uses redirects
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

    Serial.println("Cheking if New Firmware Version is Available...");
    Serial.println(newfirmwareURL);

    if (!http.begin(client, newfirmwareURL))
    {
        Serial.println("HTTP connection failed");
        return;
    }

    else { 
        
        int httpCode = http.GET();

        Serial.printf("HTTP Code: %d\n", httpCode);

        if (httpCode != HTTP_CODE_OK)
        {
            Serial.println("New Firmware Version not AVAILABLE");
            http.end();
        }

        else {

            int contentLength = http.getSize();

            Serial.printf(
                "Firmware size: %d bytes\n",
                contentLength
            );

            if (contentLength <= 0)
            {
                Serial.println("Invalid firmware size");
                http.end();
                return;
            }

            // Start OTA

            Serial.println("Starting OTA...");
            
            if (!Update.begin(contentLength))
            {
                Serial.println("Not enough space for OTA");
                Update.printError(Serial);
                http.end();
                return;
            }

            Serial.println("Downloading firmware...");

            WiFiClient* stream = http.getStreamPtr();

            size_t written = Update.writeStream(*stream);

            Serial.printf(
                "Written: %d / %d bytes\n",
                written,
                contentLength
            );

            if (written != contentLength)
            {
                Serial.println("Firmware download incomplete");

                Update.abort();
                http.end();

                return;
            }

            // Finish OTA
            if (!Update.end())
            {
                Serial.println("OTA update failed");

                Update.printError(Serial);

                http.end();

                return;
            }

            if (!Update.isFinished())
            {
                Serial.println("OTA not finished");

                http.end();

                return;
            }

            Serial.println();
            Serial.println("OTA UPDATE SUCCESSFUL!");
            Serial.println("Rebooting...");

            http.end();

            delay(2000);

            ESP.restart();

        }
    }
}


void loop()
{
}
