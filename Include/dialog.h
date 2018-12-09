#ifndef DIALOG_HPP
#define DIALOG_HPP

using namespace std;

#include "tinyxml2.h"
using namespace tinyxml2;

struct answer {
	int id;
	string text;
};

struct reply {
	int id;
	string text;
	map <int, answer> answers;
};

struct conversation {
	int id;
	map <int, reply> replies;
};

void loadDialog(const char *name, std::map <unsigned int, entity> &things) {
	XMLDocument* doc = new XMLDocument();

	doc->LoadFile(name);
	XMLElement* c1 = doc->FirstChildElement("osm")->FirstChildElement("conversation");

	while (c1) {

		conversation conv1;

		int id;
		c1->QueryAttribute("id", &id);
		conv1.id = id;

		XMLElement* r1 = c1->FirstChildElement("reply");

		XMLElement* t1 = r1->FirstChildElement("text");
		t1->


		n1 = n1->NextSiblingElement("conversation");
	}

}

#endif