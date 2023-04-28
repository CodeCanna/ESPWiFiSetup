#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <functional>
#include <memory>
#include <EEPROM.h>
#include "WebPages.h"
#include "WiFiSetup.h"

WiFiSetup::WiFiSetup(String ssidAP, String passAP, int port) : server(port), _ssidAP(ssidAP), _passAP(passAP)
{
    Serial.println("Initialized");
}

void WiFiSetup::begin()
{
    server.on("/", std::bind(&WiFiSetup::showHomePage, this));
    server.begin();
    WiFi.begin("bean", "spode");
    while (WiFi.status() == WL_DISCONNECTED)
    {
        Serial.println("Connecting");
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connected!");
            break;
        }
        else if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("Wrong Pass");
            this->softAPBegin();
            break;
        }
        else if (WiFi.status() == WL_NO_SSID_AVAIL)
        {
            Serial.println("Wrong SSID");
            this->softAPBegin();
            break;
        }
    }
}

void WiFiSetup::softAPBegin()
{
    WiFi.softAP(this->_ssidAP, this->_passAP);
    server.on("/connect_page", std::bind(&WiFiSetup::showConnectionPortal, this));
    server.on("/connect", std::bind(&WiFiSetup::handleConnect, this));
    server.on("/setup", std::bind(&WiFiSetup::handleSetup, this));
    server.on("/app", std::bind(&WiFiSetup::handleApp, this));
    server.on("/wifi-info", std::bind(&WiFiSetup::handleInfo));
}

int WiFiSetup::getPort()
{
    return this->_port;
}

void WiFiSetup::showConnectionPortal()
{
    // Scan for networks to insert into our webpage
    int networksFound = WiFi.scanNetworks();
    if (networksFound == 0) {
        this->server.send(200, "text/html", "No networks found");
        return;
    }
    // Create an array to store our SSIDs
    String detectedSSIDs[networksFound];
    String ssidHTML[networksFound];

    for (int i = 0; i < sizeof(detectedSSIDs) / sizeof(detectedSSIDs[0]); i++) {
        detectedSSIDs[i] = WiFi.SSID(i);
        for (int o = 0; o < sizeof(detectedSSIDs) / sizeof(detectedSSIDs[0]); o++) {
            String element = "<option value=\"" + detectedSSIDs[o] + "\">" + detectedSSIDs[o] + "</option>";
            ssidHTML[o] = element;
        }
    }

      // Create a string of the <option> elements
    String htmlString;
    for(int i = 0; i < sizeof(ssidHTML) / sizeof(ssidHTML[0]); i++) {
        htmlString.concat(ssidHTML[i]);
    }
    // Inject custom HTML into networkSetupPage and serve it
    networkSetupPage.replace("<option>", htmlString);
    this->server.send(200, "text/html", networkSetupPage);
}

void WiFiSetup::showHomePage()
{
    this->server.send(200, "text/html", homePage);
}

void WiFiSetup::handleClient()
{
    this->server.handleClient();
}

void WiFiSetup::handleConnect()
{
    Serial.println("SSID: " + String(this->server.arg("ssid")));
    Serial.println("PASS: " + String(this->server.arg("pass")));
    WiFi.begin(String(this->server.arg("ssid")), String(this->server.arg("pass")));

    while(WiFi.status() == WL_DISCONNECTED)
    {
        Serial.println("Connecting " + String(WiFi.status()));
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("Connected!");

            this->networkConfig.networkSSID = String(this->server.arg("ssid"));
            this->networkConfig.networkPASS = String(this->server.arg("pass"));

            // Store this stuff
        }
        else if (WiFi.status() == WL_CONNECT_FAILED)
        {
            // Send wrong password page
        }
        else if (WiFi.status() == WL_NO_SSID_AVAIL)
        {
            // Send ssid not found page
        }
    }
}

void WiFiSetup::setNetworkConfig(String ssid, String pass)
{
    this->networkConfig.networkSSID = ssid;
    this->networkConfig.networkPASS = pass;
}

NetworkConfig WiFiSetup::getNetworkConfig()
{
    return this->networkConfig;
}

