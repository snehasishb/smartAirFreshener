# smartAirFreshener
1. The air freshener unit is operated by 2 X AA batteries, resulting in a nominal supply voltage of 3V DC. This derives the air fresheners is designed to operate within this 3V DC range. In alignment with this standard, the output pin intended for interfacing with the air freshener has been configured to deliver a regulated 3V DC. You should determine the required operating voltage for your specific air freshener based on the same factors.
2. Make sure you have Home Assistant (https://www.home-assistant.io/) server running in your local to implement this project.
3. After completing the initial version of the project, I observed that a 2500mAh battery lasted approximately one day under continuous operation with the original code. Upon reviewing the codebase, I identified opportunities to optimize power consumption. To improve power efficiency, I implemented several optimizations. This included enabling the deep sleep function, restricting Wi-Fi connectivity to wake-up intervals only, and removing the MQTT-based on/off control previously integrated with Home Assistant, which was causing unnecessary background activity. As a result of these enhancements, the battery life of the device increased significantly—from approximately one day to nearly two weeks using a 2500mAh battery.
4. Electronics Components:
#ESP-WROOM-32 (ESP32 Dev Board)
#1 Channel Relay Module 5V High and Low Level Trigger Relay Module
#Mini DC-DC Boost Converter 0.9V~5V to 5V 600MA USB Output charger step up Power Module
#Type-C USB 5V 2A Step-Up Boost Converter with USB Charger
#18650 SMD/SMT High-Quality Single Battery Holder
#DMEGC INR18650-26E 3.7V 2600mAh Li-Ion Battery
#2 X diode 1N4007
#2.1×5.5mm DC Power Jack Socket Panel Mount (Female) 
#Slide Switch SS-12D00 1P2T

![AirFreshner Diagram](https://github.com/user-attachments/assets/9d40beff-a252-4c45-807f-9604b397f0ba)
