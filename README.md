## pH to I2C v2.0
pH metering with Automatic Temperature Compensation for Arduino and other devices. 
__________

## pH measurement


Connect the module to I2C bus the microcontroller board via cable Grove or Arduino pin headers.


![pH to I2C](http://image.cyber-plant.com/var/resizes/pHtoI2C_pH_measurement.jpg?m=1447442262)



See the sample code *PhMeasurementSerial.ino*

![pH to I2C](http://image.cyber-plant.com/var/resizes/PhSerial.png?m=1447486499)

## pH and Temp measurement

Switch between the temperature and pH sensors is available through the SPI interface. Sensors reading at alternately via I2C bus.


![pH to I2C](http://image.cyber-plant.com/var/resizes/pHtoI2C_pH%26temp_measurement.jpg?m=1447449390)

For activate the control function on SPI cut off three jumpers on the bottom side of the board. Solder PLC headers to pin SCLK (D13), SDI (D11) and CSB (D8, D9 or D10). Solder the temperature sensor pt100 and 100 ohm resistor (blue)

![pH to I2C](http://image.cyber-plant.com/var/resizes/pHtoI2C_SPI_set.jpg?m=1447450340)

See the sample code *PhAndTempMeasurementSerial.ino*

![pH to I2C](http://image.cyber-plant.com/var/resizes/PhTempSerial.png?m=1447486499)

For more information, about the control through the SPI interface, see datasheet LMP91200

## Connecting multiple devices

![pH to I2C](http://image.cyber-plant.com/var/resizes/pHtoI2C_pullUp.jpg?m=1447452013)

Connecting multiple pHtoI2C modules to a single bus is trivial.
Up to eight modules (provided their addresses
are different) can be connected to a single I2C bus.
Note that only one set of pull-up resistors is needed per
bus.

For more information about connect to I2C bus, see datasheet ADS1110

_______________________________________

[CyberPlant LLC ](http://www.cyberplant.info).

14/11/2015