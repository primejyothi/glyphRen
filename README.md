glyphRen
========
Utility to rename glyphs in Unicode font SFD files.

For the Malayalam Unicode fonts, there is no standard conventions for naming a glyphs across different fonts. Since it is difficult for the developers to associate code point values and the characters, meaningful and human readable names are given to glyphs. However, these names are not standard across different fonts. If the names are same, it would be easier to apply different rules from one font to another. This utility is an attempt to rename the Malayalam glyphs in SFD files based on a standard naming convention.

The program reads a reference file that contain Malayalam Unicode code points and their names and use it to rename the glyphs in the SFD file. The composite characters are named based on the constituent glyphs from corresponding ligatures. In case multiple ligatures, the glyphs from akhn will be used. In case of a tie, the glyphs from the ligature having maximum number of glyphs are taken. In case the newly generated name is already taken by a character, the utility will attempt to create a unique name by adding sequence number to the newly created name.


#### Running glyphRen
glyphRen -r referenceFile -i inputSFDName -o outputSFDName

	-l : Log level (DBG or TRACE)
	-h : Display the help message

The reference file is a plain text file which contains the Unicode code point values in hex and the corresponding names. The fields are separated by spaces and records are separated by new lines. The reference file for a font can be generated from the font's SFD file using FontForge (Encoding->Save Namelist of Font).

Currently the reference file is generated from the Rachana font (http://wiki.smc.org.in/Fonts).

#### Testing glyphRen

The grTest.sh script can be used to run some automated tests quickly. This utility does not test the accuracy of rendering but compares the rendering before and after the conversion. grTest can be executed as follows:

grTest.sh -r ReferenceFile -t text.text

	-r : Reference file Unicode code points and corresponding names
	-t : Text file containing the text to render
	-d : Enable debug messages
	-h : Display help message

The script will launch glyphRen and rename the glyphs in the SFD files present in the current directory.

As the script generates a bunch of files during the tests, it would be a good idea to create a directory for running the tests.

#### Documentation

make docs (requires doxygen) will create documentation in docs folder.

#### Acknowledgement

aglfn.nam & glyphlist.nam provided by Crissov (https://github.com/Crissov)
