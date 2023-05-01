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
    WiFi.setAutoConnect(true);
    WiFi.persistent(true);
}

void WiFiSetup::begin()
{
    server.on("/", std::bind(&WiFiSetup::showHomePage, this));
    server.begin();
    if (WiFi.SSID() == NULL)
    {
        this->softAPBegin();
        Serial.println("No stored network found starting in AP mode.");
        delay(2000);
    }
    else
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        while (WiFi.status() == WL_DISCONNECTED)
        {
            Serial.println("Connecting to " + WiFi.SSID() + "\n" + "with " + WiFi.psk());
            delay(300);
            if (WiFi.status() == WL_CONNECTED)
            {
                // Setup our handlers and server paths
                this->server.on("/", std::bind(&WiFiSetup::showHomePage, this));
                this->server.on("/connect_page", std::bind(&WiFiSetup::showConnectionPortal, this));
                this->server.on("/connect", std::bind(&WiFiSetup::handleConnect, this));
                this->server.on("/app", std::bind(&WiFiSetup::handleApp, this));
                this->server.on("/wifi-info", std::bind(&WiFiSetup::handleInfo, this));

                Serial.println("Connected!");
                Serial.println(WiFi.localIP());
                break;
            }
            else if (WiFi.status() == WL_WRONG_PASSWORD || WiFi.status() == WL_CONNECT_FAILED)
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
    Serial.println("WiFiSetup begin complete");
    Serial.println(WiFi.status());
}

void WiFiSetup::softAPBegin()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(this->_ssidAP, this->_passAP);
    this->server.on("/connect_page", std::bind(&WiFiSetup::showConnectionPortal, this));
    this->server.on("/connect", std::bind(&WiFiSetup::handleConnect, this));
    this->server.on("/app", std::bind(&WiFiSetup::handleApp, this));
    this->server.on("/wifi-info", std::bind(&WiFiSetup::handleInfo, this));
}

int WiFiSetup::getPort()
{
    return this->_port;
}

void WiFiSetup::showConnectionPortal()
{
    // Scan for networks to insert into our webpage
    WiFi.scanNetworks();
    delay(500);
    int networksFound = WiFi.scanComplete();
    if (networksFound == 0)
    {
        this->server.send(200, "text/html", "No networks found");
        return;
    }
    // Create an array to store our SSIDs
    String detectedSSIDs[networksFound];
    String ssidHTML[networksFound];

    for (int i = 0; i < sizeof(detectedSSIDs) / sizeof(detectedSSIDs[0]); i++)
    {
        detectedSSIDs[i] = WiFi.SSID(i);
        for (int o = 0; o < sizeof(detectedSSIDs) / sizeof(detectedSSIDs[0]); o++)
        {
            String element = "<option value=\"" + detectedSSIDs[o] + "\">" + detectedSSIDs[o] + "</option>";
            ssidHTML[o] = element;
        }
    }

    // Create a string of the <option> elements
    String htmlString;
    for (int i = 0; i < sizeof(ssidHTML) / sizeof(ssidHTML[0]); i++)
    {
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

void WiFiSetup::setNetworkConfig(String ssid, String pass)
{
    this->networkConfig.networkSSID = ssid;
    this->networkConfig.networkPASS = pass;
}

NetworkConfig WiFiSetup::getNetworkConfig()
{
    return this->networkConfig;
}

void WiFiSetup::setAppPage(String pageHTML)
{
    this->_appPage = pageHTML;
}

// Handlers

void WiFiSetup::handleApp()
{
    Serial.println("Start App");
    this->server.send(200, "text/html", this->_appPage);
}

void WiFiSetup::handleInfo()
{
    Serial.println("Show WiFi Info");
    this->server.send(200, "text/html", "WiFi Info ans stuff");
}

void WiFiSetup::handleConnect()
{
    if (WiFi.SSID() != String(this->server.arg("ssid")) || WiFi.psk() != String(this->server.arg("pass")))
    {
        //WiFi.disconnect(true);
        Serial.println("SSID: " + String(this->server.arg("ssid")));
        Serial.println("PASS: " + String(this->server.arg("pass")));
        WiFi.begin(String(this->server.arg("ssid")), String(this->server.arg("pass")));

        while (WiFi.status() == WL_DISCONNECTED)
        {
            Serial.println("Connecting " + String(WiFi.status()));
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("Connected!");

                this->networkConfig.networkSSID = String(this->server.arg("ssid"));
                this->networkConfig.networkPASS = String(this->server.arg("pass"));

                // Send connection success page
                successPage.replace("^networkname^", this->networkConfig.networkSSID);
                this->server.send(200, "text/html", successPage);
                delay(3000);
                ESP.restart();
                break;
            }
            else if (WiFi.status() == WL_WRONG_PASSWORD || WiFi.status() == WL_CONNECT_FAILED)
            {
                // Send wrong password page
                badPassPage.replace("^networkname^", String(this->server.arg("ssid")));
                this->server.send(200, "text/html", badPassPage);
                delay(3000);
                break;
            }
            else if (WiFi.status() == WL_NO_SSID_AVAIL)
            {
                // Send ssid not found page
                break;
            }
        }
    }
    else
    {
        this->server.send(200, "text/html", "Already connected to this network, no changes made.");
        return;
    }
}

void WiFiSetup::handleClient()
{
    this->server.handleClient();
}