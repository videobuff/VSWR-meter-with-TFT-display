Work in progress

Code to create a cross needle display for use with SWR metersuse is made of a combined display tpe WT32-SCO1, which contains a ESP32 Wrover B
The dial is a jpeg image taken from a Diamond meter, and converted to hexadecimaal using http://www.rinkydinkelectronics.com/_t_doimageconverter_mono.php#google_vignette.
The sketch can be put in simulated or real mode  by changing #define SIMULATION_MODE 0
Set to 0 for real ADC values or 1 for simulated values

The VSWR meter will be used with the qo-100 amplifier by SG Lab 2.4 GHz 20W Power Amplifier V3
Items to be completed.
1. ADC input for FWD and REF. The SG labs amplifier gives 0 to 3.5 V  for 0 to 20W output. But the ADC can only handle 3.3V max.
    FWD Voltage Output 3.5V typ. @ 20W Output power
    REF Voltage Output - reflected wave , can be used to calculate estimated output SWR
3. 

Based on a design by Robert de Kok, PA2RDK, and rewritten by PA0ESH & CHATGPT.
Sunday March 2nd 2025

