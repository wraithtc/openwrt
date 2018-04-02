#!/bin/sh -xv

#wjianjia
#用于磁盘格式化，加密, 且 挂载

echo "===wjj is a nice boy ====\n"
qtec_disk_reformat_lock="/tmp/.qtec_disk_lock"


#运行程序直至成功
repeat()
{
	while true
	do
		$@ && return
		uci set qtec_disk.status.status_errorcode=3001
		uci set qtec_disk.status.status_message='get key fail from spi device'
		uci get qtec_disk.status.status_errorcode
		uci get qtec_disk.status.status_message
		sleep 2
	done
}

if [ -f $qtec_disk_reformat_lock ]
then
	echo "====already one qtec_disk_reformat script is running, so we exit=====\n"
	exit;
else 
	echo "===no other qtec_disk script running, so let we lock it ====\n"
	touch $qtec_disk_reformat_lock
fi

#//游戏开始
uci set qtec_disk.status.status_errorcode=3000
uci set qtec_disk.status.status_message="init status code for status"
uci set qtec_disk.process.process_statuscode=4000
uci set qtec_disk.process.process_message="init status code for prcoess"

#//debug 用
uci show qtec_disk

#重新格式化前，先尝试解绑和关闭
umount /mnt/sda1
cryptsetup close qtec_disk

uci delete qtec_disk.status.status_key

uci set qtec_disk.process.process_statuscode=4001
uci set qtec_disk.process.process_message="getting key from spi device"
#获取mainkey 直至成功
repeat qtec_getkey

password=`uci get qtec_disk.status.status_key`
echo $password

if [[ -z $password ]]
then 
	echo "====fail get password form uci ====\n"
	uci set qtec_disk.status.status_errorcode=3001
	uci set qtec_disk.status.status_message='get key fail from spi device'
	uci set qtec_disk.process.process_statuscode=4999
	uci set qtec_disk.process.process_message='exit in fail status'
	uci commit qtec_disk
	rm -rf $qtec_disk_reformat_lock
	exit
fi

uci set qtec_disk.process.process_statuscode=4002
uci set qtec_disk.process.process_message="checking disk"
#搜寻磁盘（暂时只考虑只有一个分区的情况）
dev=`ls /dev/ | grep 'sd[a-z]1'`

echo $dev

#如果dev为空，则表明磁盘未找到
if [[ -z $dev ]]
then
	echo "===can not find disk ====\n"
	uci set qtec_disk.status.status_errorcode=3002
	uci set qtec_disk.status.status_message='can not find disk device'
	uci set qtec_disk.process.process_statuscode=4999
	uci set qtec_disk.process.process_message='exit in fail status'
	uci commit qtec_disk
	rm -rf $qtec_disk_reformat_lock
	exit
fi

uci set qtec_disk.process.process_statuscode=4003
uci set qtec_disk.process.process_message="formating disk" 
cryptsetup --cipher aes-xts-plain64 --key-size 512 --hash sha512 --iter-time 10000 luksFormat /dev/$dev <<EOF
$password
$password
EOF

if [ $? -eq 0 ]
then 
	echo "===success format disk=====\n"
else
	echo "fail format disk, maybe dis is in use===\n"
	uci set qtec_disk.status.status_errorcode=3005
	uci set qtec_disk.status.status_message='fail format disk,please check disk is not in use'
	uci set qtec_disk.process.process_statuscode=4999
	uci set qtec_disk.process.process_message='exit in fail status'
	uci commit qtec_disk
	rm -rf $qtec_disk_reformat_lock
	exit
fi

#//打开磁盘
uci set qtec_disk.process.process_statuscode=4004
uci set qtec_disk.process.process_message="luksOpening disk"
cryptsetup luksOpen /dev/sda1 qtec_disk -T 1 <<EOF
$password
EOF

result=$?
if [ $result -eq 0 ]
then
	echo "===success open disk with password====\n"
elif [ $result -eq 5 ]
then
	echo "===disk alread open===\n"
	
else
	echo "===fail open disk with password======\n"
	uci set qtec_disk.status.status_errorcode=3003
	uci set qtec_disk.status.status_message='can not open disk with password,need user check to re-format disk'
	uci set qtec_disk.process.process_statuscode=4999
	uci set qtec_disk.process.process_message='exit in fail status'
	uci commit qtec_disk
	rm -rf $qtec_disk_reformat_lock
	exit
fi

#//创建文件系统
uci set qtec_disk.process.process_statuscode=4005
uci set qtec_disk.process.process_message="creating filesystem(ext4) in disk"
mkfs.ext4 /dev/mapper/qtec_disk

uci set qtec_disk.process.process_statuscode=4006
uci set qtec_disk.process.process_message='mounting disk'
mkdir -p /mnt/sda1
mount /dev/mapper/qtec_disk /mnt/sda1

echo $?
result_string=`mount | grep qtec_disk`

if [[ -z "$result_string" ]]
then
	echo "===fail mount ====\n"
	uci set qtec_disk.status.status_errorcode=3004
	uci set qtec_disk.status.status_message='fail mount filesystem'
	uci set qtec_disk.process.process_statuscode=4999
	uci set qtec_disk.process.process_message='exit in fail status'
	uci commit qtec_disk
	rm -rf $qtec_disk_reformat_lock
	exit
fi

uci set qtec_disk.status.status_errorcode=0
uci set qtec_disk.status.status_message='success!!!'

uci set qtec_disk.process.process_statuscode=0
uci set qtec_disk.process.process_message="exit in success status"
chmod 777 /mnt/sda1
rm -rf $qtec_disk_reformat_lock
exit 
