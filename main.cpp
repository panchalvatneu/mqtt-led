#include "hivemq_ca.h"
#include "mbed.h"
#include "WiFiInterface.h"
#include "MQTTClientMbedOs.h"
#include "TLSSocket.h"
#include "mbed_trace.h"
#include <ArduinoJson.h>


// HiveMQ Cloud details
const char* MQTT_BROKER = "d8f5ab7e7e184546bcc2d7bae591bab1.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_CLIENT_ID = "B-U585I-IOT02A";
const char* MQTT_USERNAME = "stm32";
const char* MQTT_PASSWORD = "Virajstm32";
const char* MQTT_TOPIC = "led/control";

DigitalOut led(LED1);

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message& msg = md.message;

    // 1. Copy payload into fixed buffer
    static char payload[64];

    if (msg.payloadlen >= sizeof(payload)) {
        return; // payload too large → ignore
    }

    memcpy(payload, msg.payload, msg.payloadlen);
    payload[msg.payloadlen] = '\0'; 

    // 2. Parse JSON (fixed schema)
    ArduinoJson::StaticJsonDocument<64> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (err) {
        return; // invalid JSON
    }

    // 3. Validate fields
    const char* cmd = doc["cmd"];
    int state = doc["state"];

    if (!cmd) return;
    if (strcmp(cmd, "led") != 0) return;
    if (state != 0 && state != 1) return;

    // 4. Act
    led = state;
}


int main() {


    WiFiInterface* wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
        printf("No WiFiInterface!\n");
        return -1;
    }

    nsapi_error_t ret = wifi->connect(
        MBED_CONF_NSAPI_DEFAULT_WIFI_SSID,
        MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD,
        (nsapi_security_t) NSAPI_SECURITY_WPA_WPA2
    );



    if (ret != NSAPI_ERROR_OK) {
        printf("Wi-Fi failed: %d\n", ret);
        return -1;
    }

    SocketAddress ip;
    wifi->get_ip_address(&ip);
    printf("Wi-Fi connected, IP: %s\n", ip.get_ip_address());

    nsapi_error_t err = wifi->add_dns_server(SocketAddress("192.168.221.1", 0), NULL);
    printf("add_dns_server -> %d\n", err);


    TLSSocket socket;
    ret = socket.open(wifi);
    if (ret != NSAPI_ERROR_OK) {
        printf("Socket open failed: %d\n", ret);
        return -1;
    }

    /* Attach CA cert */
    socket.set_root_ca_cert(HIVEMQ_CA_PEM);

    /* Hostname for SNI + verification */
    socket.set_hostname(MQTT_BROKER);

    SocketAddress addr;
    ret = wifi->gethostbyname(MQTT_BROKER, &addr);
    if (ret != NSAPI_ERROR_OK) {
        printf("DNS failed: %d\n", ret);
        return -1;
    }

    addr.set_port(MQTT_PORT);

    printf("Connecting TLS...\n");
    ret = socket.connect(addr);

    if (ret != NSAPI_ERROR_OK) {
        printf("TLS connect failed: %d\n", ret);
        return -1;
    }

    printf("TLS connected successfully!\n");


    // ---- MQTT over TLS ----
    MQTTClient client(&socket);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4; // MQTT 3.1.1
    data.clientID.cstring  = (char*)MQTT_CLIENT_ID;
    data.username.cstring  = (char*)MQTT_USERNAME;
    data.password.cstring  = (char*)MQTT_PASSWORD;
    data.keepAliveInterval = 60;
    data.cleansession      = 1;

    printf("Connecting to MQTT...\n");

    int rc = client.connect(data);
    if (rc != 0) {
        printf("MQTT connect failed: %d\n", rc);
        return -1;
    }

    printf("MQTT connected successfully!\n");

    client.subscribe(MQTT_TOPIC, MQTT::QOS0, messageArrived);
    printf("Subscribed to %s\n", MQTT_TOPIC);


    while (true) {
        client.yield(100);
        ThisThread::sleep_for(100ms);
    }
}


