# ESP8266-based Relay IoT device 

## Idea 

This is a personal IoT project using C/C++ on the PIO editor to assemble an alternate current relay that could be controled remotely over the network using MQTT messages.

The end result is a cable extender that can be turned on and off from my laptop.

## Features

It uses MQTT as a communication protocol with the remote device controller by connecting to a mosquitto broker accessible by both the device and the controller. 

This controller can be another application fowarding orders to the device, or commands `mosquitto_pub` and `mosquitto_sub`. 

Connection to the MQTT broker is made via user and password credentials that must be defined beforehand on the broker. 

Broker credentials are hardcoded in the code.

All communications are encrypted using TLS. The CA certificate of the broker keys and the broker certificate fingerprint are also hardcoded in the device code. 

The main requirements were to setup a logic that could stay connected at all times and with reconnection capabilities, both the wifi and the MQTT broker. 

However, TLS encryption and broker authentication were also desirable to provide resistance against possible network intrusions. 

### Messages

Messages are propagated over two topics with the format ```wifi-rele/<n>/<x>```, where n is the number ID of a specific device and x is either 'r' or 'w': 

a. ```wifi-rele/<n>/r```: 
    - As an only subscription topic. 
    - Orders to device are received through this topic from the device controller. 

b. ```wifi-rele/<n>/w```:
    - As an only publishing. 
    - Used to publish device status and log messages. 

The system reacts to two messages or orders published on topic ```wifi-rele/<n>/r```:
- `OPEN` To open the relay and cut the AC current. 
- `CLOSE` To close the relay and activate AC current. 

## What I learned about

- Notions about software development for embebbed systems. 
- Development and deployment of MQTT distributed systems.  
- ESP8266 standard library. 
- Notions about network security using TLS/SSL on BearSSL library.  
- Soldering, electronics and AC current.

## Documentation 

*"If I have seen further than others, it is by standing upon the shoulders of giants."* 
-Isaac Newton

- [NTP docs](https://lastminuteengineers.com/esp8266-ntp-server-date-time-tutorial/)
- [Enabling TLS on ESP8266](https://raphberube.com/blog/2019/02/18/Making-the-ESP8266-work-with-AWS-IoT.html)
- [ESP8266 ans an IoT endpoint with encrypted mqtt transport](https://blog.thewalr.us/2019/03/27/using-esp8266-as-an-iot-endpoint-with-encrypted-mqtt-transport/)
