#!/bin/bash

## Defines - Change these to match your setup
## NOTE: Password is not encrypted!

server="192.168.143.100"
username="pi"
password="-p EcoDome"
ssh_port="22"

#server="localhost"
#username="pi"
#password="-p EcoDome"
#ssh_port="3221"

#path to save the project
#project="EcoDome_Main_dev"
project="EcoDome_Main_activeBuild"
path="/home/$username/programming/$project/"

## Uploading and running make
echo "Uploading..."

sshpass $password ssh -p$ssh_port $auth $username@$server "mkdir -p $path && cd $path && make clean"
sshpass $password scp -P$ssh_port -rp ./* $username@$server:$path

echo "Upload Complete!"
echo "Running 'make' on final destination..."

sshpass $password ssh -p$ssh_port $username@$server "cd $path && make"
