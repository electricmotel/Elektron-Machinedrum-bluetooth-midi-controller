# include <SPI.h>
# include <TFT_eSPI.h>
# include <LedKeypad.h>

# include <MIDI.h>
# include <BLEMIDI_Transport.h>
# include <hardware/BLEMIDI_Client_ESP32.h>

# define GFXFF         1
# define TFT_GREY      0x5AEB 
# define FF18          &FreeSans12pt7b
# define CF_OL24       &Orbitron_Light_24

# define key           A0
# define connected     D7
# define transmitting  D9

///////////////////////////////////////////////////////////////////////////////////////////////////
BLEMIDI_CREATE_DEFAULT_INSTANCE()     
///////////////////////////////////////////////////////////////////////////////////////////////////

float sx = 0, sy = 1;   
unsigned int x0 = 0, x1 = 0, yy0 = 0, yy1 = 0;

const int      interval = 250;
unsigned long oldbouncy = 0;
bool              state = LOW,
            isConnected = false;

byte             pushed = 0,
              togglerun = 0,          
                toggle1 = 1,
                toggle2 = 1,
                toggle3 = 1,
                toggle4 = 1,
                 offset = 30,
            basechannel = 0;

int              oldkey = 0;         

///////////////////////////////////////////////////////////////////////////////////////////////////
int      ctrl[] = {0x0c, 0x0d, 0x0e, 0x0f},
           ch[] = {basechannel + 1, basechannel + 2, basechannel + 3, basechannel + 4};

const char*  mch[4] = {" 01", "02", "03", "04"};
const char*  instrument[][4] = {
             {"BD", "SD", "HT", "MT"},
             {"LT", "CP", "RS", "CB"},
             {"CH", "OH", "RC", "CC"},
             {"M1", "M2", "M3", "M4"},
             };
///////////////////////////////////////////////////////////////////////////////////////////////////

TFT_eSPI tft = TFT_eSPI(); 
    
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

void setup() 
{

  pinMode(connected, OUTPUT);
  pinMode(transmitting, OUTPUT);
  pinMode(key, INPUT);

  MIDI.begin();
  Serial.begin( 115200 );
  tft.init();

///////////////////////////////////////////////////////////////////////////////////////////////////
  BLEMIDI.setHandleConnected([]() {
    isConnected = true;
  });

  BLEMIDI.setHandleDisconnected([]() {
    isConnected = false;
  });
///////////////////////////////////////////////////////////////////////////////////////////////////
  
  ledkeypad.begin(); 
  ledkeypad.setBrightness(1);

  tft.setRotation(0);
  tft.fillScreen(TFT_GREY);
  tft.fillCircle(120, 120, 118, TFT_GREY);
  tft.fillCircle(120, 120, 110, TFT_BLACK); 

  for(int i = 0; i<360; i+= 30) 
  {
    sx = cos((i-45)*0.0174532925);
    sy = sin((i-45)*0.0174532925);
    x0 = sx*118+120;
    yy0 = sy*118+120;
    x1 = sx*115+120;
    yy1 = sy*115+120;

    tft.fillCircle(x1, yy1, 2, TFT_RED);
    tft.drawCircle(x1, yy1, 4, TFT_BLACK);
  }

  for(int i = 0; i<360; i+= 3) 
  {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*115+120;
    yy0 = sy*115+120;
   
    if(i==0  || i==180 || i==45  || i==135) tft.fillCircle(x0, yy0, 4, TFT_WHITE);
    if(i==90 || i==270 || i==225 || i==315) tft.fillCircle(x0, yy0, 4, TFT_WHITE);
    if(i==0  || i==180 || i==45  || i==135) tft.drawCircle(x0, yy0, 6, TFT_BLACK);
    if(i==90 || i==270 || i==225 || i==315) tft.drawCircle(x0, yy0, 6, TFT_BLACK);
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  tft.setFreeFont(FF18);
  tft.setTextColor(TFT_RED, 0); 
  tft.drawCentreString("Run|", 38, 100, 1);
  tft.setTextColor(TFT_RED, 0);
  tft.drawCentreString("|Stop", 200, 100, 1);
  
  tft.setFreeFont(CF_OL24);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawCentreString(" SiK.MiDi ", 120, 28, 1);
  tft.drawCentreString("ChL ", 80, 64, 1);
  tft.drawCentreString("CtrL ", 160, 64, 1);
  
  tft.setTextColor(TFT_WHITE);
  
  for (byte n=0; n<4; n++)
  {
    tft.drawCentreString(mch[n], 88, (94 + (n * offset)), 1);
  } 
    
  tft.drawCentreString(instrument[0][0], 148, (94 + (0 * offset)), 1);  // UP
  tft.drawCentreString(instrument[2][1], 148, (94 + (1 * offset)), 1);  // RIGHT
  tft.drawCentreString(instrument[2][2], 148, (94 + (2 * offset)), 1);  // DOWN
  tft.drawCentreString(instrument[3][2], 148, (94 + (3 * offset)), 1);  // LEFT

  ledkeypad.display(0, toggle1); 
  ledkeypad.display(1, toggle2);
  ledkeypad.display(2, toggle3); 
  ledkeypad.display(3, toggle4);

}

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

void loop() 
{
  
  long keyval;
  int   buff = 50,
         off = 10,
          on = 0;

  unsigned long bouncy = millis();
     
  keyval = analogReadMilliVolts(key);
    
  Serial.println(keyval);
  Serial.println("---------------");
  Serial.println(oldkey);
  Serial.println(pushed);
  
  MIDI.read();  /* Advertise and establish connection */

///////////////////////////////////////////////////////////////////////////////////////////////////
  
  tft.setFreeFont(CF_OL24);
  if (keyval>0 && keyval<300 && pushed == 1)  //////////////////////////////////////////////// DOWN    
  {
   
   if (toggle3 == 1) 
   {
     MIDI.sendControlChange( ctrl[2], off, ch[2] );
     toggle3 = 0;
     ledkeypad.display(2, toggle3);
     tft.setTextColor(TFT_WHITE, TFT_GREY);
     tft.drawCentreString(instrument[2][2], 148, (94 + (2 * offset)), 1);  /////////////////// DOWN
     oldkey = keyval;
   } 
   else 
   {
     MIDI.sendControlChange( ctrl[2], on, ch[2] );
     toggle3 = 1;
     ledkeypad.display(2, toggle3);
     tft.setTextColor(TFT_WHITE, 0);
     tft.drawCentreString(instrument[2][2], 148, (94 + (2 * offset)), 1);  /////////////////// DOWN
     oldkey = keyval;
   }  
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  if (keyval>400 && keyval<800 && pushed == 1)  ////////////////////////////////////////////// LEFT   
  {
   
   if (toggle4 == 1) 
   {
     MIDI.sendControlChange( ctrl[2], off, ch[3] );
     toggle4 = 0;
     ledkeypad.display(3, toggle4);
     tft.setTextColor(TFT_WHITE, TFT_GREY);
     tft.drawCentreString(instrument[3][2], 148, (94 + (3 * offset)), 1);  /////////////////// LEFT
     oldkey = keyval;
   } 
   else 
   {
     MIDI.sendControlChange( ctrl[2], on, ch[3] );
     toggle4 = 1;
     ledkeypad.display(3, toggle4);
     tft.setTextColor(TFT_WHITE, 0);
     tft.drawCentreString(instrument[3][2], 148, (94 + (3 * offset)), 1);  /////////////////// LEFT
     oldkey = keyval;   
   }   
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  if (keyval>1100 && keyval<1500 && pushed == 1)  ////////////////////////////////////////////// UP   
  {
   
   if (toggle1 == 1)     
   {
     MIDI.sendControlChange( ctrl[0], off, ch[0] );
     toggle1 = 0;
     ledkeypad.display(0, toggle1);
     tft.setTextColor(TFT_WHITE, TFT_GREY);
     tft.drawCentreString(instrument[0][0], 148, (94 + (0 * offset)), 1);  ///////////////////// UP
     oldkey = keyval;
   } 
   else 
   {
     MIDI.sendControlChange( ctrl[0], on, ch[0] );
     toggle1 = 1;
     ledkeypad.display(0, toggle1);
     tft.setTextColor(TFT_WHITE, 0);
     tft.drawCentreString(instrument[0][0], 148, (94 + (0 * offset)), 1);  ///////////////////// UP
     oldkey = keyval;
   }    
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  if(keyval>1800 && keyval<2200 && pushed == 1)  //////////////////////////////////////////// RIGHT 
  {
   
   if (toggle2 == 1) 
   {
     MIDI.sendControlChange( ctrl[1], off, ch[2] );
     toggle2 = 0;
     ledkeypad.display(1, toggle2);
     tft.setTextColor(TFT_WHITE, TFT_GREY);
     tft.drawCentreString(instrument[2][1], 148, (94 + (1 * offset)), 1);  ////////////////// RIGHT
     oldkey = keyval;
   } 
   else 
   {
     MIDI.sendControlChange( ctrl[1], on, ch[2] );
     toggle2 = 1;
     ledkeypad.display(1, toggle2);
     tft.setTextColor(TFT_WHITE, 0);
     tft.drawCentreString(instrument[2][1], 148, (94 + (1 * offset)), 1);  ////////////////// RIGHT
     oldkey = keyval;
   }     
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  if(keyval>2300 && keyval<2700 && pushed == 1)  /////////////////////////////////////////// SELECT  
  {
   
   tft.setFreeFont(FF18);
   if (togglerun == 0)
   { 
     MIDI.sendStart();   /////////////////////////////////////////////////////////////////// SELECT
     tft.fillRect(34, 134, 10, 10, TFT_BLUE);
     oldkey = keyval;
     for (int j=0; j<4; j++)
     {
       ledkeypad.dotShow(j);
     }
     tft.setTextColor(TFT_RED, TFT_WHITE); 
     tft.drawCentreString("Run|", 38, 100, 1);
     tft.setTextColor(TFT_RED, 0);
     tft.drawCentreString("|Stop", 200, 100, 1);
     togglerun = 1;
   }
   else                                                  
   {
     MIDI.sendStop();   //////////////////////////////////////////////////////////////////// SELECT
     tft.fillRect(34, 134, 10, 10, TFT_RED);
     oldkey = keyval;
     for (int k=0; k<4; k++)
     {
       ledkeypad.dotVanish(k);
     }
     tft.setTextColor(TFT_RED, 0); 
     tft.drawCentreString("Run|", 38, 100, 1);
     tft.setTextColor(TFT_RED, TFT_WHITE);
     tft.drawCentreString("|Stop", 200, 100, 1);
     togglerun = 0; 
   }
  }

///////////////////////////////////////////////////////////////////////////////////////////////////

  if (abs(keyval - oldkey) > 100) {pushed = 1; delay(buff);}
  else pushed = 0;
  delay(buff);

  if (pushed == 0) digitalWrite(transmitting, HIGH);
  else digitalWrite(transmitting, LOW);
  
  if ((bouncy - oldbouncy >= interval) && (isConnected == true)) 
  {
   oldbouncy = bouncy;
   if (state == LOW) state = HIGH;
   else state = LOW;
   digitalWrite(connected, state);
  }
  else digitalWrite(connected, LOW);
}
