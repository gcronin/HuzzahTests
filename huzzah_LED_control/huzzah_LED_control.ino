/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  Adafruit IO example additions by Todd Treece.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// function prototypes
void connect(void);

/****************************** Pins ******************************************/

#define LEDpin            2  // power switch tail
#define LEDPWMpin         5
#define SWITCHpin         4 // know that 2 has an input pullup resistor

int current = 0;            // used to store the current and last readings of the switch
int last = -1;

/************************* WiFi Access Point *********************************/

//#define WLAN_SSID       "W2LCC"
//#define WLAN_PASS       "R56386Q2Q5Y5SM8N"

#define WLAN_SSID       "MOTOROLA-CB317"
#define WLAN_PASS       "10759f9fe636a551e0d3"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "gcronin"
#define AIO_KEY         "75f95bbad10f44c0a4823a611e93abf3"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'LED' for subscribing to changes.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test-LED");
Adafruit_MQTT_Subscribe LEDPWM = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test-PWM");

// Setup a feed called SWITCH for publishing changes.  MQTT_QOS_1 means that AIO should send an "OK" back
// If we don't get that, the command SWITCH.publish() will return false.
Adafruit_MQTT_Publish SWITCH = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test-switch", MQTT_QOS_1);


Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test-onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

void setup() {

  // set LED pin as an output
  pinMode(LEDpin, OUTPUT);
  pinMode(LEDPWMpin, OUTPUT);         

  // setup SWITCH pin as input pulled HI
  pinMode(SWITCHpin, INPUT_PULLUP);

  Serial.begin(9600);

  Serial.println(F("Adafruit IO Example"));  //F means that the string is stored in flash memory

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  delay(10);
  Serial.print(F("Connecting to "));
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  // listen for events on the LED feed
  mqtt.subscribe(&LED);
  mqtt.subscribe(&LEDPWM);

  // Setup MQTT will to set on/off to "OFF" when we disconnect
  mqtt.will(AIO_USERNAME "/feeds/test-onoff", "OFF");

  // connect to adafruit io
  connect();

  lastwill.publish("ON");  // make sure we publish ON first thing after connecting

}

void loop() 
{

  Adafruit_MQTT_Subscribe *subscription;

  // ping adafruit io three times to make sure we remain connected
  if(! mqtt.ping(3)) 
  {
    // reconnect to adafruit io if the ping failed
    if(! mqtt.connected())
      connect();
  }


  // this is our 'wait for incoming subscription packets' busy subloop
  while (subscription = mqtt.readSubscription(5000)) 
  {

    // for LED events
    if (subscription == &LED) 
    {

      // convert mqtt ascii payload to int
      char *value = (char *)LED.lastread;
      Serial.print(F("LED Received: "));
      Serial.println(value);
      int current = atoi(value);

      // write the current state to the power switch tail
      digitalWrite(LEDpin, current == 1 ? HIGH : LOW);

    }
    
    // for PWM events
    else if (subscription == &LEDPWM)
    {
      char *value = (char *)LEDPWM.lastread;
      Serial.print(F("PWM Received: "));
      Serial.println(value);
      int current = atoi(value);

      analogWrite(LEDPWMpin, current);

    }
  }


    // grab the current state of the button
  current = digitalRead(SWITCHpin);
  Serial.println(current);

  // return if the value hasn't changed
  if(current == last)
    return;

  //int32_t value = (current == LOW ? 1 : 0);

  // Now we can publish stuff!
  Serial.print(F("\nSending switch value: "));
  Serial.print(current);
  Serial.print("... ");

  if (! SWITCH.publish(current))
    Serial.println(F("Failed."));
  else
    Serial.println(F("Success!"));

  // save the switch state
  last = current;

}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default: Serial.println(F("Connection failed")); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
