/*--------------------------------------------------------------
  VSWR meter - Version 1.0
  sketch name: Webrotor_Antenna_sensor_TTGO.ino
  Last updated 03032025-22:30 CET
  --------------------------------------------------------------

  Description:  Code to create a cross needle display for use with SWR meters
    // use is made of a combined display tpe WT32-SCO1, which conains a ESP32 Wrover B
    // but any ESP32 with an (cheap) TFT display will do.
    // The dial is a jpeg image taken from a Diamond meter, and converted to hexadecimaal using http://www.rinkydinkelectronics.com/_t_doimageconverter_mono.php#google_vignette.
    // The sketch can be put in simulated or real mode  by changing #define SIMULATION_MODE 0
    // Set to 0 for real ADC values or 1 for simulated values

    // The sketch calculates the size of the buffer memory required and
    // reserves the memory for the TFT block copy.
    // Based on a design by Robert de Kok, PA2RDK, and rewritten by PA0ESH & CHATGPT.
    // Monday March 3rd 2025
--------------------------------------------------------------*/



#include <TFT_eSPI.h>
// Include the colored gauge image swr.h
// The meter.h is a black and white scale for testing.
#include "swr.h"

// Initialize display and sprite
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Define needle centers
#define NEEDLE1_X 38
#define NEEDLE1_Y 214
#define NEEDLE2_X 286
#define NEEDLE2_Y 214

// Define ADC pins
#define ADC1_PIN 32  // GPIO32 for Needle 1 (Reflected Power)
#define ADC2_PIN 33  // GPIO33 for Needle 2 (Forward Power)

// Box dimensions and positions
#define BOX_WIDTH 100
#define BOX_HEIGHT 30
#define BOX_Y 270             // Position of the box
#define LABEL_Y (BOX_Y - 20)  // Position of the label (above the box)
#define BOX_SPACING 10

// Simulation or Real mode define
#define SIMULATION_MODE 0  // Set to 0 for real ADC values or 1 for simulated values

// Needle animation counters
int needleREF = 0;
bool countingUpREF = true;

int needleFWD = 0;
bool countingUpFWD = true;
bool increasing = true;
float adc1 = 0.0;
float adc2 = 0.0;

// Simulated values for ADC1 (Reflected Power) and ADC2 (Forward Power)
//float simulatedADC1 = 0.3;  // Simulated voltage for ADC1 (Reflected Power) (0-1V range)
// float simulatedADC2 = 2.1;  // Simulated voltage for ADC2 (Forward Power) (0-3V range)



void setup() {
  Serial.begin(115200);
  Serial.println("Arduino / ESP32 sketch for cross needle meter with JPEG flash");
  Serial.println("Derived from a design by Robert de Kok, PA2RDK");
  Serial.println("Revision: 3 March 2025 - PA0ESH");
  Serial.println(__FILE__);
  Serial.println();

  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(320, 218);  // Create sprite only for meter image
  sprite.setSwapBytes(true);

  // Initialize ADC
  //analogReadResolution(12);  // 12-bit ADC (0-4064)
  analogSetWidth(12);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return out_max - (out_max - out_min) * ((x - in_min) / (in_max - in_min));
}


// Function to draw the meter image
void drawMeterImage() {
  sprite.pushImage(0, 0, 320, 218, meter_image);  // Push colored meter image
}

// Function to draw needles
void drawNeedles(float angle1, float angle2) {
  sprite.fillSprite(TFT_BLACK);  // Clear sprite
  drawMeterImage();

  // Calculate dynamic needle length for Needle 1 (Reflected Power)
  int needleLength1 = map(angle1, -64, -6, 185, 255);
  float rad1 = radians(angle1);
  int x1 = NEEDLE1_X + cos(rad1) * needleLength1;
  int y1 = NEEDLE1_Y + sin(rad1) * needleLength1;

  // Draw Needle 1 (Red)
  drawWideLine(NEEDLE1_X, NEEDLE1_Y, x1, y1, TFT_RED, 3);


  // Calculate dynamic needle length for Needle 2 (Forward Power)
  int needleLength2 = map(angle2, -174, -115, 255, 195);
  float rad2 = radians(angle2);
  int x2 = NEEDLE2_X + cos(rad2) * needleLength2;
  int y2 = NEEDLE2_Y + sin(rad2) * needleLength2;

  // Draw Needle 2 (Red)

  drawWideLine(NEEDLE2_X, NEEDLE2_Y, x2, y2, TFT_RED, 3);
  //drawThickNeedle(NEEDLE1_X, NEEDLE1_Y, x1, y1, TFT_RED, 4);

  // Push sprite to TFT
  sprite.pushSprite(0, 0);
}

// Function to draw the labels and text boxes
void drawInfoBoxes(float value1, float value2, float vswr) {
  int startX = (320 - (3 * BOX_WIDTH + 2 * BOX_SPACING)) / 2;  // Center the three boxes

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);  // Readable font size

  // Labels
  tft.setCursor(startX + 40, LABEL_Y);
  tft.print("FWD");

  tft.setCursor(startX + BOX_WIDTH + BOX_SPACING + 30, LABEL_Y);
  tft.print("VSWR");

  tft.setCursor(startX + 2 * (BOX_WIDTH + BOX_SPACING) + 40, LABEL_Y);
  tft.print("REF");

  // Box 1: Display ADC2 (Forward Power) in watts
  tft.fillRect(startX, BOX_Y, BOX_WIDTH, BOX_HEIGHT, TFT_BLACK);  // Clear previous box
  tft.drawRect(startX, BOX_Y, BOX_WIDTH, BOX_HEIGHT, TFT_WHITE);  // Draw border
  char fwdStr[10];
  snprintf(fwdStr, sizeof(fwdStr), "%.1fW", value2);  // 1 decimal place

  int textWidthFWD = tft.textWidth(fwdStr);
  int textX_FWD = startX + (BOX_WIDTH - textWidthFWD) / 2;  // Center text horizontally
  tft.setCursor(textX_FWD, BOX_Y + 8);
  tft.print(fwdStr);  // Print FWD value

  // Box 2: Display VSWR (Background changes dynamically)
  uint16_t vswrColor = (vswr < 3.0) ? TFT_GREEN : TFT_RED;
  uint16_t vswrTextColor = (vswr < 3.0) ? TFT_WHITE : TFT_BLACK;  // Change font color if red background

  tft.fillRect(startX + BOX_WIDTH + BOX_SPACING, BOX_Y, BOX_WIDTH, BOX_HEIGHT, vswrColor);
  tft.drawRect(startX + BOX_WIDTH + BOX_SPACING, BOX_Y, BOX_WIDTH, BOX_HEIGHT, TFT_WHITE);
  tft.setTextColor(vswrTextColor, vswrColor);  // Set text color dynamically

  char vswrStr[10];
  if (vswr > 4.0) {
    strcpy(vswrStr, "DANGER");  // Show "DANGER" when VSWR is above 5
  } else {
    snprintf(vswrStr, sizeof(vswrStr), "%.1f", vswr);
  }

  int textWidthVSWR = tft.textWidth(vswrStr);
  int textX_VSWR = startX + BOX_WIDTH + BOX_SPACING + (BOX_WIDTH - textWidthVSWR) / 2;

  tft.setCursor(textX_VSWR, BOX_Y + 8);
  tft.print(vswrStr);

  // Box 3: Display ADC1 (Reflected Power) in watts
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Reset text color
  tft.fillRect(startX + 2 * (BOX_WIDTH + BOX_SPACING), BOX_Y, BOX_WIDTH, BOX_HEIGHT, TFT_BLACK);
  tft.drawRect(startX + 2 * (BOX_WIDTH + BOX_SPACING), BOX_Y, BOX_WIDTH, BOX_HEIGHT, TFT_WHITE);
  char refStr[10];
  snprintf(refStr, sizeof(refStr), "%.1fW", value1);  // 1 decimal place

  int textWidthREF = tft.textWidth(refStr);
  int textX_REF = startX + 2 * (BOX_WIDTH + BOX_SPACING) + (BOX_WIDTH - textWidthREF) / 2;  // Center text horizontally
  tft.setCursor(textX_REF, BOX_Y + 8);
  tft.print(refStr);  // Print REF value
}

void loop() {

  if (SIMULATION_MODE == 0) {
    int raw1 = analogRead(ADC1_PIN); // Read raw ADC value from GPIO 32
    int raw2 = analogRead(ADC2_PIN); // Read raw ADC value from GPIO 33
    float voltage1 = raw1 * (3.3 / 4095.0); // Convert raw value to voltage (assuming 3.3V reference)
    float voltage2 = raw2 * (3.3 / 4095.0); // Convert raw value to voltage
    
    Serial.print("Voltage at GPIO 32: ");
    Serial.print(voltage1, 2); // Print with 2 decimal places
    Serial.print(" V  |  Voltage at GPIO 33: ");
    Serial.print(voltage2, 2);
    Serial.println(" V");
    adc1 = voltage1;
    adc2 = voltage2;

  } else {
    Serial.println("Simulating ADC values...");

    if (increasing) {
      adc2 += 0.05;                         // Increase voltage
      if (adc2 >= 3.3) increasing = false;  // Reverse direction at max
    } else {
      adc2 -= 0.05;                        // Decrease voltage
      if (adc2 <= 0.0) increasing = true;  // Reverse direction at min
    }

    adc1 = adc2 / 5;
  }
  Serial.print("Simulation mode = ");
  Serial.println(SIMULATION_MODE);
  if (SIMULATION_MODE) {
  Serial.print("Increasing =  ");
  Serial.println(increasing);
  }
  Serial.print("ADC1 Voltage (REF): ");
  Serial.println(adc1, 2);
  Serial.print("ADC2 Voltage (FWD): ");
  Serial.println(adc2, 2);

  // Map voltages correctly to angles
  needleREF = mapFloat(adc1, 0.0, 1.0, -64, -6);
  needleFWD = mapFloat(adc2, 0.0, 3.3, -115, -174);

  Serial.print("Mapped needleREF: ");
  Serial.println(needleREF);

  Serial.print("Mapped needleFWD: ");
  Serial.println(needleFWD);

  float boxREF = mapFloat(needleREF, -64, -6, 0.0, 10.0);
  float boxFWD = mapFloat(needleFWD, -115, -174, 0.0, 30.0);

  // Ensure boxREF is not negative and is exactly 0 when ADC1 is 0V
  if (adc1 <= 0.01) {
    boxREF = 0.0;
  }

  // Ensure boxFWD is not negative and is exactly 0 when ADC2 is 0V
  if (adc2 <= 0.01) {
    boxFWD = 0.0;
  }

  Serial.print("Reflected Power (W): ");
  Serial.println(boxREF, 2);

  Serial.print("Forward Power (W): ");
  Serial.println(boxFWD, 2);

  float vswr = calculateVSWR(boxREF, boxFWD);
  Serial.print("VSWR: ");
  Serial.println(vswr, 2);

  drawNeedles(needleREF, needleFWD);
  drawInfoBoxes(boxREF, boxFWD, vswr);

  delay(50);
}

void drawWideLine(int x0, int y0, int x1, int y1, int color, int width) {
  float angle = atan2(y1 - y0, x1 - x0);  // Calculate the angle of the line
  float offsetX = sin(angle) * width / 2.0;
  float offsetY = cos(angle) * width / 2.0;

  // Define the 3 points of a triangle
  int x2 = x0 + offsetX;
  int y2 = y0 - offsetY;
  int x3 = x0 - offsetX;
  int y3 = y0 + offsetY;
  int x4 = x1 + offsetX;
  int y4 = y1 - offsetY;
  int x5 = x1 - offsetX;
  int y5 = y1 + offsetY;

  // Draw filled triangles for a smooth thick line
  sprite.fillTriangle(x2, y2, x3, y3, x4, y4, color);
  sprite.fillTriangle(x3, y3, x4, y4, x5, y5, color);
}

void drawThickNeedle(int x0, int y0, int x1, int y1, int color, int width) {
  float angle = atan2(y1 - y0, x1 - x0);
  float dx = cos(angle) * width / 2.0;
  float dy = sin(angle) * width / 2.0;

  sprite.fillRect(x0 - dx, y0 - dy, x1 - x0 + width, y1 - y0 + width, color);
}

// Function to calculate VSWR
float calculateVSWR(float reflectedPower, float forwardPower) {
  if (forwardPower == 0) {
    return 1.0;  // Prevent division by zero, VSWR = 1 if no forward power
  }

  float ratio = reflectedPower / forwardPower;
  float denominator = 1.0f - sqrt(ratio);
  if (denominator < 0.0001f) denominator = 0.0001f;  // Prevent division by zero

  float vswr = (1 + sqrt(ratio)) / denominator;
  return max(vswr, 1.0f);  // Ensure VSWR is at least 1.0
}
