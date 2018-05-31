# Made by Hans V. Rasmussen for EcoDome Bachelor Project.
# Heavily inspired by https://www.summet.com/dmsi/html/readingTheWeb.html
# Version:          0.1
# Creation Date:    19/02/2018 14:30
# Last Edit:        19/02/2018 14:30

import urllib.request
from re import findall

#Read the webpage:
response = urllib.request.urlopen("https://www.yr.no/place/Denmark/South_Denmark/S%C3%B8nderborg/forecast.xml")
html = response.read()
text = html.decode()

#print (text)        # For debugging

# Find the data chunk we want
validFrom = findall(r'Valid at \d*-\d*-\d*T\d*:\d*:\d*', text)
print (validFrom)

windDir = findall(r'windDirection deg="\d*[.]?\d*"', text)
print (windDir)

windSpeed = findall(r'windSpeed mps="\d*[.]?\d*"', text)
print (windSpeed)

temperature = findall(r'temperature unit="celsius" value="\d*[.]?\d*"', text)
print (temperature)

pressure = findall(r'pressure unit="hPa" value="\d*[.]?\d*"', text)
print (pressure)