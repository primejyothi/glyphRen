/*!
 * \mainpage glyphRen
 * \brief Rename the glyphs in a SFD file based on names from a reference file.
 * \author Prime Jyothi (primejyothi [at] gmail [dot] com)
 * \date 2014-01-01
 * \copyright This project is released under the GNU Public License.
 *
 * For the Malayalam Unicode fonts, there is no standard conventions for
 * naming a glyphs across different fonts. Since it is difficult for the
 * developers to associate code point values and the characters, meaningful
 * and human readable names are given to glyphs. However, these names are
 * not standard across different fonts. If the names are same, it would be
 * easier to apply different rules from one font to another. This utility
 * is an attempt to rename the Malayalam glyphs in SFD files based on a
 * standard naming convention.
 *
 * \par Acknowledgment 
 * Thanks to Rajeesh K Nambiar, who taught me the first glyphs of Malayalam fonts.
 *
 */

using namespace std;
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <getopt.h>
#include <limits.h>
#include <string.h>
#include "fontClass.hpp"
#include "jlog.hpp"

//! \file glyphRen.cc Rename glyphs in SFD file
//!	\brief Rename the glyphs in a SFD file based on a standard file.
//!
//! Usage : glyphRen -r referenceFile -i inputSFDName -o outputSFDName
//!		-l : Log level (DBG or TRACE)
//!		-h : Display the help screen
//!
//!	1. Read the code points and the standard values from the Reference file.
//!	2. Read all Unicode characters and the names into the list	
//!	3. Traverse through the list of characters and set the new names
//!		for the characters.
//!	4. Write the new SFD file with renamed glyphs
//!

// Performance considerations are thrown out of the window. 

int loadReferenceData (char *refFile, map<int, CharRefData>& ref);
int hexStrtoInt (string hexVal);
int analyzeSFDFile (char *inSfdFile, vector<FontChar>& vFontChar);
int getTok (string inStr, string& out, char delim, int pos);
int storeLigature (string sfdData, Ligature& sfdLigature);
int renameGlyphs (map<int, CharRefData> vRefData, vector <FontChar>& vFontChar, map<string, string>& nameMap, int& renCount);
void showMap (map<string, string> nameMap);
int buildName (map<string, string> nameMap, vector<string> comps, string& out);
int writeNewSFD (char *inSFDName, char *outFile, vector <FontChar>& vFontChar, map<string, string> nameMap);
int replaceFCName (map <string, string> nameMap, string& sfdData);
int replaceGlyphNames (map<string, string> nameMap, string& sfdData);
void help (char *progName);
int processArgs (int argc, char **argv, char *inFile, char *outFile, char *refFile, string& lvl);
int checkDups (vector<FontChar>& vFontChar, unsigned int idx, string newName);
int processHalfForms (string curName, string newName, string& hName);

string Virama;
string Zwj;

//! \fn int main (int argc, char **argv)
//! \brief Starting point of glyphRen.
int main (int argc, char **argv)
{

	char inFile[PATH_MAX];
	char outFile[PATH_MAX];
	char refFile[PATH_MAX];
	string logLvl;

	memset (inFile, '\0', PATH_MAX);	
	memset (outFile, '\0', PATH_MAX);	
	memset (refFile, '\0', PATH_MAX);	

	// Process the command line arguments.
	processArgs (argc, argv, inFile, outFile, refFile, logLvl);
	if (logLvl == "DBG")
	{
		SETMSGLVL (DBG);
	}
	else if (logLvl == "TRACE")
	{
		SETMSGLVL (TRACE);
	}
	else
	{
		SETMSGLVL (LOG);
	}
	SETFWDT (13);

	jTRACE ("inFile = " << inFile);

	//! Map that hold the ref data from the file.
	map<int, CharRefData> vRefData;
	
	//! Vector that hold the glyph data from the SFD file.
	vector<FontChar> vFontChar;

	int retVal;
	//! Load the reference data 
	retVal = loadReferenceData (refFile, vRefData);
	if (SUCCESS != retVal)
	{
		jERR ("Error : loadReferenceData failed");
		return (2);
	}

	// Print the data from the reference list
	jTRACE ("Data from the reference list");
	
	jTRACE ("vRefData.size () " << vRefData.size ());
	for (map <int, CharRefData>::iterator i = vRefData.begin ();
			i != vRefData.end(); ++i)
	{
		jTRACE ("vRefData[" << (*i).first << "] = ["
			<< (*i).second.getCharName() << "]");
	}

	//! Analyze the input SFD file and load the data into FontChar class.
	retVal = analyzeSFDFile (inFile, vFontChar);
	if (SUCCESS != retVal)
	{
		jERR ("Error : analyzeSFDFile failed");
		return (2);
	}

	map<string, string> nameMap;
	int renCount = 0;

	//! Load all glyphs into a map for convenience. This map will contain
	//! old name and new name. The "old name" will be the key.
	for (unsigned int i = 0; i < vFontChar.size(); i++)
	{
		vFontChar[i].loadMap (nameMap);
	}

	showMap (nameMap);

	int pass = 1;
	while (1)
	{
		jLOG ("renameGlyphs() : pass - " << pass);
		//! Traverse the glyph info and rename the glyphs
		retVal = renameGlyphs (vRefData, vFontChar, nameMap, renCount);
		if (SUCCESS != retVal)
		{
			jERR ("Error : renameGlyphs failed");
			return (2);
		}

		jLOG ("Number of glyphs renamed : " << renCount);
		if (renCount == 0)
		{
			break;
		}
		pass++;
	}
	
	jDBG ("Starting writeNewSFD ========================================");
	//! Read the SFD file again and write a new file with new glyph names.
	retVal = writeNewSFD (inFile, outFile, vFontChar, nameMap);
	if (SUCCESS != retVal)
	{
		jERR ("Error : renameGlyphs failed");
		return (2);
	}
	showMap (nameMap);
	
}

//! \fn int loadReferenceData (char *refFile, map<int, CharRefData>& ref)
//! \brief Load the reference data from the reference file
//! \param [in] refFile Name of the file containing reference data.
//! \param [out] ref The CharRefData map that will hold the ref data
//! \returns SUCCESS if operation is successful.
//! \returns FAIL if operation is not successful.
int loadReferenceData (char *refFile, map<int, CharRefData>& ref)
{
	//! Read the data from the reference file
	ifstream stdFile (refFile);
	if (!stdFile.is_open())
	{
		jERR ("Unable to read reference file " << refFile);
		return (FAIL);
	}
	
	jLOG ("Loading Reference data");

	string readLine;
	while (getline (stdFile, readLine))
	{
		CharRefData t;
		int codeValue;

		string tmpStr;
		// Extract the codepoint value which is the first token.
		if (getTok (readLine, tmpStr, ' ', 1) == SUCCESS)
		{
			// Convert hex to int and store.
			codeValue = hexStrtoInt (tmpStr);
			t.setCodeptVal (codeValue);
		}
		else
		{
			return (FAIL);
		}

		// Extract the glyph name which is the second token.
		if (getTok (readLine, tmpStr, ' ', 2) == SUCCESS)
		{
			// Set the char name
			t.setCharName (tmpStr);
		}
		else
		{
			return (FAIL);
		}

		ref[codeValue] = t;
		ref[codeValue].displayData ();
	}
	stdFile.close ();
	jLOG ("Finished Loading Reference data");
	return (SUCCESS);
}

//! \fn int hexStrtoInt (string)
//! \brief Convert a hex string to int
//! \param [in] hexVal string containing hex value
//! \returns int value of hex string
int hexStrtoInt (string hexVal)
{
	// Convert the hex string to int.
	int h;
	stringstream x;
	x << hex << hexVal;
	x >> h;
	return h;
}

//! \fn int analyzeSFDFile (char *inSfdName, vector<FontChar>& vFontChar)
//! \brief Analyze the input SFD file and load the data into FontChar vector.
//! \param [in] inSfdName Name of the input SFD file.
//! \param [out] vFontChar vector holding glyph data
//! \returns SUCCESS if operation is successful
//! \returns FAIL if operation is not successful
//
//! The glyphs are enclosed within [StartChar:] and [EndChar] sections. 
//! Read the SFD file and load the following into FontChar vector:
//! -# Name of the glyph as mentioned in the SFD file.
//! -# Start position(?) of the glyph
//! -# End position(?) of the glyph
//! -# Code point value of the glyph
//! -# Skip the glyph if it is not a Malayalam glyph
//
int analyzeSFDFile (char *inSfdName, vector<FontChar>& vFontChar)
{
	ifstream inSfdFile (inSfdName);
	if (! inSfdFile.is_open ())
	{
		jERR ("ERROR : Unable to open SFD file " << inSfdName);
		return FAIL;
	}

	string glyphName; // Name of the glyph from SFD file
	int dataFlag; // Indicate if the StartChar pattern is found
	int startPos;
	int codeValue;
	string sfdData;
	int retVal;

	FontChar sfdFC;
	jLOG ("Analyzing the SFD file");

	Ligature sfdLigature; // Ligatures read from input SFD file.
	vector<Ligature> vLigature;
	//! Read the data from the input SFD file.
	dataFlag = 0;
	int lineNo = 0;
	while (getline (inSfdFile, sfdData))
	{
		lineNo++;
		//! Look for [StartChar:]
		string srch (START_CHAR_TEXT);
		size_t found;
		found = sfdData.find (srch);
		if (found != string::npos)
		{
			 jTRACE ("Rec# " << lineNo <<  " Processing ["
			 		<< sfdData << "]");
			dataFlag = 1;
			//! [StartChar:] found, extract the glyph name which is
			//! the 2nd token.
			if (getTok (sfdData, glyphName, ' ', 2) != SUCCESS)
			{
				return FAIL;
			}
		}

		//! Look for [Encoding:] 
		srch = ENCODING_TEXT;
		found = sfdData.find (srch);
		if (found != string::npos)
		{
			jTRACE (setw(5) << "Rec# " <<lineNo <<  " Processing ["
					<< sfdData << "]");
			//! Check if StartChar is already found, if not skip.
			if (0 == dataFlag)
			{
				jTRACE (setw (5) << "Rec# " << lineNo <<  " Skipping [" 
					<< sfdData << "]");
				continue;
			}
			//! [Encoding:] found, extract start, code point value, and end.
			string tmpStr; 
			 if (getTok (sfdData, tmpStr, ' ', 2) != SUCCESS)
			 {
			 	return FAIL;
			 }
			 else
			 {
			 	startPos = atoi (tmpStr.c_str ());
			 }

			 // Code point value at position 3
			 if (getTok (sfdData, tmpStr, ' ', 3) != SUCCESS)
			 {
			 	return FAIL;
			 }
			 else
			 {
			 	if (tmpStr == "-1")
				{
					codeValue = -1;	
				}
				else
				{
					codeValue = atoi (tmpStr.c_str());
				}
			 }
		}
		
		//! Look for Ligature
		srch = LIGATURE_TEXT;
		found = sfdData.find (srch);
		if (found != string::npos)
		{
			 jTRACE ("Rec# " << lineNo <<  " Processing ["
				<< sfdData << "]");
			//! Split the data and store in Ligature class
			retVal = storeLigature (sfdData, sfdLigature);
			if (retVal != SUCCESS)
			{
				jERR ("Error : storeLigature [" << sfdData << "]");
				continue;
			}
			jTRACE ("Rec# " << lineNo <<  " Storing Ligatures");
			// sfdLigature.displayData ();

			//! Add the ligature to the temp list.
			vLigature.push_back (sfdLigature);

			// Clear the sfdLigature data.
			sfdLigature.clearGlypName ();
			sfdLigature.setForm ("");
		}

		//! Look for EndChar
		srch = END_CHAR_TEXT;
		found = sfdData.find (srch);
		if (found != string::npos)
		{
			jTRACE (setw(5) << "Rec# " << lineNo <<  " Processing ["
				<< sfdData << "]");
			
			//! Save the glyph name into FontChar vector.
			sfdFC.setCurName (glyphName);

			//! Save the start pos into FontChar vector
			sfdFC.setStartPos (startPos);

			//! Save the code value into the FontChar vector
			sfdFC.setUnicodeVal (codeValue);

			//! Save the ligatures to FontChar vector from the 
			//! temporary Ligature list.
			for (unsigned int i = 0; i < vLigature.size (); i++)
			{
				sfdFC.addLigature (vLigature[i]);
			}
			vFontChar.push_back (sfdFC);
			jTRACE (setw(5) << "Rec# " << lineNo << " Added glyph info for " <<
				glyphName << "]");

			sfdFC.displayData ();

			vLigature.clear();
			sfdFC.clearData ();
		}
	}
	jLOG ("Finished analyzing the SFD file");
	inSfdFile.close ();
	return SUCCESS;
}

//! \fn int getTok (string inStr, string& out, char delim, int pos)
//! \brief Copy the token at the pos from the string inStr delimited by
//! delim to the string out.
int getTok (string inStr, string& out, char delim, int pos)
{
	string tok;
	stringstream s (inStr);

	for (int i = 0; i < pos; i++)
	{
		if (!getline (s, tok, delim))
		{
			return FAIL;
		}
	}
	if (inStr == tok)
	{
		// The delimiter was not present in the input string.	
		return FAIL;
	}
	else
	{
		out = tok;
	}

	return (SUCCESS);
}
//! \fn int storeLigature (string sfdData, Ligature& sfdLigature)
//! \brief Populate the Ligature class from the Ligature Line read from
//! the input SFD file
//! \param [in] sfdData Ligature line from the input SFD file.
//! \param [in] sfdLigature Ligature class to which the data will be loaded.
//! \returns SUCCESS if data is stored successfully.
//! \returns FAIL if operation fails.
int storeLigature (string sfdData, Ligature& sfdLigature)
{
	//! Extract the form from the SFD line. Form will be enclosed in single
	//! quotes. The form will be second token when the delimiter is
	//! single quote.
	
	jTRACE ("Store Ligature");
	string tmpStr;
	int retVal;
	retVal = getTok (sfdData, tmpStr, '\'', 2);
	if (retVal != SUCCESS)
	{
		return FAIL;
	}
	else
	{
		sfdLigature.setForm (tmpStr);
	}
	//! Extract the names of the glyphs from the end. The glyphs will be 
	//! the third token if the delimiter is set to double quotes.
	retVal = getTok (sfdData, tmpStr, '"', 3);
	if (retVal != SUCCESS)
	{
		return FAIL;
	}

	int i;
	i = 1;
	string glyphName;
	while (1)
	{
		retVal = getTok (tmpStr, glyphName, ' ', i);
		i++;
		if (retVal != SUCCESS)
		{
			//! getTok will return FAIL when it encounter the end of the
			//! string. At this moment, it cannot be determined whether
			//! getTok encountered an error or end of the string.
			return SUCCESS;
		}
		else
		{
			if (glyphName.length () == 0)
			{
				// Ignore the spaces.
				continue;
			}
			sfdLigature.addGlypToList (glyphName);
		}
	}

	return SUCCESS;
}

//! \fn int renameGlyphs (map<int, CharRefData> vRefData, vector <FontChar>& vFontChar, map<string, string>& nameMap, int& renCount)
//! \brief Traverse through the glyph info and identify the glyphs
//! that need to be renamed.
//!	\param [in] vRefData Map containing reference data
//! \param [in] vFontChar Vector holding SFD glyph data
//! \param [in] nameMap map holding key value pair of old and new glyph names.
//! \param [out] renCount Number of renames performed
//! \returns SUCCESS if operation is successful
//! \returns FAIL if operation is not successful
//!
//! Rules of the game:
//!
//! -# Only Malayalam glyphs (simple & composite) will be renamed based on
//! the reference data provided.
//! -# Glyphs between 0D00 and 0D7F will be renamed as mentioned in the
//! reference file.
//! -# Composite glyphs will be renamed based on the constituent ligatures.
//! The name of the constituent glyphs will be combined to form the new name
//! of the composite glyph.
//! -# If there are multiple ligatures for a composite glyph, the one
//! with akhn will be used.
//! -# In case of a tie, the ligature with maximum glyphs will be used
//! for the creation of the new name
//! -# When two or more glyphs are joined to form new glyph name, the Virama
//! symbols are ignored to keep the name short and readable.
//! -# If the derived new name is already used in the SFD file, a 'j'
//! will be appended to the new name to avoid conflicts.
int renameGlyphs (map<int, CharRefData> vRefData,
	vector <FontChar>& vFontChar, map<string, string>& nameMap, int& renCount)
{

	unsigned int i;

	string fcName;
	string refName;

	//! Rename the characters that fall between ML_CODE_PT_START 
	//! and ML_CODE_PT_END. Since the Map and reference data are
	//! not directly connected, have to use the data loaded from
	//! the SFD file.

	// renCount might contain value from previous run, set it to 0,
	renCount = 0;
	jLOG ("renameGlyphs() : Renaming the Glyphs");
	
	jLOG ("renameGlyphs() : Processing base characters");
	for (i = 0; i < vFontChar.size (); i++)
	{
		int fcUniVal;
		fcUniVal = vFontChar[i].getUnicodeVal();
		fcName = vFontChar[i].getCurName();
		if (fcUniVal == -1)
		{
			//! Ignore composite characters while renaming base characters.
			continue;
		}

		refName = vRefData[fcUniVal].getCharName ();

		// Name of character from SFD file and corresponding name from
		// ref file.
		nameMap[fcName] = refName;

		// Set the new name in the FontChar.
		vFontChar[i].setNewName (refName);
		jLOG ("Base char " << "old [" << fcName <<
					"] new [" << refName << "]");
	}

	for (map <string, string>::iterator i = nameMap.begin ();
			i != nameMap.end(); ++i)
	{
		if ((*i).second == VIRAMA)
		{
			Virama = (*i).first;
			jTRACE ("Virama [" << Virama << "]");
		}

		if ((*i).second == ZWJ)
		{
			Zwj = (*i).first;
			jTRACE ("Zwj [" << Zwj << "]");
		}
	}

	jLOG ("renameGlyphs() : Finished processing base characters");
	showMap (nameMap);

	//! Traverse through the glyphs in the FontChar vector and see if 
	//! any glyphs can be renamed with the available data in Rename map.
	
	jLOG ("renameGlyphs() : Processing the glyphs");
	for (unsigned int i = 0; i < vFontChar.size (); i++)
	{
		string tCurName = vFontChar[i].getCurName ();
		string tNewName = nameMap[tCurName];
		if (tNewName.length() != 0)
		{
			jTRACE ("New name found for " << tCurName << ": "
					<< tNewName);
			// Set the new name in the FontChar.
			vFontChar[i].setNewName (tNewName);
		}
	}
	jLOG ("renameGlyphs() : Finished processing the glyphs");

	jLOG ("renameGlyphs() : Processing the Ligatures");

	//! Traverse through the glyphs of the Ligatures in the FontChar vector
	//! and see if any of FontChar glyphs can be renamed.
	for (unsigned int i = 0; i < vFontChar.size (); i++)
	{
		string curName;
		string newName;
		vector<string> nameComps;
		vector<string> akhnComps;
		vector<string> maxComps;
		vector<string> finalComps;

		int maxCount; // Maximum glyphs in a ligature
		int copyToMax;
		int akhnFlag; // Indicate if akhn form is found

		int renFlag; // Indicate if the glyph can be renamed.

		curName = vFontChar[i].getCurName ();
		int LigatureCount = vFontChar[i].getLigatureCount ();
		maxCount = 0;
		copyToMax = 0;
		akhnFlag = 0;
		renFlag = 0;

		// Clear the vectors holding the glyph names.
		nameComps.clear ();
		akhnComps.clear ();
		maxComps.clear ();
		newName.clear ();

		jTRACE ("\n");
		jTRACE ("renameGlyphs() : Processing Ligature : " << curName); 

		if (LigatureCount == 0)
		{
			// No ligatures, skip.
			continue;
		}

		// Check in the nameMap to see if it is already renamed.
		newName = nameMap[curName];
		if (newName.length() > 0)
		{
			// New name available, skip this.
			jTRACE ("[" << curName << "] already renamed to [" << newName << "]");
			continue;
		}

		for (int l = 0; l < LigatureCount; l++)
		{
			Ligature& tLig  = vFontChar[i].getLigature (l);
			// If the index out of range, getLigature will return a object
			// with form set to InvalidObject. If that is the case free the
			// object and return FAIL.
			if (curName == "InvalidObject")
			{
				jERR ("getLigature at " << l << " failed.");
				delete &tLig;
				return (FAIL);
			}

			string tForm;
			tForm = tLig.getForm ();
			jTRACE ("Processing form [" << tForm << "]");

			// Check if all the glyphs are renamed.
			unsigned int glyphCount = tLig.getGlypListSize ();
			int newCount;
			newCount = 0;

			if ((int) glyphCount > maxCount)
			{
				maxCount = glyphCount;
				// This Ligature has got max glyphs so far, set copyToMax flag
				// so that the glyphs will be copied to maxComps.
				copyToMax = 1;

				// maxComps may contain data from the previous Ligature,
				// clear it.
				maxComps.clear ();
			}
			else
			{
				copyToMax = 0;
			}

			if ("akhn" == tForm)
			{
				jTRACE ("Setting akhn flag ");
				akhnFlag = 1;
				akhnComps.clear ();
			}
			else
			{
				akhnFlag = 0;
			}

			// Clear the data from the previous ligature.
			nameComps.clear ();

			for (unsigned int k = 0; k < glyphCount; k++)
			{
				string tGlyphName;
				string tNewName;
				tLig.getNthglyphName (k, tGlyphName);
				jDBG (k << " : " << tGlyphName);
				nameComps.push_back (tGlyphName);

				if (copyToMax)
				{
					// Max # of glyphs, save it for later use.
					maxComps.push_back (tGlyphName);
				}
				jTRACE ("tForm: " << tForm);
				if (akhnFlag)
				{
					jTRACE ("Adding [" << tGlyphName << "] to akhnComps");
					akhnComps.push_back (tGlyphName);
				}

				// Check if the glyph name has got new name in the map.
				tNewName = nameMap[tGlyphName];
				if (tNewName.length () != 0)
				{
					newCount++;
				}
				jTRACE ("[" << tGlyphName << "] [" << tNewName << "]");
			}

			if (newCount == (int) glyphCount)
			{
				// All the constituent glyphs have new names, can be renamed.
				jDBG (curName << " Can be renamed");
				renFlag++;
			}
			else
			{
				jDBG (curName << " Cannot be renamed");
				continue;
			}
		}


		int dupRet;
		if ((LigatureCount == 1) && (renFlag == LigatureCount))
		{
			// Only one form, straight away rename.
			jDBG ("Straight rename");
			finalComps = nameComps;
		}
		else
		{
			if (renFlag != LigatureCount)
			{
				// At least one of the ligature had a glyph without new name.
				// Skip the renaming
				jDBG ("RenFlag check failed for " << curName);
				continue;
			}

			jDBG ("Multiple ligatures, further processing required");
			if (akhnFlag)
			{
				jDBG ("Multiple ligatures, akhn form being added");
				finalComps = akhnComps;
			}else
			{
				jDBG ("Multiple ligatures, max being added");
				finalComps = maxComps;
			}
		}

		string suffix;
		int seq;
		string base;
		seq = 0;
		buildName (nameMap, finalComps, newName);
		base = newName; // Base name, required in case of duplicates.
		base.append ("_");
		do
		{
			jTRACE ("Calling checkDups");
			jTRACE ("Current glyph is [" << curName << "]");
			dupRet = checkDups (vFontChar, i, newName);
			if (dupRet == FAIL)
			{
				// Special processing required for some half forms.
				int retVal;

				string hName;
				retVal = processHalfForms (curName, newName, hName);
				if (retVal == SUCCESS)
				{
					// It was one of those cases that required special processing.
					newName = hName;

					dupRet = SUCCESS; // Break out of the checkDups loop.
				}
				else
				{
					jLOG ("[" << newName
						<< "] already taken, appending j");
					seq ++;
					stringstream ss;
					ss << seq;
					suffix = ss.str();

					jTRACE ("Suffix [" << suffix << "]");
					newName = base;
					newName.append (suffix);
				}
			}
		} while (dupRet == FAIL);
		jDBG ("Adding [" << curName << "] and [" << newName <<
				"]to the map");
		nameMap[curName] = newName;
		
		// Set the new name.
		vFontChar[i].setNewName (newName);
		renCount++;
	}
	jLOG ("renameGlyphs() : Finished processing the Ligatures");
	showMap (nameMap);

	/*
	for (i = 0; i < vFontChar.size (); i++)
	{
		vFontChar[i].displayData ();
		vFontChar[i].displayGlyphs ();
	}
	*/

	return SUCCESS;
}


//! \fn void showMap (map<string, string> nameMap)
//! \brief Display the contents of the Rename map
void showMap (map<string, string> nameMap)
{
	jTRACE ("showMap ()");
	for (map <string, string>::iterator i = nameMap.begin ();
			i != nameMap.end(); ++i)
	{
		jTRACE ((*i).first << ":" << (*i).second);
	}
}

//! \fn int buildName (map<string, string> nameMap, vector<string> comps, string& out)
//! \brief Build the new name for a glyph.
//! The new names of the strings are looked up against the Rename map and
//! creates new name.
//! \param [in] nameMap The rename map from which the new names will be
//! looked up
//! \param [in] comps Vector containing the glyphs to be renamed.
//! \param [out] out The string that will hold the new name.
//!
//! -# If the strings are glyph + xx + zwj, it is considered as a chillu
//! and new new name will be glyph + "chillu"
//! -# If the strings are glyph + xx, the new name would be glyph+xx
int buildName (map<string, string> nameMap, vector<string> comps, string& out)
{
	unsigned int i;
	int zFlag;
	int cFlag;
	string newName;
	string mappedName; // Currently mapped name for the glyph

	cFlag = 0;
	zFlag = 0;
	newName = "";

	for (i = 0; i < comps.size (); i++)
	{
		jTRACE ("i = " << i << " " << comps[i]);
	}

	for (i = 0; i < comps.size(); i++)
	{
		jDBG ("Finding new name for " << comps[i]);
		// Check for Chillu & ZWJ
		// if (comps[i] == ZWJ) 
		if ((comps[i] == ZWJ) || (comps[i] == Zwj))
		{
			zFlag++;

			jTRACE ("Found ZWJ case");
			//! Check if this is a chillu - glyph + xx + zwj. If true, rename
			//! glyph to glyph + chil

			if ( (i == 2) && (cFlag == 1) && (comps.size() == 3))
			{
				jDBG ("Found chillu comibination for " << comps[0]);
				// out = comps[0];
				out.append (CHILLU_NANE);
			}
			continue;
		}

		/*
		jTRACE ("Virama [" << Virama << "]");
		if ((comps[i] == "VIRAMA") || (comps[i] == Virama))
		{
			//! If there are only two glyphs and the 2nd one is xx, retain it.
			jTRACE ("Found Virama case");
			cFlag++;

			jTRACE ("out, before appening virama [" << out << "]");
			if ( (i == 1) && (comps.size() == 2))
			{
				// out = comps[0];
				out.append (VIRAMA);
			}
			jTRACE ("Appended Virama, now the name is [" << out);
			continue;
		}
		*/

		if ((comps[i] == "VIRAMA") || (comps[i] == Virama))
		{
			// Skip Virama.
			cFlag++;
			jTRACE ("Skipping Virama");
			continue;
		}

		mappedName = nameMap[(comps[i])];
		if (mappedName.length() != 0)
		{
			if (mappedName == VIRAMA)
			{
				// Virama, skip it.
				continue;
			}

			jDBG ("Named map [" << comps[i]
					<< "] [" << mappedName << "]");
			newName.append (mappedName);

			jDBG ("New Name : [" << newName << "]");
			out = newName;
		}
		else
		{
			// Not found in the remap, use the old one itself.
			newName.append (comps[i]);
			jDBG ("New Name : [" << newName << "]");
			out = newName;
		}
	}

	return (SUCCESS);
}


//! \fn int writeNewSFD (char *inSfdName, char *outFname, vector <FontChar>& vFontChar, map<string, string> nameMap)
//! \brief Read the input SFD file and create new SFD file with new glyph names.
//!
//! Read the input SFD file and and rename the glyphs using the look up table.
//! \param [in] inSfdName Name of the input SFD file.
//! \param [in] outFname Name of the output SFD file.
//! \param [in] vFontChar FontChar vector
//! \param [in] nameMap The lookup table for new glyph names.
int writeNewSFD (char *inSfdName, char *outFname, vector <FontChar>& vFontChar, map<string, string> nameMap)
{
	string sfdData; // Data read from the input SFD file.
	string srchPattern; // Search pattern

	ifstream inSfdFile (inSfdName);
	size_t strPos;

	jLOG ("Writing new SFD file");

	if (! inSfdFile.is_open ())
	{
		jERR ("Error : Unable to open SFD file " << inSfdName);
		return FAIL;
	}

	ofstream outFile (outFname);
	if (! outFile.is_open ())
	{
		jERR ("Uanble to open output file " <<  outFname);
	}

	while (getline (inSfdFile, sfdData))
	{
		srchPattern = START_CHAR_TEXT;
		strPos = sfdData.find (srchPattern);

		if (strPos != string::npos)
		{
			replaceFCName (nameMap, sfdData);
		}

		srchPattern = LIGATURE_TEXT;
		strPos = sfdData.find (srchPattern);
		if (strPos != string::npos)
		{
			replaceGlyphNames (nameMap, sfdData);
		}
		outFile << sfdData << "\n";
	}
	jLOG ("Finished Writing new SFD file");

	return SUCCESS;
}

//! \fn int replaceFCName (map <string, string> nameMap, string& sfdData)
//! \brief Replace the name of the StartChar.
//! Replace the glyph name in the StartChar section using the look up data
//! from the map. If the new name is not found, keep the old one.
//! \param [in] nameMap Look up data for renaming
//! \param [out] sfdData Line from SFD file
int replaceFCName (map <string, string> nameMap, string& sfdData)
{
	string glyphName;
	string newName; // New name of the glyph
	size_t strPos;

	if (getTok (sfdData, glyphName, ' ', 2) == SUCCESS)
	{
		jTRACE ("Found start char [" << sfdData << "], glyph name ["
			<< glyphName << "]");

		// Check if the new name for glyph name is available
		// in the Rename map.
		newName = nameMap[glyphName];
		if ((newName.length () != 0 ) && (newName != glyphName))
		{
			// Proceed only if the new name is different
			jTRACE ("Found new name for [" << glyphName <<
				"] = [" << newName << "]");
			strPos = sfdData.find (glyphName);
			if (strPos != string::npos)
			{
				jTRACE ("Glyph " <<  glyphName << " is at " << strPos);
				sfdData.replace (strPos, glyphName.length (), newName);
				jTRACE ("Replaced [" << sfdData << "]");
			}
		}
	}
	return SUCCESS;
}

//! \fn int replaceGlyphNames (map<string, string> nameMap, string& sfdData)
//! \brief Replaces the glyph names in the Ligature line with the new names.
//! \param [in] nameMap Look up data for renaming
//! \param [out] sfdData Line from SFD file
//!
//! Replace the glyph names in the Ligature line based on the data from the
//! look up table. The glyphs are renamed individually to prevent any
//! incorrect partial renames.
int replaceGlyphNames (map<string, string> nameMap, string& sfdData)
{
	string glyphNames;
	string oldGlyphNames;
	size_t gPos;
	size_t nextPos;
	jTRACE ("\n");

	if (getTok (sfdData, glyphNames, '"', 3) == SUCCESS)
	{
		jTRACE ("Found glyphs [" << glyphNames << "]");
		oldGlyphNames = glyphNames;
	}
	else
	{
		return FAIL;
	}

	map<string, string> glyphComps;

	string t;
	int i = 1;
	//! Store the glyph names in the map. The map is used so that only
	//! unique values will be stored.
	while (getTok (glyphNames, t, ' ', i) == SUCCESS)
	{
		if (t.length() != 0)
		{
			glyphComps[t] = t;
		}
		i++;
	}

	for (map<string, string>::iterator m = glyphComps.begin(); m != glyphComps.end(); ++m)
	{
		//! Look for the glyphs in the input string.
		
		gPos = 0;
		nextPos = 0;
		string oldName = (*m).first;
		string newName;
		while (1)
		{
			// Look for the glyph name in the string.
			gPos = glyphNames.find (oldName, nextPos);
			if (gPos != string::npos)
			{
				// Found the glyph in the string, replace it.
				//! Set the start point for the next search just beyond the
				//! end of the glyph.
				nextPos = gPos + oldName.length();
				newName = nameMap[oldName];
				if (newName.length() == 0)
				{
					//! If glyph does not have new name, skip it.
					// newName = oldName;
					continue;
				}

				//! Make sure that the match is not a partial match (k1 is
				//! matched with k1 and not with xxk1 or k1xx).
			
				char prev;
				char next;
				// If the previous char is not space, it is a partial match.
				if (gPos != 0)
				{
					// Not at the beginning of the string.
					prev = glyphNames.at(gPos - 1);	
					if (prev != ' ')
					{
						// Partial match
						continue;
					}

					if (gPos + oldName.length () < glyphNames.length ())
					{
						next = glyphNames.at(gPos + oldName.length());
						if (next != ' ')
						{
							// Partial match
							continue;
						}
					}
				}
			
				glyphNames.replace (gPos, oldName.length (), newName);
			}
			else
			{
				break;
			}
		}
	}
	//! Replace the glyphs part in the input string with the new names.
	jTRACE ("sfd [" << sfdData << "] glyphNames [" << oldGlyphNames << "]");
	gPos = sfdData.find (oldGlyphNames, 0);
	if (gPos != string::npos)
	{
		sfdData.replace (gPos, oldGlyphNames.length (), glyphNames);
	}
	
	jTRACE ("New [" << sfdData << "]");

	return SUCCESS;
}

//! \fn void help (char *progName)
//! \brief Display the help text.
void help (char *progName)
{
	cout << "Usage : " << progName <<
		" -r referenceFile -i inputSFDName -o outputSFDName" << endl;
	cout << "\t -r Reference File" << endl;
	cout << "\t -i Input SFD File" << endl;
	cout << "\t -o Output SFD File" << endl;
	cout << "\t [-l DBG | TRACE ] " << endl;
	cout << "\t -h Display this help message" << endl;

}

//! \fn int processArgs (int argc, char **argv, char *inFile, char *outFile, char *refFile, string& lvl)
//! \brief Process and validate the input arguments and parameters.
//! Process and validate the input arguments and parameters. The program
//! expects three mandatory parameters - -i, -o and -r.
//! \param [in] argc argc from main().
//! \param [in] argv argv from main().
//! \param [out] inFile Name of the input SFD file.
//! \param [out] outFile Name of the output SFD file.
//! \param [in] refFile Name of the Reference file.
//! \param [out] lvl The log level (DBG, TRACE).
int processArgs (int argc, char **argv, char *inFile, char *outFile, char *refFile, string& lvl)
{
	static struct option glyphOptions[] = 
	{
		{"insfd",		required_argument,	0, 'i'},
		{"outsfd",		required_argument,	0, 'o'},
		{"refnam",		required_argument,	0, 'r'},
		{"log",			required_argument,	0, 'l'},
		{"help",		no_argument, 		0, 'h'},
		{0,				0,					0, 0}
	};

	int helpFlag = 0;
	int c = 0;
	int optIdx = 0;

	while (1)
	{
		c = getopt_long (argc, argv, "i:o:r:l:h", glyphOptions, &optIdx);
		jDBG ("optIdx " << optIdx);
		if ( -1 == c )
		{
			break;
		}

		switch (c)
		{
			case 'h' :
				helpFlag = 1;
				jDBG ("Help option found");
				break;
			case 'i' :
				jDBG ("i: name " << glyphOptions[optIdx].name
						<<" optarg "<< optarg);
				strcpy (inFile, optarg);
				// jDBG ("inFile " << inFile);
				break;
			case 'o' :
				jDBG ("o: name " << glyphOptions[optIdx].name
						<<" optarg "<< optarg);
				strcpy (outFile, optarg);
				break;
			case 'r' :
				jDBG ("r: name " << glyphOptions[optIdx].name
						<<" optarg "<< optarg);
				strcpy (refFile, optarg);
				break;
			case 'l' :
				jDBG ("l: name " << glyphOptions[optIdx].name
						<<" optarg "<< optarg);
				lvl = optarg;
				jDBG ("Log level " << lvl);
				break;
			case '?' :
				jDBG ("Try " << argv[0] << " --help for more information");
				exit (2);
				break;
		}
		if (helpFlag)
		{
			help (argv[0]);
			exit (1);
		}

	}

	if (strlen (inFile) == 0)
	{
		jERR ("Input SFD file not specified, try " << argv[0] << " -h");
		exit (1);
	}

	if (strlen (outFile) == 0)
	{
		jERR ("Output SFD file not specified, try " << argv[0] << " -h");
		exit (1);
	}

	if (strlen (refFile) == 0)
	{
		jERR ("Reference file not specified, try " << argv[0] << " -h");
		exit (1);
	}
	return SUCCESS;
}

//! \fn int checkDups (vector<FontChar>& vFontChar, unsigned int idx, string newName)
//! \brief Check if the new name is already taken
//! \param [in] vFontChar FontChar vector
//! \param [in] idx Index of the glyph in FontChar vector.
//! \param [in] newName The new name for the glyph.
//! \returns FAIL if the name is already in use.
int checkDups (vector<FontChar>& vFontChar, unsigned int idx, string newName)
{

	string fcCurName;
	string fcNewName;
	jTRACE ("Checking for existing name [" << newName << "]");
	
	//! Search through the FontChar vector.
	for (unsigned int i = 0; i < vFontChar.size (); i++)
	{
		if (i == idx)
		{
			// This is the search key, skip it.
			jDBG ("Skipping " << i);
			continue;
		}

		//! Check both current and new name of the glyphs.
		fcCurName = vFontChar[i].getCurName();
		fcNewName = vFontChar[i].getNewName();
		if (fcCurName == newName )
		{
			//! If found being used, return fail.
			jDBG ("Name already exists [" << fcCurName << "] at " << i);
			return FAIL;
		}

		if (fcNewName == newName )
		{
			jDBG ("Name already exists [" << fcNewName << "] at " << i);
			return FAIL;
		}
	}
	
	//! The new name is not being used, return SUCCESS.
	jTRACE ("checkDups returning SUCCESS");
	return SUCCESS;
}

//! \fn int processHalfForms (string curName, string newName, string& hName)
//! \brief Alternate naming method for some special glyphs
//! \param [in] curName Current glyph name.
//!	\param [in] newName New name built for the glyph.
//! \param [out] hName The final new name.
//! \returns SUCCESS if alternate name is found.
//! \returns FAIL if alternate name is not found.

int processHalfForms (string curName, string newName, string& hName)
{
	map<string, string> specials;

	jTRACE ("processHalfForms [" << curName << "] [" << newName <<"]");

	specials["y1"] = "y2";
	specials["y1xx"] = "y2";
	specials["r3"] = "r4";
	specials["r3xx"] = "r4";
	specials["l3"] = "l4";
	specials["l3xx"] = "l4";
	specials["v1"] = "v2";
	specials["v1xx"] = "v2";

	if (specials[newName] == curName)
	{
		hName = curName;
		jTRACE ("Setting special to [" << hName << "]");
		return SUCCESS;
	}

	return FAIL;
}
