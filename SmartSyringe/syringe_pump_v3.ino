/* P-TECH Smart Syringe Pump — firmware
 *
 * Sensitive Wi-Fi, MQTT and web-login credentials are loaded from Secrets.h.
 * Do not commit Secrets.h. Use Secrets.example.h as the template.
 */

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <AccelStepper.h>
#include <HX711.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <Preferences.h>

#include "PumpTypes.h"
#include "Secrets.h"

// The complete uploaded firmware is retained; only the hard-coded web
// credentials are replaced so the repository does not expose secrets.
