Code to create a cross needle display for use with SWR metersuse is made of a combined display tpe WT32-SCO1, which conains a ESP32 Wrover B
The dial is a jpeg image taken from a Diamond meter, and converted to hexadecimaal using http://www.rinkydinkelectronics.com/_t_doimageconverter_mono.php#google_vignette.
The sketch can be put in simulated or real mode  by changing #define SIMULATION_MODE 0
Set to 0 for real ADC values or 1 for simulated values

The sketch calculates the size of the buffer memory required and
reserves the memory for the TFT block copy.
Based on a design by Robert de Kok, PA2RDK, and rewritten by PA0ESH & CHATGPT.
Sunday March 2nd 2025

