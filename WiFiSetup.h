/*
DOC STRING
*/
#ifndef WiFiSetup_h
#define WiFiSetup_h
#include <Arduino.h>
#include <ESP8266WebServer.h>

struct NetworkConfig
{
  String networkSSID;
  String networkPASS;
};

class WiFiSetup
{
    public:
        // If no port is given, we are in setup AP mode
        WiFiSetup(String ssidAP, String passAP, int port);
        void begin();
        int getPort();
        void handleClient();
        void handleConnect();
        void handleInfo();
        void handleApp();
        void showConnectionPortal();
        void showHomePage();
        void setNetworkConfig(String ssid, String pass);
        void softAPBegin();
        void setAppPage(String pageHTML);
        NetworkConfig getNetworkConfig();
        ESP8266WebServer server = {80};
        
    private:
        int _port;
        String _ssidAP;
        String _passAP;
        String _appPage = "Insert your HTML here.";
        NetworkConfig networkConfig;  
};
#endif