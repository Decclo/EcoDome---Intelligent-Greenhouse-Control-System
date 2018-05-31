# Made by Hans V. Rasmussen for EcoDome Bachelor Project.
# Heavily inspired by https://www.summet.com/dmsi/html/readingTheWeb.html
# Version:          1.01
# Creation Date:    19/02/2018 14:30
# Last Edit:        05/03/2018 16:00

import urllib.request
from re import findall
import os, errno

## Defines:
directory = "./data/"


## NOTES:
# Note about numbers: 
#   '\d*[.]?\d*' accepts numbers such as '1' , '7' , '2.7' , '300.7933'
#   The '.' can be exchanged for a ',' or both can be used at the same time.

# print (validFrom[0])  # Describes how to read from variables
# windDir_compact = findall(r'\d+[.]?\d*', windDir[0])  # how to extract actual value from saved data


## Read the webpage:
response = urllib.request.urlopen("https://www.yr.no/place/Denmark/South_Denmark/S%C3%B8nderborg/forecast.xml")
html = response.read()
text = html.decode()


## Find the data chunk we want
validFrom = findall(r'Valid at \d*-\d*-\d*T\d*:\d*:\d*', text)
#print (validFrom)

windDir = findall(r'windDirection deg="\d*[.]?\d*"', text)
#print (windDir)

windSpeed = findall(r'windSpeed mps="\d*[.]?\d*"', text)
#print (windSpeed)

temperature = findall(r'temperature unit="celsius" value="\d*[.]?[-]?\d*"', text)
#print (temperature)

pressure = findall(r'pressure unit="hPa" value="\d*[.]?\d*"', text)
#print (pressure)


## Make sure that the path for the weatherdata exists:
if not os.path.exists(directory):
    os.makedirs(directory)


## Print the data to files:
for i in range(len(validFrom)):
    
    # Debugging and awesomeness
    print("\n")
    print(validFrom[i])
    print(windDir[i])
    print(windSpeed[i])
    print(temperature[i])
    print(pressure[i])

    # open file and prepare to write:
    filestring = directory + "wData" + str(i)
    file = open(filestring, "w")

    # prepare data to be written:
    windDir_tmp = findall(r'\d+[.]?\d*', windDir[i])
    windSpeed_tmp = findall(r'\d+[.]?\d*', windSpeed[i])
    temperature_tmp = findall(r'\d+[.]?\d*', temperature[i])
    pressure_tmp = findall(r'\d+[.]?\d*', pressure[i])

    # write data:
    file.write(validFrom[i] + "\n")
    file.write(windDir_tmp[0] + "\n")
    file.write(windSpeed_tmp[0] + "\n")
    file.write(temperature_tmp[0] + "\n")
    file.write(pressure_tmp[0] + "\n")

    # close file and prepare for next iteration
    file.close()