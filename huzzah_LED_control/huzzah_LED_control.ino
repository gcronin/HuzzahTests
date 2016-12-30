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

  Modified GCronin... includes four feeds, two are subscribed to and
  vary the state of two attached LEDs.  Two are published to and the
  values displayed on the Adafruit IO website.
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// function prototypes
void connect(void);

/****************************** Pins ******************************************/

#define LEDpin            2  
#define LEDPWMpin         5
#define SWITCHpin         4 

int current = 0;              // variable used throughout to store feed and switch values
int last = -1;                // stores the last value of the switch so we only need to publish data on a change of state
long timeStamp = 0;             // used to publish state
int publishDelay = 60 * 1000; // send will every 60 seconds

/************************* WiFi Access Point *********************************/

// SAAS
#define WLAN_SSID       "stream"
#define WLAN_PASS       "performance"

//MEGs (failed)
//#define WLAN_SSID       "W2LCC"
//#define WLAN_PASS       "R56386Q2Q5Y5SM8N"

//Mom and Dads (worked)
//#define WLAN_SSID       "MOTOROLA-CB317"
//#define WLAN_PASS       "10759f9fe636a551e0d3"

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

// Setup feeds called 'LED' and 'LEDPWM' for subscribing to changes from Adafruit IO.
Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test-LED");
Adafruit_MQTT_Subscribe LEDPWM = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/test-PWM");

// Setup a feed called SWITCH for publishing changes.  This links to an adafruit feed called test-switch which shows
// whether the switch is pressed (grounded) or pulled high.  MQTT_QOS_1 means that AIO should send an "OK" back
// If we don't get that, the command SWITCH.publish() will return false.
Adafruit_MQTT_Publish SWITCH = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test-switch", MQTT_QOS_1);

// Setup a feed called lastwill for publishing changes.  This links to an adafruit feed called test-onoff which shows
// whether of not the huzzah is online. 
Adafruit_MQTT_Publish lastwill = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/test-onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

void setup() {

  // set LED pins as outputs
  pinMode(LEDpin, OUTPUT);
  pinMode(LEDPWMpin, OUTPUT);         

  // setup SWITCH pin as input pulled HIGH
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

  // listen for events on the LED feeds
  mqtt.subscribe(&LED);
  mqtt.subscribe(&LEDPWM);

  // Setup MQTT 'will' to set test-on/off feed to "OFF" when we disconnect
  mqtt.will(AIO_USERNAME "/feeds/test-onoff", "OFF");

  // connect to adafruit io
  connect();

  lastwill.publish("ON");  // make sure we publish ON first thing after connecting
  timeStamp = millis();
}

void loop() 
{

  if(millis() - timeStamp > publishDelay)
  {
    lastwill.publish("ON");
    timeStamp = millis();
  }
  
  Adafruit_MQTT_Subscribe *subscription;

  // ping adafruit io three times to make sure we remain connected
  if(! mqtt.ping(3)) 
  {
    // reconnect to adafruit io if the ping failed
    if(! mqtt.connected())
      connect();
  }


  // this is our 'wait for incoming subscription packets' busy subloop.
  // this takes 5000 ms to run, which means that the rest of the program
  // only gets done once in 5 seconds.  Potential bug since if the 
  // switch is pressed and released during the 5 second period it will not
  // be registered... probably better to use an interrupt for the switch pin.
  while (subscription = mqtt.readSubscription(5000)) 
  {

    // for events from the test-LED feed
    if (subscription == &LED) 
    {

      // convert mqtt ascii payload to int
      char *value = (char *)LED.lastread;
      Serial.print(F("LED Received: "));
      Serial.println(value);
      int current = atoi(value);

      // write the current state to the LED
      digitalWrite(LEDpin, current == 1 ? HIGH : LOW);

    }
    
    // for events from the test-PWM feed
    else if (subscription == &LEDPWM)
    {
      char *value = (char *)LEDPWM.lastread;
      Serial.print(F("PWM Received: "));
      Serial.println(value);
      int current = atoi(value);

      // write the PWM value to the LED
      analogWrite(LEDPWMpin, current);

    }
  }


    // grab the current state of the switch
  current = digitalRead(SWITCHpin);
  Serial.println(current);

  // return if the value hasn't changed
  if(current == last)
    return;

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
