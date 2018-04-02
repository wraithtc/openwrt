#!/bin/sh

#wjianjia
#check whether zigbee client works well

zigbee_result_file="/tmp/.zigbeecheck"

if [ -f $zigbee_result_file ]
then 
    echo "========zigbee  ok===========";
else 
    echo "=======zigbee not ready======";
fi
