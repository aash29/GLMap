#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <string>

using namespace std;
struct utils {
static void removeSubstrs(string &s,
                   const string &p) {
   string::size_type n = p.length();

   for (string::size_type i = s.find(p); i != string::npos; i = s.find(p))
      s.erase(i, n);
}

static void replaceSubstrs(string &s,
		    const string &p, const string &q) {
   string::size_type n = p.length();

   for (string::size_type i = s.find(p); i != string::npos; i = s.find(p))
     s.replace(i,i+n,q);
}

static std::vector<std::string> tokenize (std::string s1, char sep)
{
  std::string curStr;
  std::vector<std::string> tokens;
  for (int i = 0; i < s1.length(); i++) {
    char c = s1[i];
    if (c != sep)
      {
	curStr += c;
      }
    else
      {
	if (curStr!="")
	  tokens.push_back(curStr);
	curStr = "";
      }
  }
  tokens.push_back(curStr);
  
  return tokens;
}
};

#endif