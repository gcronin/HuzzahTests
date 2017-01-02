#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// function prototypes
void connect(void);

/****************************** Variables  ******************************************/
unsigned char i;
unsigned char j; 
int tweetsReceived = 0;

/****************************** LED Matrixes  **********************************/
unsigned char disp1[4][8]={
{0xFF,0x81,0x81,0x81,0x81,0x81,0x81,0xFF},//Square 1
{0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00},//Square 2
{0x00,0x00,0x3C,0x24,0x24,0x3C,0x00,0x00},//Square 3
{0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00},//Square 4
};

unsigned char heart[1][8]={
{0x00,0x66,0xFF,0xFF,0x7E,0x3C,0x18,0x00},
};

unsigned char check[1][8]={
{0x00,0x03,0x06,0x8C,0xD8,0x70,0x20,0x00},
};

/****************************** LED Display Functions*********************************/

 void flashSquares()
{
   for(j=0;j<4;j++)
  {
   for(i=1;i<9;i++)
    Write_Max7219(i,disp1[j][i-1]);
   delay(50);
  }     
  for(j=2;j>0;j--)
  {
   for(i=1;i<9;i++)
    Write_Max7219(i,disp1[j][i-1]);
   delay(50);
  }    
}

void showHeart()
{
  for(i=1;i<9;i++)
    Write_Max7219(i,heart[0][i-1]);
}

void showCheck()
{
  for(i=1;i<9;i++)
    Write_Max7219(i,check[0][i-1]);
}

void clearScreen()
{
  for(i=1;i<9;i++)
    Write_Max7219(i,0x00);
}

/****************************** Pins ******************************************/
int Max7219_pinCLK = 14;
int Max7219_pinCS = 12;
int Max7219_pinDIN = 13;
 

/****************************** LED Matrix Helper Function**********************/ 
void Write_Max7219_byte(unsigned char DATA) 
{   
            unsigned char i;
      digitalWrite(Max7219_pinCS,LOW);    
      for(i=8;i>=1;i--)
          {     
             digitalWrite(Max7219_pinCLK,LOW);
             digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
             DATA = DATA<<1;
             digitalWrite(Max7219_pinCLK,HIGH);
           }                                 
}
 
 
void Write_Max7219(unsigned char address,unsigned char dat)
{
        digitalWrite(Max7219_pinCS,LOW);
        Write_Max7219_byte(address);           //address，code of LED
        Write_Max7219_byte(dat);               //data，figure on LED 
        digitalWrite(Max7219_pinCS,HIGH);
}
 
void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x03);       //brightness 
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
}

/************************* WiFi Access Point *********************************/

// SAAS
#define WLAN_SSID       "stream"
#define WLAN_PASS       "performance"

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

// Setup feed called 'TWEET' for subscribing to changes from Adafruit IO.
Adafruit_MQTT_Subscribe TWEET = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/RedShiftTweeted");

/*************************Connect to adafruit io via MQTT**********************/ 

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
 
/*************************** Sketch Code ************************************/
 
void setup()
{
 
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  delay(50);
  Init_MAX7219();

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

  // listen for events on the TWEET feed
  mqtt.subscribe(&TWEET);
  
  // connect to adafruit io
  connect();
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

  // this is our 'wait for incoming subscription packets' busy subloop.
  // this takes 5000 ms to run, which means that the rest of the program
  // only gets done once in 5 seconds. 
  while (subscription = mqtt.readSubscription(5000)) 
  {

    // for events from the RedShiftTweeted feed
    if (subscription == &TWEET) 
    {

      // convert mqtt ascii payload to int
      char *value = (char *)TWEET.lastread;
      Serial.print(F("TWEET Received: "));
      Serial.println(value);
      ++tweetsReceived;
      Serial.print(F("Number of Tweets Received: "));
      Serial.println(tweetsReceived);
      
    }
  }

  for(int i = 0; i < tweetsReceived; i++)
  {
    if(i%2 == 0)
    {
      showCheck();
      delay(1000);
    }
    else
    {
      showHeart();
      delay(1000);
    }
  }
  
  clearScreen();    
  
}



