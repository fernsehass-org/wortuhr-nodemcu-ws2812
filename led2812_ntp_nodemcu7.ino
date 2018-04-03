/*

 Udp NTP Worldclock mit nodemcu und ws2812b

 Zeit wird über Wlan vom ntp Server geholt und in eine anzeigbare version umgewandelt.
 
 http://en.wikipedia.org/wiki/Network_Time_Protocol

 erstellt an 4.2.2017
 von fernsehass
 
 */
//nodemcu 1
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//Sommer winterzeit dings
const int buttonPin = 2;
int buttonState = 0; 

//hier ist das für ledstrip
#include <FastLED.h>
#define NUM_LEDS 164
#define DATA_PIN 3
#define LED_TYPE    WS2812B
#define COLOR_ORDER RGB
CRGB leds[NUM_LEDS];

boolean helligkeitssteuerung  = false; // Helligkeitssteuerung an dann true
uint8_t maxBrightness         = 255;  // Wenn Fotoresistor benutzt wird, hier max. Helligkeit eintragen (0=Off->255=max)
uint8_t minBrightness         = 50;   // Wenn Fotoresistor benutzt wird, hier min. Helligkeit eintragen (0=Off->255=max)
uint8_t AmbientLightPin       = 0;    // Fotoresistor Pin 
uint8_t BRIGHTNESS            = 250;  // Wenn kein Fotoresistor benutzt wird hier dauerhafte Helligkeit eintragen

uint8_t Stunde      = 9;
uint8_t Minute      = 30; 
uint8_t WochenTag   = 0;   
uint8_t Tag         = 30;
uint8_t Monat       = 10;
boolean DST         = false;

//***HIER LED POSITIONEN EINTRAGEN***//
int Es[]        = {0,1};
int Ist[]       = {3,4,5};
int Uhr[]       = {97,98,99};
int Kein[]      = {36,37,38,39};
int Ntp[]       = {59,60,61};

int Ein[]       = {61,62,63};
int Eins[]      = {60,61,62,63};
int Zwei[]      = {53,54,55,56};
int Drei[]      = {64,65,66,67};
int Vier[]      = {71,72,73,74};
int Fuenf[]     = {49,50,51,52};
int Sechs[]     = {81,82,83,84,85};
int Sieben[]    = {86,87,88,89,90,91};
int Acht[]      = {75,76,77,78};
int Neun[]      = {101,102,103,104};
int Zehn[]      = {104,105,106,107};
int Elf[]       = {68,69,70};
int Zwoelf[]    = {92,93,94,95,96};

int MFuenf[]    = {7,8,9,10};
int MZehn[]     = {16,17,18,19};

int Viertel[]     = {24,25,26,27,28,29,30};
int Halb[]        = {42,43,44,45};
int Dreiviertel[] = {20,21,22,23,24,25,26,27,28,29,30};
int Vor[]         = {39,40,41};
int Nach[]        = {31,32,33,34};

int Plus[]      = {108};
int Minus[]     = {109};

int EinsM[]     = {110}; //Minuten Punkt 1
int ZweiM[]     = {111}; //Minuten Punkt 2
int DreiM[]     = {112}; //Minuten Punkt 3
int VierM[]     = {113}; //Minuten Punkt 4
//**********************************//

int i, Ambient, LastAmbient;

//bis hier ledstrip
char ssid[] = "HoD OG";  //  your network SSID (name)
char pass[] = "22656893";       // your network password

unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void setup()
{
  pinMode(buttonPin, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  //LEDs werden eingefügt + Farbkorrektur und Farbtemperatur auf Wolfram (warmweiß)
  //FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection(TypicalPixelString)*/;
  FastLED.setTemperature( Tungsten40W );
  Serial.println("Starte Wordclock ...");
}

void SwitchLED(int MyArray[], int n) {
  //Umgebungshelligkeit überprüfen (sofern gewünscht)
  if(helligkeitssteuerung)
  {
      Serial.println("Helligkeitssteuerung über Fotowiderstand");
      Ambient = analogRead(AmbientLightPin);
       Serial.print("Analog lesen= ");
       Serial.println(Ambient);
      //Nur bei größeren Helligkeitsänderungen soll die Helligkeit der LEDs angepasst werden:
      if((Ambient > LastAmbient*1.10) || (Ambient < LastAmbient*0.90))
      {
        BRIGHTNESS = map(Ambient, 0, 1023, maxBrightness, minBrightness);
        LastAmbient = Ambient;
        Serial.print("Helligkeit= ");
        Serial.println(BRIGHTNESS);
      }
  }
  //Funktion zum Anschalten der LEDs in warmweiß (oder gewünschter Farbe)
  for (i = MyArray[0]; i < MyArray[0]+n; i++)
  {
      //leds[i] = 0xFFC58F;                         // HEX Warmweiß | Hier gewünschte LED Farbe (HEX) eintragen
      //leds[i] = CHSV(140, 27, BRIGHTNESS);        // ODER hier gewünschte Farbe in HSV (  CHSV(FARBE, SÄTTIGUNG, BRIGHTNESS)  )
      leds[i].setRGB(255, 200, 140);                // ODER hier gewünschte Farbe in RGB (  .setRGB(Grün,Rot,Blau)   )
      FastLED.setBrightness(BRIGHTNESS);
  }
}

void stunde1()
{
  switch (Stunde)
      {
      case 1: 
        SwitchLED(Ein, (sizeof(Ein)/4));
        //Serial.println("1 ");
        break;
      case 2: 
        SwitchLED(Zwei, (sizeof(Zwei)/4));
        //Serial.println("2 ");
        break;
      case 3: 
        SwitchLED(Drei, (sizeof(Drei)/4));
        //Serial.println("3 ");
        break;
      case 4: 
        SwitchLED(Vier, (sizeof(Vier)/4));
        //Serial.println("4 ");
        break;
      case 5: 
        SwitchLED(Fuenf, (sizeof(Fuenf)/4));
        //Serial.println("8 ");
        break;
      case 6: 
        SwitchLED(Sechs, (sizeof(Sechs)/4));
        //Serial.println("6 ");
        break;
      case 7: 
        SwitchLED(Sieben, (sizeof(Sieben)/4));
        //Serial.println("7 ");
        break;
      case 8: 
        SwitchLED(Acht, (sizeof(Acht)/4));
        //Serial.println("8 ");
        break;
      case 9: 
        SwitchLED(Neun, (sizeof(Neun)/4));
        //Serial.println("9 ");
        break;
      case 10: 
        SwitchLED(Zehn, (sizeof(Zehn)/4));
        //Serial.println("10 ");
        break;
      case 11: 
        SwitchLED(Elf, (sizeof(Elf)/4));
        //Serial.println("11 ");
        break;
      case 12: 
        SwitchLED(Zwoelf, (sizeof(Zwoelf)/4));
        //Serial.println("12 ");
        break;
      }
}

void stunde2(void)
{
  switch (Stunde)
  {
          case 1: 
            SwitchLED(Zwei, (sizeof(Zwei)/4));
            //Serial.println("2 ");
            break;
          case 2: 
            SwitchLED(Drei, (sizeof(Drei)/4));  
            //Serial.println("3");
            break;
          case 3: 
            SwitchLED(Vier, (sizeof(Vier)/4));  
            //Serial.println("4 ");
            break;
          case 4: 
            SwitchLED(Fuenf, (sizeof(Fuenf)/4));
            //Serial.println("5 ");
            break;
          case 5: 
            SwitchLED(Sechs, (sizeof(Sechs)/4)); 
            //Serial.println("6 ");
            break;
          case 6: 
            SwitchLED(Sieben, (sizeof(Sieben)/4));
            //Serial.println("7 ");
            break;
          case 7: 
            SwitchLED(Acht, (sizeof(Acht)/4));
            //Serial.println("8 ");
            break;
         case 8: 
            SwitchLED(Neun, (sizeof(Neun)/4));  
            //Serial.println("9 ");
            break;
          case 9: 
            SwitchLED(Zehn, (sizeof(Zehn)/4)); 
            //Serial.println("10 ");
            break;
         case 10: 
            SwitchLED(Elf, (sizeof(Elf)/4)); 
            //Serial.println("11 ");
            break;
          case 11: 
            SwitchLED(Zwoelf, (sizeof(Zwoelf)/4));
            //Serial.println("12 ");
            break;
          case 12: 
            SwitchLED(Eins, (sizeof(Eins)/4));
            //Serial.println("1 ");
            break;
        }
}

void displaytime(void)
{
  //zuerst setzten wir alle LEDs zurück
  fill_solid( leds, NUM_LEDS, CHSV(0, 0, 0));

  // Nun suchen wir die richtigen LEDs und übergeben sie an die Funktion zum Anschalten
  
  // ===================================================================================
  // ===================================================================================
  // Zuerst die Dauerhaften Anzeigen und 5 10 vor nach viertel halb dreiviertel anzeigen
  // ===================================================================================
  //Serial.print("Es ist ");
  SwitchLED(Es, (sizeof(Es)/4));
  SwitchLED(Ist, (sizeof(Ist)/4));
  
  if ( ((Minute>4) && (Minute<10)) || ((Minute>54) && (Minute<60)) || ((Minute>24) && (Minute<30)) || ((Minute > 34) && (Minute<40)))
  { 
    SwitchLED(MFuenf, (sizeof(MFuenf)/4));
    //Serial.print("5 Minuten ");
  }
  if ( ((Minute>9) && (Minute<15)) || ((Minute>19) && (Minute<25)) || ((Minute>39) && (Minute<45)) || ((Minute>49) && (Minute<55)) )
  { 
    SwitchLED(MZehn, (sizeof(MZehn)/4));
    //Serial.print("10 Minuten ");
  }
  if ((Minute>14) && (Minute<20))
  {
    SwitchLED(Viertel, (sizeof(Viertel)/4));
    //Serial.print("Viertel ");
    stunde2();
  }
  if ((Minute>19) && (Minute<45))
  {
    SwitchLED(Halb, (sizeof(Halb)/4));
    //Serial.print("Halb ");
    stunde2();
  }
  if ((Minute>44) && (Minute<50))
  {
    SwitchLED(Dreiviertel, (sizeof(Dreiviertel)/4)); 
    //Serial.print("Dreiviertel ");
    stunde2();
  }
  // ===================================================================================
  // Anzeigen für Volle Stunden
  if (Minute <5)
  {
    stunde1();
    SwitchLED(Uhr, (sizeof(Uhr)/4));
    //Serial.println("Uhr");
  }
  // ===================================================================================
  // Anzeigen für Nach nach voller Stunde
  // ===================================================================================
  if ((Minute < 15) && (Minute > 4))
  {
    SwitchLED(Nach, (sizeof(Nach)/4));
    //Serial.print("nach ");
    stunde1();
  }
  // Anzeigen für Nach nach halber Stunde
  else if ((Minute < 45) && (Minute > 34)) 
 	{
  	SwitchLED(Nach, (sizeof(Nach)/4));
    //Serial.print("nach ");
    stunde2();
 	}
// ===================================================================================
// Anzeigen für Vor voller und halber Stunde
// ===================================================================================
if (((Minute < 30) && (Minute > 19)) || ((Minute < 60) && (Minute > 49)))
{
        SwitchLED(Vor, (sizeof(Vor)/4));
        //Serial.print("vor ");
        stunde2();
}
  // Minuten Zähler
  uint8_t MinCount = Minute-(floor(Minute/10)*10);
  if(MinCount > 5) 
    MinCount = MinCount - 5;

  if ( ((Minute > 0) && (Minute < 5)) || ((Minute > 5) && (Minute < 10)) || ((Minute > 10) && (Minute < 15)) || ((Minute > 15) && (Minute < 20)) || ((Minute > 30) && (Minute < 35)) || ((Minute > 35) && (Minute < 40)) || ((Minute > 40) && (Minute < 45)) || ((Minute > 45) && (Minute < 50)) )
    SwitchLED(Plus, (sizeof(Plus)/4));

  if ( ((Minute > 20) && (Minute < 25)) || ((Minute > 25) && (Minute < 30))  || ((Minute > 50) && (Minute < 55)) || ((Minute > 55) && (Minute < 60)) )
    SwitchLED(Minus, (sizeof(Minus)/4));
    
  switch(MinCount){
    case 4:
      SwitchLED(VierM, (sizeof(VierM)/4)); 
      break;
    case 3:
      SwitchLED(DreiM, (sizeof(DreiM)/4));
      break;
    case 2:
      SwitchLED(ZweiM, (sizeof(ZweiM)/4));
      break;
    case 1:
      SwitchLED(EinsM, (sizeof(EinsM)/4));
      break;
  }
  FastLED.show();
}

void loop()
{
  buttonState = digitalRead(buttonPin);
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    /*int stunden*/Stunde = (epoch  % 86400L) / 3600;
    /*int minuten*/Minute = (epoch % 3600) / 60;
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    Serial.print(Stunde);
    Serial.print(":");
    Serial.println(Minute);

    //SOMMERZEIT DINGS
    Stunde = Stunde +1 +buttonState;
        
    if(Stunde > 12)
      Stunde = Stunde-12;
    else if (Stunde == 0)
      Stunde = 12;
    else
      Stunde = Stunde;
    Serial.println(buttonState);
    Serial.print(Stunde);
    Serial.print(":");
    Serial.println(Minute);
    //CheckDST();
    displaytime();
  }
  // wait ten seconds before asking for the time again
  delay(20000);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}
