#!/bin/bash


## Defines - Change these to match your setup
## NOTE: Password is not encrypted!
# login = "username@server"

DX60="85.27.205.112"
DX60_username="remusdc"
DX60_auth="-i ~/.ssh/DX64_DX60"
DX60_ssh_port="-p3220"
DX60_scp_port="-P3220"

# path to save the project
project="EcoDome_Main_TEST"
path="~/programming/$project/"

## Uploading and running make
echo "Uploading to DX60..."

sshpass ssh $DX60_auth $DX60_ssh_port $DX60_username@$DX60 "mkdir -p $path"
sshpass scp $DX60_auth $DX60_scp_port -rp ./* $DX60_username@$DX60:$path

echo "Upload Complete!"
echo "Running 'make up' on DX60..."

sshpass ssh $DX60_auth $DX60_ssh_port $DX60_username@$DX60 "cd $path && make up"