//
//  File.cpp
//  PA3
//
//  Created by forest on 14/10/13.
//  Copyright (c) 2013 TTF. All rights reserved.
//

#include "File.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace std;

File::File(std::string name, std::string version, std::string origin, std::string path) {
    _name = name;
    _version = version;
    _origin = origin;
    _path = path;
}
File::~File(){}


void File::setVersion(std::string newVersion) {
    _version = newVersion;
}
std::string File::getName(){return _name;}
std::string File::getVersion(){return _version;}
std::string File::getPath(){return _path;}

void File::displayFileInfo(){
    std::cout << _name << "\t\tV_" << _version << "\t\t" << _path << std::endl;
}

void File::modif(string prefixVersion){
    int id = 0;
    vector<string> splitVersion;
    istringstream iss(_version);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(splitVersion));
    if(prefixVersion == splitVersion[0]+" "+splitVersion[1]){
        id = atoi(splitVersion[2].c_str());
        id++;
    }
    _version = prefixVersion+" "+to_string(id);
}

bool File::sameFileButDifferentVersion(File file){
    if(file._name == _name && file._version != _version)
        return true;
    return false;
}