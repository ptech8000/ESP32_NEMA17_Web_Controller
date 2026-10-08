#ifndef SECRETS_EXAMPLE_H
#define SECRETS_EXAMPLE_H

// Copy this file locally as Secrets.h and replace every placeholder.
// NEVER commit Secrets.h to GitHub.

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST     = "YOUR_HIVEMQ_CLUSTER";
const uint16_t MQTT_PORT  = 8883;
const char* MQTT_USERNAME = "YOUR_MQTT_USERNAME";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

const char* WEB_USERNAME  = "YOUR_WEB_USERNAME";
const char* WEB_PASSWORD  = "YOUR_WEB_PASSWORD";

const char* HIVEMQ_ROOT_CA = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE_YOUR_ROOT_CA_CERTIFICATE_HERE
-----END CERTIFICATE-----
)EOF";

#endif
