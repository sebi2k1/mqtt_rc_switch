all: main.cxx
	g++ -ggdb -DRPI -Irc-switch -Wall -pedantic -o mqtt_rc_switch main.cxx\
		rc-switch/RCSwitch.cpp -lwiringPi -lmosquittopp
