#ifndef MAP_HPP
#define MAP_HPP

struct building
{
  std::string id;
  std::string name;
  std::string number;
  std::vector<std::vector<float> > coords;
};

#endif
