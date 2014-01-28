#! /usr/bin/env bash

# Script to test the SFD files generated by the glyphRen program.
# Generate new SFD files for all the SFD files in the currect directory
# based on the specified reference file.
# Fonts are generated using original and new SFD files. Test data is 
# rendered using fonts generated from both original and new SFD files.
# The md5sum of the resulting images are compared to find if there is any
# difference in rendering.

# Log functions
. logs.sh

function usage ()
{
	echo "Usage : `basename $0` -r RefFile -t TextToRender"
	echo -e "\t -r : Reference file"
	echo -e "\t -t : File with text to render"
	echo -e "\t -d : Enable debug messages"
	echo -e "\t -h : Help"
}

while getopts r:t:hd args
do
	case $args in 
	r) refFile="$OPTARG"
		;;
	t) testData="$OPTARG"
		;;
	h) usage
		exit 0
		;;
	d) dbgFlag=Y
		;;
	*) usage
		exit 1
		;;
	esac
done

if [[ -z "$refFile" ]]
then
	err $LINENO "Reference file not specified, try `basename $0` -h"
	exit 1
fi

if [[ -z "$testData" ]]
then
	err $LINENO "Test data file not specified, try `basename $0` -h"
	exit 1
fi

export PATH=$PATH:.

ls *.sfd > fonts.lst
while read fontFile
do
	fontName=`basename $fontFile .sfd`
	log $LINENO "Processing $fontName"	
	
	# Rename the glyphs.
	newSFD="${fontName}.j.sfd"
	log $LINENO "Renaming glyphs in $fontFile"
	glyphRen -l TRACE -r $refFile -i $fontFile -o ${newSFD} > ${fontName}.log

	# Generate the fonts.
	# generate.pe "$fontFile" > /dev/null 2>&1

	log $LINENO "Generating font for $newSFD"
	# generate.pe "${newSFD}" > /dev/null 2>&1

	export FONTFORGE_LANGUAGE=ff
	log $LINENO "Generating font for $fontFile"
	fontforge -c 'Open($1); Generate($2);' $fontFile ${fontName}.ttf 2> /dev/null
	fontforge -c 'Open($1); Generate($2);' $newSFD ${fontName}.j.ttf 2> /dev/null


	# Render the text
	log $LINENO "Rendering $testData for ${fontName}.ttf"
	hb-view --font-size=20 ${fontName}.ttf  < $testData > $fontName.png

	log $LINENO "Rendering $testData for ${fontName}.j.ttf"
	hb-view --font-size=20 ${fontName}.j.ttf  < $testData > ${fontName}.j.png
	echo ""

done < fonts.lst

# Compare the rendering
while read fontFile
do
	fontName=`basename $fontFile .sfd`
	log $LINENO "Verifying tests results for $fontName"

	refRender=${fontName}.png
	newRender=${fontName}.j.png

	refSum=`md5sum < $refRender`
	dbg $LINENO "Ref sum [$refSum]"

	newSum=`md5sum < $newRender`
	dbg $LINENO "Ref sum [$newSum]"

	if [[ "$refSum" = "$newSum" ]]
	then
		log $LINENO "$fontName : Rendered same, test success."
	else
		err $LINENO "$fontName : Rendered different, test failed. =============="
	fi

	grep StartChar ${fontName}.sfd | sort |uniq -c | sort -n > ${fontName}.count
	uCount=`grep -c -v "^ *1" ${fontName}.count`
	dbg $LINENO "uCount = ${uCount}"
	if [[ "$uCount" -ne "0" ]]
	then
		err $LINENO "$fontName.sfd : Multiple StartChars, test failed. =============="
	fi

	grep StartChar ${fontName}.j.sfd | sort |uniq -c | sort -n > ${fontName}.j.count
	uCount=`grep -c -v "^ *1" ${fontName}.j.count`
	dbg $LINENO "uCount = ${uCount}"
	if [[ "$uCount" -ne "0" ]]
	then
		err $LINENO "$fontName.j.sfd : Multiple StartChars, test failed. =============="
	fi

	echo ""
	grep "StartChar" ${fontName}.sfd |
			awk -F": " '{print $2}' > ${fontName}.lst 
	grep "StartChar" ${fontName}.j.sfd |
			awk -F": " '{print $2}' > ${fontName}.j.lst 
	log $LINENO "List of glyphs are listed in *.lst files"	

done < fonts.lst

rm fonts.lst
