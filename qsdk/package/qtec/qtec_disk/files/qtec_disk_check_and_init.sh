#!/bin/sh 

#wjianjia
#用于磁盘检测和磁盘恢复出厂化的脚本

qtec_init_lock="/tmp/.qtec_init_lock"

if [ -f $qtec_init_lock ]
then
	#echo "====already one qtec_init_disk script is running, so we exit=====\n"
	exit;
else 
	#echo "===no other qtec_init_disk script running, so let we lock it ====\n"
	touch $qtec_init_lock
fi

usage()
{
	echo "qtec_disk_check_disk_init.sh [option-num]"
	echo "====0: just check whether the disk is inited and then exit===\n"
	echo "====1: if the disk is not inited, init it====================\n"
	echo "====2: forced to init disk===================================\n"
	return 0;
}

#1: counld not find disk /dev/sd*
#2: /dev/sd* 的uuid或label 不正确
#3：不存在/dev/sd*1
check_disk()
{
	echo "===========check_disk====================="

	local dev=`ls /dev/ | grep '^sd[a-z]$'`

	#如果/dev/sd*不存在，则报错
	if [[ -z $dev ]]
	then
		echo "===can not find disk ====\n"
		return 1;
	fi

	#比对/dev/sda 的uuid 和label
	local label=`blkid /dev/$dev -s LABEL | awk -F= '{printf $2}' | awk -F "\"" '{printf $2}'`
	if [[ -z $label ]]
	then 
		echo "===can not find label===\n"
		return 2;
	fi

	echo $label > /tmp/.qtec_check_label
	
	local uuid=`blkid /dev/$dev -s UUID | awk -F= '{printf $2}' | awk -F "\"" '{printf $2}'`
	if [[ -z $uuid ]]
	then 
		echo "===can not find uuid ====\n"
		return 2;
	fi

	echo $uuid > /tmp/.qtec_check_uuid

	#label 应该为 "qtec-uuid前一部分"
	local str1=`echo $uuid | awk -F "-" '{printf $1}'`
	local str2=qtec-$str1

	if [ $label == $str2 ]
	then
		echo "====label uuid match ===\n"
	else 
		echo "===label uuid not match ====\n"
		return 2;
	fi

	#如果/dev/sd*1不存在,则报错
	local dev1=`ls /dev/ | grep 'sd[a-z]1'`
	if [[ -z $dev1 ]]
	then 
		echo "===can not find partion ===\n"
		return 3;
	fi

	echo "it is qtec disk, we can use it"
	return 0;

	
}

init_disk()
{
	echo "==========init_disk======================"

	local dev=`ls /dev/ | grep '^sd[a-z]$'`

	#如果/dev/sd*不存在，则报错
	if [[ -z $dev ]]
	then
		echo "===can not find disk ====\n"
		return 1;
	fi

	echo y | mkfs.ext4 /dev/$dev 

	local uuid=`blkid /dev/$dev -s UUID | awk -F= '{printf $2}' | awk -F "\"" '{printf $2}'`
	if [[ -z $uuid ]]
	then 
		echo "===can not find uuid ====\n"
		return 2;
	fi

	#label 应该为 "qtec-uuid前一部分"
	local str1=`echo $uuid | awk -F "-" '{printf $1}'`
	local str2=qtec-$str1

	e2label /dev/$dev $str2
	
	#给/dev/sda 划分一个新的分区
	echo -e "n\np\n1\n\n\nw\n" | fdisk /dev/$dev 
}


main()
{
	input=$1;
	if [ $input -eq 2 ]
	then
		init_disk;
		return;
	fi
	
	check_disk;
	check_result=$?;

	if [ $input -eq 0 ]
	then
		return;
	fi

	if [ $check_result -eq 0 ]
	then 
		return;
	fi
	
	init_disk;
	return;
	
}

if [ $# -eq 1 ] && [ $1 -eq 0 -o $1 -eq 1 -o $1 -eq 2 ]
then 
	main $1
else
	usage
fi

rm $qtec_init_lock

