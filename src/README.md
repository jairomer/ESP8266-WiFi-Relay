# ESP8266-based Relay IoT device 

## Idea 

This is a personal IoT project of mine using C/C++ on the PIO editor to assemble an alternate current rele controller that could be controled remotely over the network using MQTT messages.

## Features

It uses MQTT as a communication protocol with the remote device controller by connecting to a mosquitto broker accessible by both the device and the mobile controller. 

Connection to the MQTT broker is made via user and password credentials that must be defined beforehand on the broker. 

The main requirements where to setup a logic that could stay connected at all times and with reconnection capabilities, both the wifi and the MQTT broker.

### Messages

Messages are propagated over two topics with the format ```wifi-rele/<n>/<x>```, where n is the number ID of a specific device and x is either 'r' or 'w': 

a. ```wifi-rele/<n>/r```: 
    - As an only read channel. 
    - Orders to device are received through this topic from the device controller. 

b. ```wifi-rele/<n>/w```:
    - As an only publish channel.
    - Used to publish device status and log messages. 

## What I learned about

- Notions about embebbed systems software development. 
- Development and deployment of MQTT distributed systems.  
- ESP8266 standard library. 
- Notions about network security using TLS/SSL.  
- Got to play with electronics and AC current!

## Problems 
Encountered some problems that need to be addressed in case of a more serious/commercial application of this project. 

I did not addressed these issues due to the fact that they would have increased the complexity of the project quite substantiantly.

- Confidential settings (users and passwords) hardcoded. Increases the possibility of password guessing tremendously. An attacker might achieve access to the mosquitto broker. 
    
- TLS encryption was difficult to implement due to the lack of appropriate documentation. 

- Had to solder pins on the upload board to be able to use it. 

## Documentation 

*"If I have seen further than others, it is by standing upon the shoulders of giants."* -Isaac Newton

[NTP docs](https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/)
[Enabling TLS on ESP8266](https://raphberube.com/blog/2019/02/18/Making-the-ESP8266-work-with-AWS-IoT.html)
[ESP8266 ans an IoT endpoint with encrypted mqtt transport](https://blog.thewalr.us/2019/03/27/using-esp8266-as-an-iot-endpoint-with-encrypted-mqtt-transport/)