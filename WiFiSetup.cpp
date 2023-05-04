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

void WiFiSetup::clearEEPROM(int size)
{
    for (int i = 0; i < size; i++)
    {
        EEPROM.write(i, 0);
    }

    EEPROM.commit();
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

void WiFiSetup::setAppPage(String pageHTML)
{
    this->_appPage = pageHTML;
}

void WiFiSetup::setDeviceConfig(DeviceConfig config)
{
    this->deviceConfig = config;
}

DeviceConfig WiFiSetup::getDeviceConfig()
{
    return this->deviceConfig;
}

// Handlers

void WiFiSetup::handleApp()
{
    Serial.println("Start App");
    this->server.send(200, "text/html", this->_appPage);
}

void WiFiSetup::handleInfo()
{
    DeviceConfig config = this->readDeviceConfig();
    Serial.println("Show WiFi Info");
    Serial.println(config.deviceName);
    Serial.println(config.deviceDescription);
    // Replace populate device info
    deviceInfoPage.replace("^devicename^", config.deviceName);
    deviceInfoPage.replace("^devicedescr^", config.deviceDescription);
    deviceInfoPage.replace("^wifissid^", WiFi.SSID());
    deviceInfoPage.replace("^deviceip^", WiFi.localIP().toString());
    deviceInfoPage.replace("^networkip^", WiFi.gatewayIP().toString());
    this->server.send(200, "text/html", String(deviceInfoPage));
}

void WiFiSetup::handleConnect()
{
    if (WiFi.SSID() != String(this->server.arg("ssid")) || WiFi.psk() != String(this->server.arg("pass")))
    {
        WiFi.disconnect(true);
        Serial.println("SSID: " + String(this->server.arg("ssid")));
        Serial.println("PASS: " + String(this->server.arg("pass")));
        Serial.println("Device Name: " + String(this->server.arg("device-name")));
        Serial.println("Device Description: " + String(this->server.arg("device-descr")));
        WiFi.begin(String(this->server.arg("ssid")), String(this->server.arg("pass")));

        while (WiFi.status() == WL_DISCONNECTED)
        {
            Serial.println("Connecting " + String(WiFi.status()));
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("Connected!");

                DeviceConfig config;

                //config.deviceName = String(this->server.arg("device-name"));
                //config.deviceDescription = String(this->server.arg("device-descr"));

                this->server.arg("device-name") ? config.deviceName = String(this->server.arg("device-name")) : config.deviceName = "";
                this->server.arg("device-descr") ? config.deviceDescription = String(this->server.arg("device-descr")) : config.deviceDescription = "";

                this->setDeviceConfig(config);
                this->saveDeviceConfig() ? Serial.println("Device Config Saved!") : Serial.println("Failed to Save Device Config...");

                // Send connection success page
                successPage.replace("^networkname^", String(this->server.arg("ssid")));
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

// EEPROM Stuff

bool WiFiSetup::saveDeviceConfig()
{
    DeviceConfig config = this->getDeviceConfig();
    int deviceName_StrLen = config.deviceName.length() + 1;
    int deviceDescr_StrLen = config.deviceDescription.length() + 1;
    int startAddress_DevName = this->DEV_NAME_ADDRESS_START;
    int startAddress_DevDescr = this->DEV_DESCR_ADDRESS_START;

    // Create char arrays for a buffer
    char deviceName_CharArray[deviceName_StrLen];
    char deviceDescr_CharArray[deviceDescr_StrLen];

    config.deviceName.toCharArray(deviceName_CharArray, deviceDescr_StrLen);
    config.deviceDescription.toCharArray(deviceDescr_CharArray, deviceDescr_StrLen);

    Serial.println("Recieved Device Name: " + config.deviceName);
    Serial.println("Recieved Device Description: " + config.deviceDescription);
    delay(5000);

    EEPROM.begin(300);

    // Write device name if not empty
    if (! config.deviceName.isEmpty())
    {
        //this->clearEEPROM(this->DEV_NAME_ADDRESS_MAX);
        for (int i = 0; i < sizeof(deviceName_CharArray) / sizeof(deviceName_CharArray[0]); i++)
        {
            delay(500);
            EEPROM.write(startAddress_DevName, deviceName_CharArray[i]);
            Serial.print(deviceName_CharArray[i]);
            Serial.print(" save at ");
            Serial.println(startAddress_DevName);
            startAddress_DevName++;
        }
    }

    // Write device description if not empty
    if (! config.deviceDescription.isEmpty())
    {
        //this->clearEEPROM(this->DEV_DESCR_ADDRESS_MAX);
        for (int i = 0; i < sizeof(deviceDescr_CharArray) / sizeof(deviceDescr_CharArray[i]); i++)
        {
            EEPROM.write(startAddress_DevDescr, deviceDescr_CharArray[i]);
            Serial.print(deviceDescr_CharArray[i]);
            Serial.print(" save at ");
            Serial.println(startAddress_DevDescr);
            startAddress_DevDescr++;
        }
    }

    EEPROM.commit();
    if (!EEPROM.end())
    {
        return false;
    }
    return true;
}

DeviceConfig WiFiSetup::readDeviceConfig()
{
    // Create buffer strings to hold read strings
    String deviceName_BuffString;
    String deviceDescript_BuffString;

    int startAddress_DevName = this->DEV_NAME_ADDRESS_START;
    int startAddress_DevDescr = this->DEV_DESCR_ADDRESS_START;

    EEPROM.begin(300);
    char deviceName_CharCurrent = EEPROM.read(startAddress_DevName);
    char deviceDescr_CharCurrent = EEPROM.read(startAddress_DevDescr);

    // Read deviceName
    while (deviceName_CharCurrent != '\0' && startAddress_DevName < this->DEV_NAME_ADDRESS_MAX)
    {
        deviceName_CharCurrent = EEPROM.read(startAddress_DevName);
        Serial.print(deviceName_CharCurrent);
        Serial.print("read at");
        Serial.println(startAddress_DevName);
        if (deviceName_CharCurrent == '\0')
            break;
        deviceName_BuffString += deviceName_CharCurrent;
        startAddress_DevName++;
    }

    // Read deviceDescription
    while (deviceDescr_CharCurrent != '\0' && startAddress_DevDescr < this->DEV_DESCR_ADDRESS_MAX)
    {
        deviceDescr_CharCurrent = EEPROM.read(startAddress_DevDescr);
        if (deviceDescr_CharCurrent == '\0')
            break;
        deviceDescript_BuffString += deviceDescr_CharCurrent;
        startAddress_DevDescr++;
    }

    DeviceConfig devConfig;

    Serial.println("Device Name " + deviceName_BuffString);
    devConfig.deviceName = deviceName_BuffString;

    Serial.println("Device Description " + deviceDescript_BuffString);
    devConfig.deviceDescription = deviceDescript_BuffString;

    EEPROM.end();
    return devConfig;
}