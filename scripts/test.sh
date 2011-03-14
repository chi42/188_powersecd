#!/bin/bash

# you need to be root to run

# note that the client will need to be a modified example client that
# dies immediately after recieving a signal

POWERSECD=/home/nwong/188_posersecd/bin/powersecd
CLIENT=/home/nwong/188_powersecd/bin/client

for jj in $(seq 1 100)
do

  echo "*************GO"
  rm /var/run/powersecd.pid -f

  /usr/local/bin/start_sampling 2> /ramdisk/nwong_data.txt &
  SAMPLER_PID=$!
  echo $SAMPLER_PID
  sleep 15 
  
  echo "*************running..."
  chrt 99 $POWERSECD 
  for i in $(seq 1 99) 
  do 
    $CLIENT > /dev/null &
  done
  $CLIENT > /dev/null 

  echo "*************done running, sleeping to collect data"
  sleep 10 

  kill -s 2 $SAMPLER_PID
  sleep 1
  kill -s 2 $SAMPLER_PID
  sleep 1
  kill -s 2 $SAMPLER_PID
  sleep 1

  /usr/local/bin/sync.py /ramdisk/nwong_data.txt > /ramdisk/nwong_sync-data.txt
  /usr/local/bin/user_caliper_report.py /ramdisk/nwong_sync-data.txt >> 100_out.txt
 
done

