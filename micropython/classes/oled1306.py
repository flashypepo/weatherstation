# class OLED for ssd1306 oled-displays
# 2018_0107 wrapper for ssd1306, added draw_bitmap from mchauser:
#       defaults for WeMOS OLED shield - 
#       see https://wiki.wemos.cc/products:d1_mini_shields:oled_shield
#
# https://forum.micropython.org/viewtopic.php?t=1705&start=20
# Example:
# smiley = [7,192,24,48,32,8,64,4,64,4,255,254,167,154,175,186,156,114,128,2,64,36,67,196,32,8,24,48,7,192]
# display.draw_bitmap(24, 20, smiley, 15, 15, 1)

import ssd1306   # MP1.9.3+: builtin ssd1306

__version__ = '0.1.0'
__author__ = 'Peter van der Post'
__license__ = "Apache License 2.0. https://www.apache.org/licenses/LICENSE-2.0"

class Display:
    def __init__(self, i2c, width=64, height=48, addr=0x3c, external_vcc=False):
        self.__oled = ssd1306.SSD1306_I2C (width, height, i2c, addr, external_vcc)

    # 2018-0107 PePo added, tested 2017-0107: ok with smiley.
    def draw_bitmap(self, x, y, bitmap, w, h, col=1):
        byteWidth = (w + 7) // 8
        for j in range(h):
            for i in range(w):
                if i & 7:
                    byte <<= 1
                else:
                    byte = bitmap[byteWidth * j + i // 8]
                if byte & 0x80:
                    self.__oled.framebuf.pixel(x + i, y + j, col)

    # wrapper methods: delegate actions to self_oled
    @property
    def width(self):
        return self.__oled.width
        
    @property
    def height(self):
        return self.__oled.height

    #''' 2018_0107 needed?
    @property
    def pages(self):
        return self.__oled.pages
    #'''

    ''' 2018_0107 needed?
    @property
    def framebuffer(self):
        return self.__oled.framebuf
    #'''


    def poweron(self):
        self.__oled.poweron()
        
    def poweroff(self):
        self.__oled.poweroff()

    def contrast(self, contrast):
        self.__oled.contrast(contrast)

    def invert(self, invert):
        self.__oled.invert(invert)

    def fill(self, col):
        self.__oled.fill(col)

    def pixel(self, x, y, col):
        self.__oled.pixel(x, y, col)

    def scroll(self, dx, dy):
        self.__oled.scroll(dx, dy)

    def text(self, msg, x, y, col=1):
        self.__oled.text(msg, x, y, col)

    #2018_0107 PePo: preferable method to use
    def update(self):
        self.__oled.show()

    #compatibility with ssd1306
    def show(self):
        self.update()

    # activate (val=True) or deactivate(val=False) display
    #from resource: thesheep-micropython-oled
    def active(self, val):
        i2c = self.__oled.i2c
        addr = self.__oled.addr
        i2c.writeto_mem(addr, 0x00, b'\xaf' if val else b'\xae')
