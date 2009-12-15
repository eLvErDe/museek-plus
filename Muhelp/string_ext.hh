/*****************************************************************

file:     string_ext.hh
version:  2003-09-30
author:   Steffen Brinkmann <subcom AT users.sourceforge.net>
license:  GPL
contents: inline int split(const string &src,
                           list<string> &dest,
                           const string delims=" \t\n")
          inline int split(const string &src,
                           vector<string> &dest,
                           const string delims=" \t\n")
          inline int split(const string &src,
                           vector<double> &dest,
                           const string delims=" \t\n")
          inline int split(const string &src,
                           vector<int> &dest,
                           const string delims=" \t\n")
          inline string toupper(const string& s)
          inline string tolower(const string& s)
          inline string itos(const int i)

comment:  The split funcions are inspired by the perl function of this name.
          They split a string (e.g. a file you just read) into a vector or list
          of either strings, doubles or ints and return the number of items.

          toupper(..) and tolower(..) implement the ctype-functions of the
          same name for strings.

          itos(..) replaces the function itoa() which is not implemented
          in all libraries.
          It uses a stringstream to convert a int to a string as
          implemented by Bjarne Stroustrup in his C++ Style and Technique FAQ
          <http://www.research.att.com/~bs/bs_faq2.html>

          All functions are inlined so you don't have to explicitely compile
          this file. Just include it into your application.
          Download the source from http://lsw.uni-heidelberg.de/users/sbrinkma/cpp/string_ext.hh
          Comments and suggestions are welcome!

Hyriand (2004-02-20): added str_replace

******************************************************************/

#ifndef STRING_EXT_HH
#define STRING_EXT_HH

#include <string>
#include <sstream>
#include <cctype>
#include <vector>
#include <list>
#include <cstdlib>

// Split a string into a list of strings
inline unsigned int split(const std::string &src,
		 std::list<std::string> &dest,
		 const std::string delims=" \t\n")
{
 	unsigned int n=0;
 	size_t i=0,j=0;

    while(j!=std::string::npos)
    {
	i=src.find_first_not_of(delims,j);
	if (i==std::string::npos) break;
	j=src.find_first_of(delims,i);
	dest.push_back(src.substr(i,j-i));
	n++;
    }
    return n;
}

// Split a string into a vector of strings
inline unsigned int split(const std::string &src,
		 std::vector<std::string> &dest,
		 const std::string delims=" \t\n")
{
 	unsigned int n=0;
 	size_t i=0,j=0;

    while(j!=std::string::npos)
    {
	i=src.find_first_not_of(delims,j);
	if (i==std::string::npos) break;
	j=src.find_first_of(delims,i);
	dest.push_back(src.substr(i,j-i));
	n++;
    }
    return n;
}

// Split a string into a vector of doubles
inline unsigned int split(const std::string &src,
		 std::vector<double> &dest, const
		 std::string delims=" \t\n")
{
 	unsigned int n=0;
 	size_t i=0,j=0;

    while(j!=std::string::npos)
    {
	i=src.find_first_not_of(delims,j);
	if (i==std::string::npos) break;
	j=src.find_first_of(delims,i);
	dest.push_back(atof(src.substr(i,j-i).data()));
	n++;
    }
    return n;
}

// Split a string into a vector of ints
inline unsigned int split(const std::string &src,
		 std::vector<int> &dest,
		 const std::string delims=" \t\n")
{
 	unsigned int n=0;
 	size_t i=0, j=0;

    while(j!=std::string::npos)
    {
	i=src.find_first_not_of(delims,j);
	if (i==std::string::npos) break;
	j=src.find_first_of(delims,i);
	dest.push_back(atoi(src.substr(i,j-i).data()));
	n++;
    }
    return n;
}

// Make an uppercase copy of s
inline std::string toupper(const std::string& s)
{
  std::string upper(s);
  for(size_t i=0; i<s.length(); i++)
    upper[i] = toupper(upper[i]);
  return upper;
}

// Make a lowercase copy of s
inline std::string tolower(const std::string& s)
{
  std::string lower(s);
  for(size_t i=0; i<s.length(); i++)
    lower[i] = tolower(lower[i]);
  return lower;
}

// Convert an int to a string
inline std::string itos(const int i)
{
    std::stringstream s;
    s << i;
    return s.str();
}

inline std::string str_replace(const std::string& s, char from, char to) {
	std::string r;
	std::string::const_iterator it = s.begin();
	for(; it != s.end(); ++it)
		if((*it) == from)
			r += to;
		else
			r += *it;
	return r;
}

inline std::wstring str_replace(const std::wstring& s, wchar_t from, wchar_t to) {
	std::wstring r;
	std::wstring::const_iterator it = s.begin();
	for(; it != s.end(); ++it)
		if((*it) == from)
			r += to;
		else
			r += *it;
	return r;
}

inline std::string str_replace(const std::string& s, const std::string& from, const std::string& to) {
	size_t findStartPos = 0, foundPos = 0;
	std::string out = s;

	foundPos = out.find(from, findStartPos);
    while ((findStartPos < out.length()) && (foundPos != std::string::npos)) {
        out = out.replace(foundPos, from.length(), to.c_str(), to.length());
        findStartPos = foundPos+to.length();
		foundPos = out.find(from, findStartPos);
    }

	return out;
}

#endif //STRING_EXT_HH
