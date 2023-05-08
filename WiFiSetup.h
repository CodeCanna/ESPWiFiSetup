/*
DOC STRING
*/
#ifndef WiFiSetup_h
#define WiFiSetup_h
#include <Arduino.h>
#include <ESP8266WebServer.h>

struct DeviceConfig
{
  String deviceName;
  String deviceDescription;
};

class WiFiSetup
{
    public:
        // If no port is given, we are in setup AP mode
        WiFiSetup(String ssidAP, String passAP, int port);
        // Begins things :)
        void begin();
        int getPort();
        bool saveDeviceConfig();
        bool isConnected();
        DeviceConfig readDeviceConfig();
        void handleClient();
        void handleConnect();
        void handleInfo();
        void handleApp();
        void showConnectionPortal();
        void showHomePage();
        void setNetworkConfig(String ssid, String pass);
        void softAPBegin();
        void setAppPage(String pageHTML);
        void setDeviceConfig(DeviceConfig config);
        void clearEEPROM(int size);
        DeviceConfig getDeviceConfig();
        ESP8266WebServer server = {80};
        
    private:
        int _port;
        const int DEV_NAME_ADDRESS_START = 50;
        const int DEV_NAME_ADDRESS_MAX = 100;
        const int DEV_DESCR_ADDRESS_START = 101;
        const int DEV_DESCR_ADDRESS_MAX = 201;
        
        String _ssidAP;
        String _passAP;
        String _appPage = "Insert your HTML here.";
        DeviceConfig deviceConfig;
};
#endif