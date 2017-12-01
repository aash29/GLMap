#include <string>

using namespace std;


void removeSubstrs(string &s,
                   const string &p) {
   string::size_type n = p.length();

   for (string::size_type i = s.find(p); i != string::npos; i = s.find(p))
      s.erase(i, n);
}

void replaceSubstrs(string &s,
		    const string &p, const string &q) {
   string::size_type n = p.length();

   for (string::size_type i = s.find(p); i != string::npos; i = s.find(p))
     s.replace(i,i+n,q);
}


