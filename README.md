# smart_shunt_meter

Precision Arduino based Current Meter

Uses an Arduino UNO WiFi Rev2, NX8048T050 Nextion display, ARD-LTC2499, and a 
Riedon SSA series current sensor in either a 100, 250, 500, or 1000 amp version.

The 24bit ADC on the ARD-LTC2499 matched with the SSA sensor will give better than +/-0.1% accuracy.
the ARD-LTC249 is very low noise and can measure up to 8 differential signals at +/-2.048 VDC
with either 7.5 or 15 samples per second.

The Riedon SSA sensor is amplified and has reinfoced isolation to 1500VDC (1000VAC). 
It is UL approved for both CAT II and CAT III uses.
