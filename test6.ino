#include <SPI.h>//#define rst  8  // you can also connect this to the Arduino reset
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Ethernet.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "LiquidCrystal.h"
#include <Wire.h>
#include <elapsedMillis.h>
#include <DigitalToggle.h>
#include <string.h>
#include <SD.h>
#define MAX_STRING_LEN  110
#define cs 53 //CS
#define dc 49 //A0
#define mosi 51 //SDA
#define rst 48 //RESET
#define sclk 52 //SCK
#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
LiquidCrystal lcd2(22, 23, 24, 25, 26, 27);
String Labl[8];
String chars;
String fchars;
boolean Pstat;
boolean heaterOn = false;
boolean waiting = false;
char c;
char fileName[13] = "test.txt";
char outBuf[128];
char outCount;
char thisChar;
const int Brenner = 35;
const int Mischer_AUF = 31;
const int Mischer_ZU = 32;
const int Pumpe = 33;
double tempsoll = 70;
elapsedMillis Mpause;
elapsedMillis Mtimeauf;
elapsedMillis Mtimezu;
elapsedMillis logIntervall;
elapsedMillis spread;
elapsedMillis timeOut;
elapsedMillis weather;
float Aist;
float Bist;
float Kist;
float R1;
float R2;
float V1;
float V2;
float tempist;
int Ksoll = 85;
int Spanne = 15;
int V2soll;
int col[9];
int count = 1;
int dif;
int hyst = 2;
int i = 0;
int len;
int mstat = 0;
int sens[99];
int tcastfrueh;
int tcastspaet;
int tempstor = 0;
int xPos = 10;
int yCoord[9];
unsigned long coldtime = 0;
unsigned long diftime = 0;
unsigned long interval = 80000;
unsigned long warmtime = 0;

File myFile;
#pragma region Onewire
OneWire oneWire(8);
DallasTemperature sensors(&oneWire);
DeviceAddress Kessel = { 0x10, 0xE8, 0x4A, 0x29, 0x02, 0x08, 0x00, 0xFD };
DeviceAddress Aussen = { 0x10, 0x3C, 0x08, 0x29, 0x02, 0x08, 0x00, 0x8F };
DeviceAddress Brauch = { 0x10, 0x57, 0x0B, 0x29, 0x02, 0x08, 0x00, 0xFA };
DeviceAddress V1wire = { 0x28, 0x50, 0xBD, 0xB0, 0x05, 0x00, 0x00, 0x37 };
DeviceAddress R1wire = { 0x28, 0x47, 0x21, 0xB1, 0x05, 0x00, 0x00, 0xF0 };
DeviceAddress V2wire = { 0x28, 0x4B, 0x54, 0x95, 0x05, 0x00, 0x00, 0x4C };
DeviceAddress R2wire = { 0x28, 0x30, 0x85, 0x96, 0x05, 0x00, 0x00, 0x37 };
#pragma endregion Onewire

#pragma region ETHERNET
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress fserver(141, 38, 3, 183);
EthernetServer server(23);
EthernetClient fclient;
EthernetClient dclient;
EthernetClient client = 0;
EthernetClient pushbox;
String devid = "vD55216BF26119A8";
char serverName[] = "api.pushingbox.com";
#pragma endregion ETHERNET

void setup()
{
	pinMode(53, OUTPUT);
	pinMode(4, OUTPUT);
	pinMode(47, OUTPUT);
	digitalWrite(47, HIGH);
	lcd2.begin(16, 2);
	Ethernet.begin(mac, ip, gateway, gateway, subnet);
	server.begin();
	sensors.begin();
	sensors.setResolution(Kessel, 10);
	sensors.setResolution(Aussen, 10);
	sensors.setResolution(Brauch, 10);
	sensors.setResolution(V1wire, 10);
	sensors.setResolution(R1wire, 10);
	sensors.setResolution(V2wire, 10);
	sensors.setResolution(R2wire, 10);
	weather = 360000;
	Bist = 50;
	Serial.begin(115200);
	dif = 300;
	pinMode(4, OUTPUT);
	pinMode(53, OUTPUT);
	pinMode(33, OUTPUT);
	pinMode(Brenner, OUTPUT);
	pinMode(Mischer_AUF, OUTPUT);
	pinMode(Mischer_ZU, OUTPUT);
	Pstat = false;
	col[1] = 0xF87E;
	col[2] = 0x3922;
	col[3] = 0xF800;
	col[4] = 0x07E0;
	col[5] = 0x07FF;
	col[6] = 0xF81F;
	col[7] = 0xFFE0;
	col[8] = 0xFF25;
	Labl[1] = "V1";
	Labl[2] = "R1";
	Labl[3] = "V2";
	Labl[4] = "R2";
	Labl[5] = "KE";
	Labl[6] = "BW";
	Labl[7] = "AT";
	Labl[8] = "MI";

	Serial.println("initialization done.");
	Serial.print("Chat server address:");
	Serial.println(Ethernet.localIP());
	//Serial.println("HV;HR;BV;BR;Kessel;Aussen;Brauch;HV SOLL;Pumpe;Mstat;MProzent");
	digitalWrite(33, LOW);
	digitalWrite(4, HIGH);
	digitalWrite(Brenner, LOW);
	digitalWrite(Mischer_AUF, LOW);
	digitalWrite(Mischer_ZU, LOW);
	Mpause = 0;
	tft.initR(INITR_BLACKTAB);
	mstat = 0;
	diftime = 0;

	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);
	tft.setRotation(1);
	Bist = 10;
	//if (!SD.begin(4))
	//{
	//	Serial.println("initialization failed!");
	//	return;
	//}
	//Serial.println("initialization done.");
}

void loop()
{
	if (weather < 0)
	{
		weather = 0;
		doFTP("Daten_Deutschland_morgen_spaet");
		fclient.stop();
		//////////////////////////////////////////////////////////////////////////

		doFTP("Daten_Deutschland_morgen_frueh");

		fclient.stop();
	}

	parseSensors();

	if (logIntervall > 18000)
	{
		logIntervall = 0;
		Grapher();
	}

	tcpReq();
	mischen();

	lcd2.clear();
	lcd2.setCursor(0, 0);
	lcd2.print(V1, 1);
	lcd2.setCursor(5, 0);
	lcd2.print(R1, 1);
	lcd2.setCursor(10, 0);
	lcd2.print(Kist, 1);
	lcd2.setCursor(0, 1);
	lcd2.print(V2, 1);
	lcd2.setCursor(5, 1);
	lcd2.print(R2, 1);
	lcd2.setCursor(10, 1);
	lcd2.print(Bist, 1);
}
/*

float readtemp(int pin)

float temp;

int res[20];
int sum = 0;

for (int x = 1; x <= 20; x++)
{
int t = analogRead(pin);
res[x] = t;
sum = sum + res[x];
temp = (sum / 20) * 4.8 / 1024;
temp = temp - 0.5;
temp = temp / 0.01;
}

return temp;
}*/
void tcpReq(){
	EthernetClient client = server.available();

	if (client)

	{
		while (client.connected())
		{
			mischen();
			if (client.available())
			{
				waiting = false;
				thisChar = client.read();

				chars = chars + thisChar;
			}
			else
			{
				if (waiting = false){
					timeOut = 0;
					waiting = true;
				}
				else
				{
					if (timeOut > 5000){
						client.stop();
						waiting = false;
					}
				}
			}

			//thisChar = 0;
			//////////////////////////////////////////////////////////////////////////////////////////
			if (thisChar == 10)
			{
				Serial.println(chars);
				thisChar = 0;

				/////////////////////////////////////////////////////////////////////////////////////////////

				/////////////////////////////////////////////////////////////////////////////////////////////

				if (chars.indexOf("pan") > -1)
				{
					digitalWrite(33, HIGH);
					Pstat = true;
					delay(500);
					chars = "";
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("paus") > -1)
				{
					digitalWrite(33, LOW);
					Pstat = false;
					delay(500);
					chars = "";
				}
				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("psw") > -1)
				{
					digitalToggle(33);
					Pstat = !Pstat;

					chars = "";
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("twunsch") > -1)
				{
					client.println(tempsoll);               //1
					chars = "";
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("hsoll") > -1)
				{
					String conv = chars.substring(chars.indexOf("t") + 1, chars.indexOf("t") + 3);
					int numb = conv.toInt();

					tempsoll = numb;
					chars = "";
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("ksoll") > -1)
				{
					String conv = chars.substring(chars.indexOf("t") + 1, chars.indexOf("t") + 3);
					int numb = conv.toInt();
					conv = "";
					Ksoll = numb;
					chars = "";
				}

				/////////////////////////////////////////////////////////////////////////////////////////////
				if (chars.indexOf("data") > -1)
				{
					client.print(V1);               //1
					client.print(F(";"));
					client.print(R1);
					client.print(F(";"));
					client.print(V2);
					client.print(F(";"));
					client.print(R2);
					client.print(F(";"));
					client.print(Kist);
					client.print(F(";"));
					client.print(Aist);
					client.print(F(";"));
					client.print(Bist);
					client.print(F(";"));
					client.print(tempsoll);
					client.print(F(";"));
					client.print(Pstat);
					client.print(F(";"));
					client.print(mstat);
					client.print(F(";"));
					client.print(diftime);
					client.print(F(";"));
					client.print(V2soll);
					client.print(F(";"));
					client.print(heaterOn);
					client.print(F(";"));
					client.print(Ksoll);
					client.println("#");
					chars = "";
				}
			}	/////////////////////////////////////////////////////////////////////////////////////////////
			//(thisChar == '\n') ENDE
		}//if CLIENT Ende
	}client.stop();//IF CLIENT CONNECTED ENDE

	if (spread > 60000)
	{
		pushbox.connect(serverName, 80);
		Serial.println("connected pushbox");

		Serial.println("sending request");
		pushbox.print("GET /pushingbox?devid=");
		pushbox.print(devid);
		for (int i = 1; i < 12; i++)
		{
			pushbox.print("&");
			pushbox.print(i);
			pushbox.print("=");
			pushbox.print(sens[i]);
		}
		pushbox.println(" HTTP/1.1");
		pushbox.print("Host: ");
		pushbox.println(serverName);
		pushbox.println("User-Agent: Arduino");
		pushbox.println();

		pushbox.stop();
		spread = 0;
	}
	else
	{
	}
}

void parseSensors() {
	sensors.requestTemperatures();
	delay(10);
	tempist = sensors.getTempC(V1wire);
	V1 = tempist;
	R1 = sensors.getTempC(R1wire);
	V2 = sensors.getTempC(V2wire);
	R2 = sensors.getTempC(R2wire);
	Kist = sensors.getTempC(Kessel);
	Aist = sensors.getTempC(Aussen);

	sens[1] = V1;
	sens[2] = R1;
	sens[3] = V2;
	sens[4] = R2;
	sens[5] = Kist;
	sens[6] = Bist;
	sens[7] = Aist;

	sens[8] = heaterOn;
	sens[9] = diftime;
}

void Grapher()

{
	for (int i = 1; i < 8; i++)
	{
		yCoord[i] = map(sens[i], 0, 100, 0, tft.height());
		tft.drawPixel(xPos, tft.height() - yCoord[i], col[i]);
	}

	if (xPos >= 160)
	{
		xPos = 10;
	}
	else
	{
		xPos = xPos + 1;
	}
	tft.drawFastVLine(xPos, 30, tft.height(), ST7735_BLACK);
	for (int i = 1; i < 9; i++)
	{
		tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
		tft.setCursor(0, (tft.height() - (i * 13)) - 3);
		tft.print(i * 10);
		tft.setCursor(10, (tft.height() - (i * 13)) - 5);
		tft.setTextColor(ST7735_WHITE);
	}
	for (int i = 1; i < 8; i++)
	{
		tft.setTextSize(1);
		tft.setTextColor(col[i], ST7735_BLACK);
		tft.setCursor(i * 15 + 15, 0);
		tft.print(Labl[i]);
		tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
		tft.setCursor(i * 15 + 15, 10);
		tft.print(sens[i]);
	}
	for (int i = 8; i < 161; i += 25){
		tft.drawFastVLine(i, (tft.height() - (1 * 13)), 13, ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (2 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (3 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (4 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (5 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (6 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (7 * 13)), ST7735_WHITE);
		tft.drawPixel(i, (tft.height() - (8 * 13)), ST7735_WHITE);

		tft.drawPixel(i - 12, (tft.height() - (2 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (3 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (4 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (5 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (6 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (7 * 13)), ST7735_WHITE);
		tft.drawPixel(i - 12, (tft.height() - (8 * 13)), ST7735_WHITE);
	}
}

void mischen()
{
	if (millis() > 120000)
	{
		//WartenA = millis();    //alles 10 Sekunden die Temperaturen vergleichen
		if (Mpause > 220000)
		{
			sensors.requestTemperatures();
			delay(750);
			Bist = sensors.getTempC(Brauch);
			digitalWrite(Mischer_ZU, LOW);
			digitalWrite(Mischer_AUF, LOW);

			if (V1 > (tempsoll + 4) && mstat == 0)  // Mischer zu - Temperatur runter
			{
				Mtimezu = 0;
				//.B.tempist=27(Grad) und tempsoll= 22(Grad) dann geht der Mischer 5*400ms also 2,5sec lang zu
				digitalWrite(Mischer_ZU, HIGH);
				////Serial.print("Mischzeit ist ");
				////Serial.println(((tempist - tempsoll) * 400));
				////Serial.println("Mischer faehrt ZU");
				mstat = 32;
			}

			if (V1 < (tempsoll - hyst) && mstat == 0)      // Mischer auf - Temperatur erhoehen
			{
				//MischenA = millis();
				Mtimeauf = 0;
				digitalWrite(Mischer_AUF, HIGH);
				//Serial.print("Mischzeit ist ");
				////Serial.println(((tempsoll - tempist) * 400));
				////Serial.println("Mischer faehrt AUF");
				mstat = 31;
			}

			Mpause = 0;
		}

		if (Mtimezu + 500 > ((tempist - tempsoll) * 300) && mstat == 32)
		{
			digitalWrite(Mischer_ZU, LOW);
			mstat = 0;
			coldtime = coldtime + Mtimezu;
		}
		if (Mtimeauf + 500 > (((tempsoll - tempist) * 400) - (diftime - tempist)*dif) && mstat == 31)
		{
			warmtime = warmtime + Mtimeauf;
			digitalWrite(Mischer_AUF, LOW);
			mstat = 0;
		}
		if (coldtime > warmtime)
		{
			coldtime = 0;
			warmtime = 0;
		}

		if (warmtime - coldtime > 100000)
		{
			warmtime = 100000;
			coldtime = 0;
		}

		diftime = (warmtime - coldtime) / 1000;
	}
	else
	{
		digitalWrite(Mischer_ZU, HIGH);
	}
	//--------MISCHEN Ende-----------------

	if (Kist < (Ksoll - Spanne) && !heaterOn && Kist > 0)
	{
		digitalWrite(Brenner, HIGH);
		heaterOn = true;
	}

	if (Kist >= Ksoll)
	{
		digitalWrite(Brenner, LOW);
		heaterOn = false;
	}
}