if [ "$#" -lt 1 ]; then
 	echo no input directory parameter
else
	if [ ! -d "$1" ]; then
		echo "Directory not exist"
		exit 0
	fi
	find $1 -type d
fi
