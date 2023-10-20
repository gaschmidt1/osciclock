/* oscilloscope clock with wemos lolin32 (esp32)
 * osciclock_esp32_tab.ino
 * weigu.lu
 * (reduce upload speed if errors occur)
 * for tables see libreoffice calc sheet
 * DAC 1 = Pin 25, DAC_2 = Pin 26;
 */

#include <WiFi.h>
#include "time.h"
#include <driver/dac.h>

const char* ssid       = "myssid";
const char* password   = "mypasswd";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm ti;

// sine and cosine table with 360 points
unsigned int circlex[] = 
{
  255,255,255,255,255,255,254,254,
  254,253,253,253,252,252,251,251,
  250,249,249,248,247,247,246,245,
  244,243,242,241,240,239,238,237,
  236,234,233,232,231,229,228,227,
  225,224,222,221,219,218,216,214,
  213,211,209,208,206,204,202,201,
  199,197,195,193,191,189,187,185,
  183,181,179,177,175,173,171,169,
  167,165,163,160,158,156,154,152,
  150,147,145,143,141,139,136,134,
  132,130,128,125,123,121,119,116,
  114,112,110,108,105,103,101,99,
  97,95,92,90,88,86,84,82,
  80,78,76,74,72,70,68,66,
  64,62,60,58,56,54,53,51,
  49,47,46,44,42,41,39,37,
  36,34,33,31,30,28,27,26,
  24,23,22,21,19,18,17,16,
  15,14,13,12,11,10,9,8,
  8,7,6,6,5,4,4,3,
  3,2,2,2,1,1,1,0,
  0,0,0,0,0,0,0,0,
  0,0,1,1,1,2,2,2,
  3,3,4,4,5,6,6,7,
  8,8,9,10,11,12,13,14,
  15,16,17,18,19,21,22,23,
  24,26,27,28,30,31,33,34,
  36,37,39,41,42,44,46,47,
  49,51,53,54,56,58,60,62,
  64,66,68,70,72,74,76,78,
  80,82,84,86,88,90,92,95,
  97,99,101,103,105,108,110,112,
  114,116,119,121,123,125,128,130,
  132,134,136,139,141,143,145,147,
  150,152,154,156,158,160,163,165,
  167,169,171,173,175,177,179,181,
  183,185,187,189,191,193,195,197,
  199,201,202,204,206,208,209,211,
  213,214,216,218,219,221,222,224,
  225,227,228,229,231,232,233,234,
  236,237,238,239,240,241,242,243,
  244,245,246,247,247,248,249,249,
  250,251,251,252,252,253,253,253,
  254,254,254,255,255,255,255,255
};

unsigned int circley[] = 
{
  128,130,132,134,136,139,141,143,
  145,147,150,152,154,156,158,160,
  163,165,167,169,171,173,175,177,
  179,181,183,185,187,189,191,193,
  195,197,199,201,202,204,206,208,
  209,211,213,214,216,218,219,221,
  222,224,225,227,228,229,231,232,
  233,234,236,237,238,239,240,241,
  242,243,244,245,246,247,247,248,
  249,249,250,251,251,252,252,253,
  253,253,254,254,254,255,255,255,
  255,255,255,255,255,255,255,255,
  254,254,254,253,253,253,252,252,
  251,251,250,249,249,248,247,247,
  246,245,244,243,242,241,240,239,
  238,237,236,234,233,232,231,229,
  228,227,225,224,222,221,219,218,
  216,214,213,211,209,208,206,204,
  202,201,199,197,195,193,191,189,
  187,185,183,181,179,177,175,173,
  171,169,167,165,163,160,158,156,
  154,152,150,147,145,143,141,139,
  136,134,132,130,128,125,123,121,
  119,116,114,112,110,108,105,103,
  101,99,97,95,92,90,88,86,
  84,82,80,78,76,74,72,70,
  68,66,64,62,60,58,56,54,
  53,51,49,47,46,44,42,41,
  39,37,36,34,33,31,30,28,
  27,26,24,23,22,21,19,18,
  17,16,15,14,13,12,11,10,
  9,8,8,7,6,6,5,4,
  4,3,3,2,2,2,1,1,
  1,0,0,0,0,0,0,0,
  0,0,0,0,1,1,1,2,
  2,2,3,3,4,4,5,6,
  6,7,8,8,9,10,11,12,
  13,14,15,16,17,18,19,21,
  22,23,24,26,27,28,30,31,
  33,34,36,37,39,41,42,44,
  46,47,49,51,53,54,56,58,
  60,62,64,66,68,70,72,74,
  76,78,80,82,84,86,88,90,
  92,95,97,99,101,103,105,108,
  110,112,114,116,119,121,123,125
};

// 5 min strokes
unsigned int min5linesxy[] = 
{ 
  243,128,227,185,185,227,128,243,
  70,227,28,185,13,128,28,70,
  70,28,128,13,185,28,227,70
};

// seconds hand r = 102 (80%) 
unsigned int secHandxy[] = 
{ 
  128,230,117,229,106,227,96,225,
  86,221,77,216,68,210,59,203,
  52,196,45,187,39,179,34,169,
  30,159,28,149,26,138,26,128,
  26,117,28,106,30,96,34,86,
  39,77,45,68,52,59,59,52,
  68,45,77,39,86,34,96,30,
  106,28,117,26,128,26,138,26,
  149,28,159,30,169,34,179,39,
  187,45,196,52,203,59,210,68,
  216,77,221,86,225,96,227,106,
  229,117,230,128,229,138,227,149,
  225,159,221,169,216,179,210,187,
  203,196,196,203,187,210,179,216,
  169,221,159,225,149,227,138,229
};

// minute hand r = 108 (85%) fraction =  5 angle = 12 
unsigned int minHand[] = 
{  
  128,236,123,149,132,149,
  116,235,121,148,130,149,
  105,233,119,147,128,149,
  94,230,117,146,125,149,
  84,226,115,145,123,149,
  74,221,113,144,121,148,
  64,215,111,142,119,147,
  55,208,110,140,117,146,
  47,200,109,138,115,145,
  40,191,108,136,113,144,
  34,182,107,134,111,142,
  29,171,106,132,110,140,
  25,161,106,130,109,138,
  22,150,106,128,108,136,
  20,139,106,125,107,134,
  20,128,106,123,106,132,
  20,116,107,121,106,130,
  22,105,108,119,106,128,
  25,94,109,117,106,125,
  29,84,110,115,106,123,
  34,74,111,113,107,121,
  40,64,113,111,108,119,
  47,55,115,110,109,117,
  55,47,117,109,110,115,
  64,40,119,108,111,113,
  74,34,121,107,113,111,
  84,29,123,106,115,110,
  94,25,125,106,117,109,
  105,22,128,106,119,108,
  116,20,130,106,121,107,
  128,20,132,106,123,106,
  139,20,134,107,125,106,
  150,22,136,108,128,106,
  161,25,138,109,130,106,
  171,29,140,110,132,106,
  182,34,142,111,134,107,
  191,40,144,113,136,108,
  200,47,145,115,138,109,
  208,55,146,117,140,110,
  215,64,147,119,142,111,
  221,74,148,121,144,113,
  226,84,149,123,145,115,
  230,94,149,125,146,117,
  233,105,149,128,147,119,
  235,116,149,130,148,121,
  236,128,149,132,149,123,
  235,139,148,134,149,125,
  233,150,147,136,149,128,
  230,161,146,138,149,130,
  226,171,145,140,149,132,
  221,182,144,142,148,134,
  215,191,142,144,147,136,
  208,200,140,145,146,138,
  200,208,138,146,145,140,
  191,215,136,147,144,142,
  182,221,134,148,142,144,
  171,226,132,149,140,145,
  161,230,130,149,138,146,
  150,233,128,149,136,147,
  139,235,125,149,134,148
}; 

// hour hand r = 83 (65%) fraction =  5 angle = 20 
unsigned int hourHand[] = 
{
    128,211,122,143,133,143,
    119,210,120,142,132,144,
    110,209,119,142,130,144,
    102,206,117,141,128,144,
    94,203,116,139,126,144,
    86,199,115,138,125,144,
    79,195,114,137,123,143,
    72,189,113,135,121,143,
    66,183,112,134,120,142,
    60,176,112,132,118,141,
    56,169,111,130,117,140,
    52,161,111,129,116,139,
    49,153,111,127,114,138,
    46,145,111,125,113,136,
    45,136,111,123,113,135,
    45,128,112,122,112,133,
    45,119,113,120,111,132,
    46,110,113,119,111,130,
    49,102,114,117,111,128,
    52,94,116,116,111,126,
    56,86,117,115,111,125,
    60,79,118,114,112,123,
    66,72,120,113,112,121,
    72,66,121,112,113,120,
    79,60,123,112,114,118,
    86,56,125,111,115,117,
    94,52,126,111,116,116,
    102,49,128,111,117,114,
    110,46,130,111,119,113,
    119,45,132,111,120,113,
    128,45,133,112,122,112,
    136,45,135,113,123,111,
    145,46,136,113,125,111,
    153,49,138,114,127,111,
    161,52,139,116,129,111,
    169,56,140,117,130,111,
    176,60,141,118,132,112,
    183,66,142,120,134,112,
    189,72,143,121,135,113,
    195,79,143,123,137,114,
    199,86,144,125,138,115,
    203,94,144,126,139,116,
    206,102,144,128,141,117,
    209,110,144,130,142,119,
    210,119,144,132,142,120,
    211,128,143,133,143,122,
    210,136,142,135,144,123,
    209,145,142,136,144,125,
    206,153,141,138,144,127,
    203,161,139,139,144,129,
    199,169,138,140,144,130,
    195,176,137,141,143,132,
    189,183,135,142,143,134,
    183,189,134,143,142,135,
    176,195,132,143,141,137,
    169,199,130,144,140,138,
    161,203,129,144,139,139,
    153,206,127,144,138,141,
    145,209,125,144,136,142,
    136,210,123,144,135,142
};

void setup() 
{  
  Serial.begin(115200);
  dac_output_enable(DAC_CHANNEL_1);
  dac_output_enable(DAC_CHANNEL_2);
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.enableSTA(true);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime(&ti);    
  WiFi.disconnect(true); //disconnect WiFi as it's no longer needed
  WiFi.mode(WIFI_OFF);  
}

void loop() 
{  
  getLocalTime(&ti);  
  for(int i = 0; i < 5; i++) 
  {
    circle();    // function to make a circle
    sec_hand(ti.tm_sec);
    min_hand(ti.tm_min);
    hour_hand(ti.tm_hour, ti.tm_min);
  }
  // Reset time at 4 in the morning
  if ((ti.tm_hour == 4) and (ti.tm_min == 0) and (ti.tm_sec == 0)) 
  {
    ESP.restart();
  }
}

/**
 * Sets the voltage values for DAC_CHANNEL_1 and DAC_CHANNEL_2.
 *
 * @param x the value to set DAC_CHANNEL_2
 * @param y the value to set DAC_CHANNEL_1
 *
 * @throws ErrorType if there is an error setting the voltage values
 */
void point(int x, int y) 
{
  dac_output_voltage(DAC_CHANNEL_1, y);
  dac_output_voltage(DAC_CHANNEL_2, x);
}

/**
 * Draw a line between two points.
 *
 * @param x1 The x-coordinate of the first point.
 * @param y1 The y-coordinate of the first point.
 * @param x2 The x-coordinate of the second point.
 * @param y2 The y-coordinate of the second point.
 * @param steps The number of steps to take when drawing the line.
 *
 * @throws None.
 */
void line (int x1,int y1,int x2,int y2, int steps) 
{ // draw a line between 2 points  
  for(int i = 0; i < steps; i++) 
  {
    point(x1 + i * (x2 - x1) /steps , y1 + i * (y2 - y1) / steps);
  }
}

/**
 * Draw a line back and forth.
 *
 * @param x1 the x-coordinate of the starting point
 * @param y1 the y-coordinate of the starting point
 * @param x2 the x-coordinate of the ending point
 * @param y2 the y-coordinate of the ending point
 * @param steps the number of steps to draw the line
 *
 * @throws None
 */
void dline (int x1, int y1, int x2, int y2, int steps) { // draw a line back and forth  
  for(int i = 0; i < steps; i++) 
  {
    point(x1 + i * (x2 - x1) / steps, y1 + i * (y2 - y1) / steps);
  }  
  for(int i = 0; i < steps; i++) 
  {  //draw line backwards to avoid ghosts
    point(x2 + i * (x1 - x2) / steps, y2 + i * (y1 - y2) / steps);
  }
}

/**
 * Function to draw a circle.
 *
 * @throws ErrorType description of error
 */
void circle() 
{ //function to draw a circle
  for(int i = 0; i < 360 ; i = i + 2) 
  {
    point(circlex[i], circley[i]);
    if (i % 30 == 0) 
    {  // 5 min lines
      dline(circlex[i], circley[i] ,min5linesxy[i / 30 * 2], min5linesxy[i / 30 * 2 + 1], 5);
    }
  }
}

/**
 * Calculates the position of the second hand on a clock and draws a line on the display.
 *
 * @param second the current second of the minute
 *
 * @throws None
 */
void  sec_hand(int second) 
{  
  int i = (60 - second);
  if (i == 60) 
  {
    i = 0;
  }

  i = i * 2;
  dline(127, 127, secHandxy[i], secHandxy[i+1], 30);
}

/**
 * Calculates the position of the minute hand on a clock based on the given minute value.
 *
 * @param minute the minute value to calculate the position of the minute hand
 *
 * @throws None
 */
void  min_hand(int minute) 
{
  int i = (60 - minute);
  if (i == 60)
  {
     i = 0;
  }

  i = i * 6;
  line (127, 127, minHand[i + 2], minHand[i + 3], 10);
  line (minHand[i + 2],minHand[i + 3],minHand[i],minHand[i + 1],40);
  line (minHand[i], minHand[i + 1], minHand[i + 4], minHand[i + 5], 40);
  line (minHand[i + 4], minHand[i + 5], 127, 127, 10);
}


/**
 * Calculate the position of the hour hand on a clock given the hour and minute.
 *
 * @param hour the hour value (in 24-hour format)
 * @param minute the minute value
 *
 * @throws None
 */
void  hour_hand(int hour, int minute) 
{
  if (hour > 12) 
  {
    hour = hour-12;    
  }
  int hours = (hour * 5 + (minute / 12));  
  int i = (60 - hours);
  if (i == 60) i = 0;
  i = i * 6;
  line (127 ,127 ,hourHand[i + 2], hourHand[i + 3], 7);
  line (hourHand[i + 2], hourHand[i + 3],hourHand[i], hourHand[i + 1], 30);
  line (hourHand[i], hourHand[i + 1], hourHand[i + 4], hourHand[i + 5], 30);
  line (hourHand[i + 4],hourHand[i + 5], 127, 127, 7);
}


