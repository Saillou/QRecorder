#!/bin/bash

# Functions
create() {
	if [ ! -d $1 ]
	then
		mkdir $1
	fi	
}
cleanup() {
	if [ -f $1 ]
	then
		rm $1
	fi	
}
makePathSources() {
	# Sources
	for source in "${Sources[@]}"
	do
		if [ -f "../../Sources/$source.cpp" ]
		then
			PathSources="$PathSources ../../Sources/$source.cpp"
		else
			echo "$source.cpp is missing"
			return 0
		fi
	done
	
	return 1
}
makePathObjects() {
	# Objects from source
	for source in "${Sources[@]}"
	do
		if [ -f "../Objects/$source.o" ]
		then
			PathObjects="$PathObjects ../Objects/$source.o"
		else
			echo "$source.o is missing"
			return 0
		fi
	done 
	
	return 1	
}


# Define environnement
create "Ubuntu"

cd "Ubuntu"
create "Objects"
create "Release"

cd "Release"
create "Tmp"
create "Recordings"
cd ../

OutputName="QRecorder"
declare -a Sources=(
	"mainGUI" 
)

PathSources=""
PathObjects=""


# Compile source
cd "Objects"
cleanup *.o
makePathSources
if [ $? == 0 ]
	then exit -1
fi

echo "Compile sources"
g++ -c -std=gnu++11 -O2 \
	-Wall \
	$PathSources \
	-I/usr/local/include/ 
cd ../


# Link objects
cd "Release"
cleanup $OutputName 
makePathObjects
if [ $? == 0 ]
	then exit -1
fi

echo "Link objects"
g++ -o $OutputName \
	$PathObjects \
	-L/usr/local/lib/ \
	-lopencv_core \
	-lopencv_highgui \
	-lopencv_imgproc \
	-lopencv_imgcodecs \
	-lopencv_videoio \
	-pthread \

# Launch
echo "-----------"
./$OutputName -q

exit 0

