#include "TCPSocket.h"
#include "MQTTClient.h"
#include "mbed.h"
#include "WiFiInterface.h"



int main() {
    printf("Booting...\n");

    WiFiInterface *wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("No WiFiInterface found (EMW3080B not registered)\n");
        return -1;
    }

    

    nsapi_error_t ret = wifi->connect(MBED_CONF_NSAPI_DEFAULT_WIFI_SSID, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD, (nsapi_security_t)NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("Wi-Fi connection failed: %d\n", ret);
        return -1;
    }

    SocketAddress ip;
    wifi->get_ip_address(&ip);
    printf("Connected! IP address: %s\n", ip.get_ip_address());

    while (true) {
        ThisThread::sleep_for(1000ms);
        printf("tick\n");
    }
}
