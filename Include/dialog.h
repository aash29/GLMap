#ifndef DIALOG_HPP
#define DIALOG_HPP

struct answer {
	int id;
	int address;
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

map <int, conversation> loadDialog(const char *name) {

	map <int, conversation> dialogs = map <int, conversation>();

	XMLDocument* doc = new XMLDocument();

	doc->LoadFile(name);
	XMLElement* c1 = doc->FirstChildElement("osm")->FirstChildElement("conversation");

	while (c1) {

		conversation conv1;

		conv1.replies = map <int, reply>();

		int id;
		c1->QueryAttribute("id", &id);
		conv1.id = id;

		XMLElement* r1 = c1->FirstChildElement("reply");
		while (r1) {

			reply rep1;

			int id;
			r1->QueryAttribute("id", &id);
			rep1.id = id;

			rep1.answers = map <int, answer>();

			XMLElement* t1 = r1->FirstChildElement("text");
			rep1.text = t1->GetText();

			XMLElement* a1 = r1->FirstChildElement("answer");

			while (a1) {
				answer ans1;

				int id;
				a1->QueryAttribute("id", &id);
				ans1.id = id;

				int address;
				a1->QueryAttribute("address", &address);
				ans1.address = address;

				ans1.text = a1->GetText();

				rep1.answers.insert(pair<int, answer>(id, ans1));

				a1 = a1->NextSiblingElement("answer");
			}

			conv1.replies.insert(pair<int, reply>(id, rep1));

			r1 = r1->NextSiblingElement("reply");
		}

		dialogs.insert(pair<int, conversation>(id, conv1));
		c1 = c1->NextSiblingElement("conversation");
	}
	return dialogs;
}

#endif