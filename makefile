all:	chip_speed
	sudo ./chip_speed
	battery.sh | grep charge

chip_speed:	chip_speed.cpp
	#sudo ./fast
	g++ -o chip_speed chip_speed.cpp
