#include <iostream>
#include "fontClass.hpp"
#include "jlog.hpp"
//! \file fontClass.cc 
//! \brief fontClass implementation

// Ligature methods ////////////////////
//! set method for form
void Ligature::setForm (string inForm)
{
	form = inForm;
}

//! get method for form
string Ligature::getForm (void)
{
	return form;
}

//! add glyph name to the list
void Ligature::addGlypToList (string glyphName)
{
	glyphNames.push_back (glyphName);	
}

//! Get the size of the glyph name list.
unsigned int Ligature::getGlypListSize (void)
{
	return glyphNames.size ();
}

//! \fn int Ligature::getNthglyphName (unsigned int idx, string& out)
//! \brief Get the nth name from the glyphName list.
//! \param [in] idx index of the element starting from 0
//! \param [out] out The name of glyph
//! \returns SUCCESS if operation is successful
//! \returns FAIL if operation is failure or index is out of bound.
int Ligature::getNthglyphName (unsigned int idx, string& out)
{
	if (idx > this->getGlypListSize ())
	{
		jERR  ("getNthglyphName: Index out of bound");
		return FAIL;
	}

	out = glyphNames[idx];

	return SUCCESS;
}

//! Display the Ligature data
void Ligature::displayData ()
{
	jTRACE ("Form 		: " << getForm ());
	for (unsigned int i = 0; i < glyphNames.size (); i++)
	{
		jTRACE ("Glyphname	: " << glyphNames[i]);
	}
}

//! Display the Ligature data
void Ligature::displayGlyphs ()
{
	string t;
	for (unsigned int i = 0; i < glyphNames.size (); i++)
	{
		t.append (glyphNames[i]);
		t.append (" ");
	}
	jTRACE (t);
}

void Ligature::clearGlypName (void)
{
	glyphNames.clear ();
}


Ligature& Ligature::returnLigature (void)
{
	return (*this);	
}


// FontChar methods ////////////////////

//! set method for startPos
void FontChar::setStartPos (int pos)
{
	startPos = pos;
}

//! get method for startPos
int FontChar::getStartPos (void)
{
	return startPos;
}

//! Set method for curName
void FontChar::setCurName (string name)
{
	curName = name;
}

//! Get method for curName
string FontChar::getCurName (void)
{
	return curName; 
}

//! Set method for unicodeVal
void FontChar::setUnicodeVal (int val)
{
	unicodeVal = val;
}

//! Get method for unicodeVal
int FontChar::getUnicodeVal (void)
{
	return unicodeVal;
}

//! Add ligature to the ligature vector
void FontChar::addLigature (Ligature lg)
{
	this->ligatureList.push_back (lg);
} 

//! Method to clear the data
void FontChar::clearData (void)
{
	setStartPos (0);
	setCurName ("");
	setUnicodeVal (0);

	for (unsigned int i = 0; i < ligatureList.size (); i++)
	{
		ligatureList[i].clearGlypName ();
	}
	ligatureList.clear ();
}

//! Set method for new name
void FontChar::setNewName (string name)
{
	jTRACE ("Setting new name to [" << name << "]");
	newName = name;
}

//! Get method for new name
string FontChar::getNewName (void)
{
	return newName;
}

//! Display method for FontChars
void FontChar::displayData (void)
{
	jTRACE ("========== FontChar ==========");
	jTRACE ("Glyph Name	: " << getCurName ());
	// jTRACE ("New Name	: " << getNewName ());
	jTRACE ("Start Pos	: " << getStartPos ());
	jTRACE ("Unicodeval	: " << getUnicodeVal ());
	jTRACE ("Ligatures	: ");

	jTRACE ("Ligature #	: " << ligatureList.size ());
	for (unsigned int i = 0; i < ligatureList.size (); i++)
	{
		ligatureList[i].displayData ();
	}
	jTRACE ("==============================\n");;
}

//! Display glyph info in a different format
void FontChar::displayGlyphs (void)
{
	jTRACE (getCurName () << ": ");
	for (unsigned int i = 0; i < ligatureList.size (); i++)
	{
		ligatureList[i].displayGlyphs ();
	}
	jTRACE ("");
}
//! Load the glyph name to the map.
//! \fn int FontChar::loadMap (map<string, string>& nameMap)
//! \brief Load the glyphs from the FontChar to a map.
//! This map will be used to rename the ligatures. The old value of the
//! ligature will  be the key and the value would be the new name.
//! \param [out] nameMap  The map that contain old glyph
//! name and new name.
int FontChar::loadMap (map<string, string>& nameMap)
{
	for (unsigned int i = 0; i < ligatureList.size (); i++)
	{
		for (unsigned int j = 0; j < ligatureList[i].getGlypListSize(); j++)
		{
			string glyph;
			ligatureList[i].getNthglyphName(j, glyph);
			nameMap[glyph] = "";
		}
	}
	return SUCCESS;

}
//! Get the ligature count;
int FontChar::getLigatureCount (void)
{
	return ligatureList.size ();
}

//! \fn Ligature& FontChar::getLigature (unsigned int pos)
//! Get the reference of the Nth Ligature.
//! \param pos [in] Position of the Ligature in the vector.
//! \returns Reference to the requested Ligature.
//!	\returns An object dynamically allocated with Form set to "InvalidObject"
//! if pos is out of range. The caller has to check for the InvalidObject"
//! and free the dynamically allocated object.
//! \note Returns a dynamically allocated object if the pos is out of range.
//! The calling function has to check for this and free the dynamically
//! allocated object if applicable.
Ligature& FontChar::getLigature (unsigned int pos)
{
	if (pos >= ligatureList.size())
	{
		Ligature  *t = new Ligature();
		t->setForm ("InvalidObject");
		return (t->returnLigature ());
	}
	return (ligatureList[pos].returnLigature());
}

// CharRefData methods ////////////////////
//! Set method for codePtVal
int CharRefData::setCodeptVal (int val)
{
	codePtVal = val;
	return SUCCESS;
}

//! Get method for codePtVal
int CharRefData::getCodeptVal (void)
{
	return codePtVal;
}

//! Set method for charName
int CharRefData::setCharName (string name)
{
	charName = name;
	return SUCCESS;
}

//! Get method for charName
string CharRefData::getCharName (void)
{
	return (charName);
}

//! Display the CharRefData info
void CharRefData::displayData (void)
{
	jTRACE ("========== CharRefData ==========");
	jTRACE ("Char Name	: " << getCharName ());
	jTRACE ("CodePtVal	: " << getCodeptVal ());
}
