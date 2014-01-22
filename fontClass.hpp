#ifndef __FONTCLASS_H
#define __FONTCLASS_H
using namespace std;
#include <string>
#include <vector>
#include <map>
//! \file fontClass.hpp
//! \brief Class declarations for glypRen

//! Return value for Success
#define SUCCESS 0

//! Return value for failure
#define FAIL 1

//! Starting value of Malayalam code point
#define ML_CODE_PT_START 3328

//! End value of Malayalam code point
#define ML_CODE_PT_END 3455

//! Number of reference Unicode characters
#define REF_UNICODE_CHARS 128

//! Chillu glyph
#define CHILLU "xx"

//! Chillu glyph name
#define CHILLU_NANE "chillu"

//! Zero Width Joiner glyph
#define ZWJ "ZWJ"

//! Search string for StartChar:
#define START_CHAR_TEXT "StartChar:"

//! Search string for Encoding.
#define ENCODING_TEXT "Encoding:"

//! Search string for Ligature2
#define LIGATURE_TEXT "Ligature2:"

//! Search string for EndChar
#define END_CHAR_TEXT "EndChar"

//! Store the ligature info of the glyphs.
class Ligature
{
public:
	//! set method for form
	void setForm (string form);

	//! get method for form
	string getForm (void);

	//! add glyph name to the list.
	void addGlypToList (string glyph);

	//! get the size of the glyph name list
	unsigned int getGlypListSize (void);

	//! get the nth glyph name from the glyphNames list.
	int getNthglyphName (unsigned int index, string& out);

	//! Display the contents
	void displayData ();

	//! Display the glyphs in a different format 
	void displayGlyphs ();

	//! Clear the list of glyph names.
	void clearGlypName (void);

	//! Return a reference to the Ligature object
	Ligature& returnLigature (void);
private:
	string form; //!< Form type - prebase, akhn etc.
	vector<string> glyphNames; //!< associated glyph names.
};

//! Store & manipulate the glyph information.
class FontChar
{
public:
	//! set method for startPos
	void setStartPos (int); 

	//! get method for startPos
	int getStartPos (void);

	//! Set method for curName
	void setCurName (string name);

	//! Get method for curName
	string getCurName (void);

	//! set method for unicodeVal
	void setUnicodeVal (int val);

	//! get method for unicodeVal
	int getUnicodeVal (void);

	//! Add ligature to the ligature vector
	void addLigature (Ligature l);

	//! Clear the data.
	void clearData (void);

	//! Display the data stored in the class.
	void displayData (void);

	//! Display the glyp data in a diffrent format
	void displayGlyphs (void);

	//! Set method for new name
	void setNewName (string name);

	//! Get method for new name
	string getNewName (void);

	//! Load the glyph name to the map.
	int loadMap (map<string, string>& nameMap);

	//! Get the ligature count
	int getLigatureCount (void);

	//! Get the reference to the nth Ligature
	Ligature& getLigature (unsigned int pos);
private:
	int startPos; //!< starting position of the char in SFD
	int endPos; //!< End position of the char in SFD
	int unicodeVal; //!< Code point of the character
	string curName; //!< Current name of the glyph
	string newName; //!< New name of the glyph

	vector<Ligature> ligatureList; //!< Ligature list.
};


//! Store the character reference data.
class CharRefData
{
public :
	int setCodeptVal (int); //!< set method for codePtVal
	int getCodeptVal (void); //!< get method for codePtVal

	int setCharName (string name); //!< set method for charName
	string getCharName (void); //!< get method for charName
private:
	int codePtVal; //!< Codepoint value of the character
	string charName; //!< Std(?) Name of the Unicode character
};

#endif 
