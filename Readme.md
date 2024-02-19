# ESP8266 MQTT Relay Controller with AWS IoT

This project is designed for controlling a relay module using an ESP8266 microcontroller through MQTT messages from AWS IoT Core. It features WiFi configuration through WiFiManager, secure connection to AWS IoT Core, and handling of incoming MQTT commands to control the relay state.

## Prerequisites

- ESP8266 development board (e.g., NodeMCU, Wemos D1 Mini)
- Relay module compatible with ESP8266
- Arduino IDE installed on your computer
- AWS account and AWS IoT Thing set up

## Installation

1. Open the Arduino IDE and install the following libraries through Library Manager:
    - `ESP8266WiFi`
    - `WiFiManager`
    - `PubSubClient`
    - `ArduinoJson`
2. Clone this repository or copy the source code into a new Arduino sketch.

## Configuration

1. Update the `secrets.h` file with your own AWS IoT credentials:
    - Set `THINGNAME` to your AWS IoT Thing Name.
    - Fill in the `MQTT_HOST` with your AWS IoT endpoint.
    - Replace the contents between the `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----` markers in `cacert` with your AWS IoT root CA certificate.
    - Insert your AWS IoT Thing's certificate contents between the `-----BEGIN CERTIFICATE-----` and `-----END CERTIFICATE-----` markers in `client_cert`.
    - Place your AWS IoT Thing's private key contents between the `-----BEGIN RSA PRIVATE KEY-----` and `-----END RSA PRIVATE KEY-----` markers in `privkey`.
2. Set your time zone in the `TIME_ZONE` define.
3. Configure the `RELAY_PIN` to the correct GPIO pin connected to your relay.

## Usage

1. Upload the sketch to your ESP8266 device.
2. After uploading, the device will try to connect to the previously configured WiFi. If it's the first time or the device can't connect to the WiFi, it will start an Access Point named "Relay ESP8266 MQTT" for you to configure the WiFi credentials.
3. Once connected to the internet, the device will connect to AWS IoT Core and listen for incoming MQTT messages on the topic defined in `AWS_IOT_SUBSCRIBE_TOPIC`.
4. Send `on`, `off`, or `toggle` commands to `AWS_IOT_SUBSCRIBE_TOPIC` to control the relay.

## MQTT Messages

- **Publish Topic (`AWS_IOT_PUBLISH_TOPIC`):** The device publishes its state (on/off) on any state change
- **Subscribe Topic (`AWS_IOT_SUBSCRIBE_TOPIC`):** The device subscribes to this topic to listen for relay control commands.

## Troubleshooting

Ensure that your AWS IoT Thing is correctly set up and that the policy attached to it allows publishing and subscribing to the topics.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

