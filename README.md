![EC Reader](http://image.cyber-plant.com/var/resizes/CyberPlantMiniSeries-01.jpg?m=1458131397)

##CyberPlant pH Mini v3.0

pH metering with Automatic Temperature Compensation for Arduino and other devices. The pH Mini supports the connection of a pH electrode with BNC and temperature sensor Pt100 / Pt1000 for temperature compensation of readings. To transfer the readings to the device module uses the I2C. You can connect many  modules to a single I2С bus at the same time busy will be only two pins at microcontroller. The CyberPlant pH Mini is perfect for automating hydroponic systems or aquariums, will be useful in laboratories or to collect data on the computer.

##Features:

- Power Supply: 2.7V to 5.5V
- Measuring Range pH: 0-14
- Accurate pH readings: ± 0.01 pH
- Temp sensor support: pt100/pt1000
- Accurate temperature readings::  ±0.3°C
- BNC Connector
- Can use as Shield for Arduino Pro Mini
- PCB Size : 33.02 mm×17.78 mm

## pH measurement


Connect the module to I2C bus the microcontroller board via cable Grove or Arduino pin headers. 
Connect the pH electrode to BNC connector. When first connecting need send the command "R" to configure the microcontroller.

![pH to I2C](http://image.cyber-plant.com/var/resizes/PHminiBaner1.jpg?m=1458074438)



See the sample code *PhMeasurementSerial.ino*

In sample code was used in one touch calibration sensor function.



## pH and Temp measurement

Switch between the temperature and pH sensors is available through the SPI interface. Sensors reading at alternately via I2C bus.




For activate the control function on SPI cut off three jumpers on the bottom side of the board. Solder PLC headers to pin SCLK (D13), SDI (D11) and CSB (D8, D9 or D10). Solder the temperature sensor pt100 and 100 ohm resistor (blue)



See the sample code *PhAndTempMeasurementSerial.ino*



For more information, about the control through the SPI interface, see datasheet LMP91200


## Isolated sensor

To eliminate electrical noise use [I2Ciso Module](https://github.com/cyberplantru/I2C-iso/). The I2Ciso recommended to when reading pH and Conductivity together.

![pH to I2C](http://image.cyber-plant.com/var/resizes/PHminiBaner3%2Cjpg.jpg?m=1458077695)


_______________________________________

[link to CyberPlant](http://www.cyber-plant.com).

