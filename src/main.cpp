#include <Wire.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>

// DHT22 config
DHT my_sensor(21, DHT22);
float temp, humid;

// Wifi config
const char *ssid = "";
const char *password = "";
const char *mqtt_server = "192.168.0.34";
#define WIFI_TIMEOUT_MS 5000

// deep sleep config (minutes)
#define DEEP_SLEEP_TIME 10

WiFiClient espClient;
PubSubClient client(espClient);

void goToDeepSleep()
{
    Serial.println("going to deep sleep");
    esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60 * 1000000);
    esp_deep_sleep_start();
}

void setup_wifi()
{
    delay(10);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Failed!");
        WiFi.disconnect();
        setup_wifi();
    }
    else
    {
        Serial.println("Connected");
        Serial.println(WiFi.localIP());
    }
}

// reconnect the mqtt connection
void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect, sending the client name
        if (client.connect("Huzzah32"))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void gatherAndPublishData()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    humid = my_sensor.readHumidity();
    temp = my_sensor.readTemperature();

    Serial.println("Temp: ");
    Serial.println(temp);
    Serial.println(" Humidity: ");
    Serial.println(humid);

    String msg = "home temperature=" + String(temp, 1) + ",humidity=" + String(humid, 1);
    Serial.println(msg);

    // Convert the value to a char array
    char *tab2 = new char[msg.length() + 1];
    strcpy(tab2, msg.c_str());

    client.publish("home/sensors", tab2);
}

void setup()
{
    Serial.begin(9600);
    setup_wifi();
    my_sensor.begin();
    client.setServer(mqtt_server, 1883);
    gatherAndPublishData();
    goToDeepSleep();
}

void loop()
{
    // not used
}