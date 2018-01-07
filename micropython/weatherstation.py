# WeatherStation application
#
# Configuration:
#   WeMOS D1 mini v2.1.0, I2C: SCL=D1/GPIO5, SDA=D2/GPIO4
#     OLED shield v1.1.0, I2C: 0x3C (default)
#     SHT30 temperature shield v1.0.0, I2C: 0x44 (default)
#     MicroSD shield
#   Battery shield
#   triple base
#
# Software:
#   Micropython v1.9.*
#   SHT30 class/module - TBD
#   MicroSD class/module - TBD
#
# WeMOS sources:
#  D1 mini: https://wiki.wemos.cc/products:d1:d1_mini
#  SHT30 temperature shield: https://wiki.wemos.cc/products:d1_mini_shields:sht30_shield

from micropython import const
import machine
import time

# OLED 1306 shield
from classes import oled1306

# SHT30 temperature and humidity sensor
from classes import sht30

# I2C specifications
__GPIO_SCL = const(5)       # D1 mini: D1 / GPIO5
__GPIO_SDA = const(4)       # D1 mini: D2 / GPIO4
__SHT30_ADDR = const(0x45)  # SHT30 D1 mini shield I2C addr

# OLED shield specifications
__OLED_ADDR = const(0x3c)   # OLED D1 mini shield I2C addr
__OLED_WIDTH = const(64)    # OLED width in pixels
__OLED_HEIGTH = const(48)   # OLED height in pixels


class WeatherStation:

    def __init__(self):
        self.__smiley = [7, 192, 24, 48, 32, 8, 64, 4, 64, 4, 255, 254, 167, 154, 175, 186, 156, 114, 128, 2, 64, 36,
                         67, 196, 32, 8, 24, 48, 7, 192]

        # create i2c object
        self.__i2c = machine.I2C(scl=machine.Pin(__GPIO_SCL), sda=machine.Pin(__GPIO_SDA))
        # create OLED object nd name it display
        self.__display = oled1306.Display(self.__i2c, __OLED_WIDTH, __OLED_HEIGTH, __OLED_ADDR)

        # create a WeMOS SHT30 shield object
        self.__sensor = sht30.SHT30() # WeMOS SHT30 shield defaults
        #expanded: self.__sensor = sht30.SHT30(__GPIO_SCL, __GPIO_SDA, 0, 0, __SHT30_ADDR)


    def testdisplay(self):
        # display: white page
        self.__display.fill(1)
        self.__display.update()
        # showtime
        self.__time.sleep(1)

        # smiley in center
        self.smile(24, 20)

    # display smiley at (x,y)
    def smile(self, x=24, y=20):
        self.__display.draw_bitmap(x, y, self.__smiley, 15, 15, 1)
        self.__display.update()


    def displaySensorData(self, t, h):
        print('Temperature:{0:4.2f} ºC, RH:{1:4.2f} %'.format(t, h))

        #format sensordata for OLED-display
        tempMsg = 'T:{0:4.1f} C'.format(t)
        humMsg = 'H:{0:4.1f} %'.format(h)

        #clear screen, display header and sensor data
        self.__display.fill(0) #blank display
        self.__display.text(' WEATHER', 0, 0)
        self.__display.text(tempMsg, 0, 10)
        self.__display.text(humMsg, 0, 20)
        self.__display.update()  #update screen


    # test SHT30 sensor, waittime in seconds
    def testsht30 (self, waittime = 10):
        try:
            if not self.__sensor.is_present():
                print('SHT30 shield is not connected')
                self.__display.text('no SHT30', 0, 0)
                self.__display.update() #update screen
                pass
            print('Start measurements...')

            while True:
                t, h = self.__sensor.measure()
                self.displaySensorData(t, h)
                time.sleep(waittime)

        except sht30.SHT30Error as ex:
            print('Error:', ex)

            # OLED-message
            self.__display.fill(0)
            self.__display.text(' WEATHER', 0, 0)
            self.__display.text('SHT30', 0, 15)
            self.__display.text(ex.get_message(), 0, 30)
            self.__display.update()  # update screen

        except:
            print('done')
            self.smile(20, 30)
            self.__display.update()  # update screen

# execute if its the main program
if __name__ == "__main__":
    ws = WeatherStation()
    ws.testsht30()
