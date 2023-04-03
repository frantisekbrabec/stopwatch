# Two gate stopwatch

Measures time elapsed between two events ocurring at different locations (within WiFi range) with 1/10th of a second precision. Events can be triggered by simple button presses or by any other sensor. It utilizes two ESP32 boards (e.g., [Lolin D32](https://www.wemos.cc/en/latest/d32/d32.html)), one at each gate. The finish gate also has an OLED display (e.g., [0.96"](https://amzn.to/3Gcpkxz)) attached to display time and battery status.

When the starting gate is triggered, it sends a signal to the finish gate to reset and start the clock. It also includes its battery status. When the finish gate is triggered, the time is stopped. The display also shows the start and finish gates' current battery status (charge percentage).

By default, the trigger (e.g., a Normally Open (NO) button) is placed between GPIO 32 and GND. 

The code needs to be customized to your speficic hardware by entering the MAC addresses of your ESP32 processors.
