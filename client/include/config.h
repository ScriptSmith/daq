#include <pgmspace.h>

#define THING_NAME "daq-Thing-XXXXXXXXXXXX"

#define ROOM "room1"

static boolean ENABLE_WIFI = false;

static const int TIME_ZONE = 10;

// $aws/rules/rule_name for basic ingest
// name topic for regular
static const char AWS_TOPIC[] = "$aws/rules/TopicRule_XXXXXXXXXXXX";

// MQTT endpoint
static const char AWS_IOT_ENDPOINT[] = "xxxxxxxxxxxxxx-ats.iot.us-east-2.amazonaws.com";

// Amazon Root CA
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_DEVICE[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)KEY";
