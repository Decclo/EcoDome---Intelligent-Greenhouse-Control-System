#!/bin/bash


## Defines - Change these to match your setup
## NOTE: Password is not encrypted!
# login = "username@server"

#server="192.168.1.34"
#username="remus"
#password="orange"

#server="192.168.43.128"
#username="pi"
#password="raspberry"

server="10.42.0.161"
username="pi"
password="raspberry"

# path to save the project
project="Window_Control"
path="/home/$username/programming/$project/"

## Uploading and running make
echo "Uploading..."

sshpass -p $password ssh $username@$server "mkdir -p $path && cd $path && make clean"
sshpass -p $password scp -rp ./* $username@$server:$path

echo "Upload Complete!"
echo "Running 'make' on remote machine..."

sshpass -p $password ssh $username@$server "cd $path && make"
