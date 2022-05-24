# mh-z19b CO2 Sensor

This is an example project which allows to send array messages from an mh-z19b sensor to ros2
The code can be modified according to application.

------------

Sample video pending [..](.. "..").

------------

### List of components used:
- [ESP 32](https://www.az-delivery.de/en/products/esp32-developmentboard "ESP 32")
- [mh-z19b CO2 Sensor](https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19b-co2-ver1_0.pdf "mh-z19b CO2 Sensor") 

------------

### About
This example uses a CO2 sensor connected to esp32 board. The serial reading from CO2 sensor are sent over ROS2 via micro-ros. The values are in ppm according to the datasheet.
In ROS2, these values are published to mhz_19b topic to demonstrate the output for this example. The value is an array. First value is counter and second value is co2 ppm value.

------------

### Tutorial
1. Follow the  [tutorial here](https://link.medium.com/pdmyDUIh9nb "tutorial here") to setup the micro-ros environment.
2. Clone this repository and copy the folder 'mhz19b_microros' inside your workspace such that folder structure is as  [YOUR WORKSPACE]/firmware/freertos_apps/apps/mhz19b_microros 
3. Open terminal can follow these commands:
- ros2 run micro_ros_setup configure_firmware.sh mhz19b_microros -t udp -i [LOCAL MACHINE IP ADDRESS] -p 8888
- ros2 run micro_ros_setup build_firmware.sh
- ros2 run micro_ros_setup flash_firmware.sh
4. Once the flashing is successful, run
- ros2 run micro_ros_agent micro_ros_agent udp4 --port 8888
5. Open new terminal window, and check ros topic
- ros2 topic echo /mhz_19b

------------

### Connections
|  ESP32  |  mhz_19b Sensor  |
|  ------------ |  ------------ |
|  G12  |  RX  |
|  G13  |  TX  |
|  5V  |  VCC  |
|  GND  |  GND  |

------------

### Array message inputs
|  ESP32  |  ROS2 Topic  |
|  ------------ |  ------------ |
|  Counter  |  data[0]  |
|  CO2 PPM  |  data[1]  |
