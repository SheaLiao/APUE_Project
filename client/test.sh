#! /bin/bash 

i=0
count=100
while [ $i -le $count ] 
do
	/home/iot24/liaoshengli/project/client/client -i 192.168.2.40 -p 12345 &
	i=`expr $i + 1`
done

echo "All clients start"
