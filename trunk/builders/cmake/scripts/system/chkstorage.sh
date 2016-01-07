minSpace=$1
dirPath=$2
needSpace=-1
total=0

ChkStorage()                                    
{                                               
  freeSpace=$(df | grep dev/ | awk '{ print $4 }' | tr -d '\n')
  echo "freeSpace:$freeSpace"
  #if [ "$freeSpace" -lt "$minSpace" ]; then                           
    #let "needSpace = $minSpace - $freeSpace"                          
    needSpace = "$minSpace - $freeSpace"                          
    echo "needSpace:$needSpace"
  #fi                                                           
}  



if [ "$#" -lt 2 ]; then
	echo need input parameter
else
	if [ ! -d "$2" ]; then
		echo "Directory not exist"
		exit 0
	fi
	ChkStorage
	if [ "$needSpace" -lt 0 ]; then                           
		echo "Space enough"
		exit 0
	fi
	
	fileArray=$(find ./ -name "*.mp4" | xargs stat -c "%Y %s %n " | sort | head -n 20)
	IFS=' '

	let "needSpace = $needSpace * 1024"
	echo "needSpace:$needSpace"
        fileIndex=0
	for file in $fileArray; do
                let "modValue = $fileIndex % 3"
                if [ "$modValue" -eq 0 ]; then
		 	if [ "$total" -gt $needSpace ]; then
			exit 0
			fi
                elif [ "$modValue" -eq 1 ]; then
			let "total = $total + $file"
			echo $total
                else
			echo "remove $file"
			rm $file
                fi
		let "fileIndex = $fileIndex + 1"
	done

fi
