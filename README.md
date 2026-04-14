# task_switch_simulator
This is a copy of my project from Embedded Systems lab at University

Hardware: ESP32, four pushbuttons (red, yellow, green, and blue), jumper wires

red = run task A (priority 1)
yellow = run task B (priority 2)
green = run task C (priority 3)
blue = run task D (priority 4)

Scheduler Simulator Commands
============================
- fq - Use Function Queue Scheduling
- rr - Use Round Robin Queue Scheduling
- show - Show service times for all devices
- reset - Reset all device service times to zero
- help - Show this help message

To set device service times, enter the device letter (a-d) followed by a space and the service time in ms. You may enter more than one device and service time by separating names and times with spaces. For example:

     a 3200 b 2500 c 1250 d 1234
To request service time for a device, press its button.
You may only request service for blocked devices (i.e.)
not in the Ready or Running state
