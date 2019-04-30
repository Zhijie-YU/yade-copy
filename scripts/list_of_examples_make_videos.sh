#!/usr/bin/zsh
# INFO: run this script inside ./scripts
#       It can also be used to check if the examples are working. Only need to modify line the_examples=…… to use a different grep pattern
#       Then when asked '(Press Ctrl-C when recording is complete) Ready? [y/n] :' press n to skip recording. Then a new example is tested.
#       Also can start more examples simultaneously - just press 'n' to start next one without closing the previous one.

# This script automates creation of videos from examples. First it greps list_of_examples for pattern "...│...│no │"
# to identify examples which do not have a video uploaded.
# Then it runs a loop over all those examples. Opens two terminals, in first terminal there is yade running,
# ready to start simulation. In second terminal waits until Ready [y/n] to start recording.
# If 'n' is pressed, then it skips recording.


# this command I used to verify that I got a correct regexp for finding the examples.
#grep -E "...│...│no │" ../examples/list_of_examples.txt | grep -E "\<[A-Za-z_0-9-]+\>\.py\>" --color

# get a list of all examples which have "no" uploaded video
#grep -E "...│...│no │" ../examples/list_of_examples.txt | sed -e "s/.*\(\<[A-Za-z_0-9-]\+\>\.py\>\).*/\1/"

the_examples=("${(@f)$(grep -E "...│...│no │" ../examples/list_of_examples.txt | sed -e "s/.* \([A-Za-z_0-9-]\+\>\.py\>\).*/\1/")}")
#the_examples=("${(@f)$(grep -E "^\?  │...│...│" ../examples/list_of_examples.txt | sed -e "s/.* \([A-Za-z_0-9-]\+\>\.py\>\).*/\1/")}")

mkdir -p /tmp/video
mkdir -p /tmp/run
cp ../examples/yade /tmp/run

for example in $the_examples; do
	file_location=("${(@f)$(find ../ -name "${example}" -type f)}")
	print "Working on ${example} in ${file_location}"
	cp ${file_location} /tmp/run/${example}
	SUCCESS=$?
	if [ ${SUCCESS} -eq 0 ]; then
		BASENAME=`basename ${example} .py`
		xterm -en UTF-8 -b 0 -bg black -fg darkgray -geometry 102x37+0+0 -fn "-Misc-Fixed-Medium-R-Normal--20-200-75-75-C-100-ISO10646-1" -e bash -c "cd /tmp/run ; ./yade ./${example}" &!
		xterm -en UTF-8 -b 0 -bg black -fg darkgray -geometry 102x37+1000+1000 -fn "-Misc-Fixed-Medium-R-Normal--20-200-75-75-C-100-ISO10646-1" -e /usr/bin/zsh -c "\
		cd /tmp/video;\
		echo \"working on ${example} as ${BASENAME}\";\
		REPLY=\"n\";\
		read -q REPLY\\?\"(Press Ctrl-C when recording is complete) Ready? [y/n] :\";\
		echo \$REPLY;\
		if test \$REPLY = \"y\" ; then;\
			echo OK, recording ${example};\
			sleep 1;\
			cd /tmp/video
			recordmydesktop -x 0 -y 0 --width 1024 --height 768 -o ${BASENAME}.ogv --no-sound
		fi;
		"
		rm /tmp/run/${example}
	else
		echo "Error copying file ${example}"
		sleep 1
	fi
done

