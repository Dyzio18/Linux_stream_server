#!/bin/bash
server_pid=$1

echo "PID: $server_pid "
# ksiega 12 slowa co 32 sig 12
lxterminal -e ./client.out -r 12 -o 32 -f s -p .info -x 12 -s $server_pid
# ksiega 12 slowa co 64 sig 12
lxterminal -e ./client.out -r 12 -o 64 -f s -p .info -x 12 -s $server_pid
# ksiega 1 linia co 16 sig 8
lxterminal -e ./client.out -r 8 -o 16 -f l -p .info -x 1 -s $server_pid
# ksiega 1 linia co 128 sig 9
lxterminal -e ./client.out -r 9 -o 128 -f l -p .info -x 1 -s $server_pid

sleep 2
# ksiega 2 znaki co 44 sig 9 
lxterminal -e ./client.out -r 9 -o 44 -f z -p .info -x 2 -s $server_pid
sleep 1
# ksiega 2 znaki co 4 sig 9 
lxterminal -e ./client.out -r 9 -o 4 -f z -p .info -x 2 -s $server_pid
