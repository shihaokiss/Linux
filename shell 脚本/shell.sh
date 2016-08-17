#!/bin/bash

#/*数组*/
#array=("a" "b" "c")
#array[4]="d"
#array[5]="e"
#
#echo ${array[*]}
#echo ${#array[@]}

#/*函数*/
#fun()
#{
#	echo $1
#	echo $2
#	echo $3
#	echo $4
#	return 110
#}
#
#fun 1 2 3 4
#echo  $?

#/*位置参数和特殊变量*/
#echo $0 
#echo $1 
#echo $2 
#echo $@ 
#echo $# 
#echo $? 
#echo $$ 
#
#shift 2
#echo $@ 
#echo $# 

#/*循环语句*/

#count=0
#while [ $count -lt 100 ]
#do
#echo "$count time"
#((count++))
#done


#for i in {a..z}
#do 
#	echo $i
#done
#
#echo '\n\n'
#
#for((i=0;i<100;i++))
#do 
#	echo "i am $i"
#done








#read val
#case $val in
#	a )
#		echo "a"
#	;;
#	b )
#		echo "b"
#	;;
#	c )
#		echo "c"
#	;;
#esac


#[ 'boy' == "girl" ] || [ 'boy' == "boy" ]
#echo $?

#str="zhangmi"
#if [ $str == "zhangmi" ];then
#	echo "yes1"
#elif [ "zhangjia" == "zhangmi" ];then
#	echo "yes2"
#else 
#	echo "yes3"
#fi

#val=100
#str1="shihao"; str2="shihao"
#[ ! $val -eq 100 ] 
#echo $?
#[ $val -ne 100  -a  $str1 ==  $str2 ]
#echo $?
#[ $val -eq 100  -o  $str1 ==  $str2 ]
#echo $?

#`test $val -ge 100`
#echo $?
#`test $val -le 100`
#echo $?
#`test $val -gt 100`
#echo $?
#`test $val -lt 100`
#echo $?
#`test $val -eq 100`
#echo $?
#$(test $val -ne 100)
#echo $?

#echo "`data`"
#echo "value is ${val}, `ls` \$ \" \` \\"
#echo 'value is ${val}, `ls` \$ \" \` \\'

#data=$`((10*110))`
#echo $data
#
#echo $((10+10))
#echo $((10-10))
#echo $((10*10))
#echo $((10/10))



#a=100
#echo $a
#echo $all
#echo ${a}ll
#echo $b
#cd ..;ls -l
