#!/bin/bash
echo "Uploading..."
sshpass -p "raspberry" scp -r ./* pi@168.192.43.128:/home/pi/EcoDome_Development/ControlTest/
sshpass -p "raspberry" ssh pi@168.192.43.128 "cd /home/pi/EcoDome_Development/ControlTest/ && make"

#sshpass -p "raspberry" scp -r ./* pi@192.168.1.38:/home/pi/EcoDome_Development/ControlTest/
#sshpass -p "raspberry" ssh pi@192.168.1.38 "cd /home/pi/EcoDome_Development/ControlTest/ && make"
echo "Upload Complete"
