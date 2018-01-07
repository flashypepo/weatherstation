# main startup
# 2018_01: weatherstation - version 0.1

import weatherstation
ws = weatherstation.WeatherStation() #create a Weatherstation object
ws.testsht30() # make measurements with SHT30
