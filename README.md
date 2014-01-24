glyphRen
========
Utility to rename glyphs in Malayalam font SFD files.

For the Malayalam Unicode fonts, there is no standard conventions for naming a glyphs across different fonts. Since it is difficult for the developers to associate code point values and the characters, meaningful and human readable names are given to glyphs. However, these names are not standard across different fonts. If the names are same, it would be easier to apply different rules from one font to another. This utility is an attempt to rename the Malayalam glyphs in SFD files based on a standard naming convention.

The program reads a reference file that contain Malayalam Unicode code points and their names and use it to rename the glyphs in the SFD file. The composite characters are named based on the constituent glyphs from corresponding ligatures. In case multiple ligatures, the glyphs from akhn will be used. In case of a tie, the glyphs from the ligature having maximum number of glyphs are taken. In case the newly generated name is already taken by a character, the utility will attempt to create a unique name by adding more characters to the newly created name.


#### Running glyphRen
glyphRen -r referenceFile -i inputSFDName -o outputSFDName

	-l : Log level (DBG or TRACE)

	-h : Display the help screen

The reference file is a plain text file which contains the Unicode code point values in hex and the corresponding names. The fields are separated by spaces and records are separated by new lines. The reference file for a font can be generated from the font's SFD file using FontForge (Encoding->Save Namelist of Font).

Currently the reference file is generated from the Rachana font (http://wiki.smc.org.in/Fonts).
