//
//  File.cpp
//  PA3
//
//  Created by forest on 14/10/13.
//  Copyright (c) 2013 TTF. All rights reserved.
//

#include "File.h"

File::File(std::string name, std::string version, std::string path) {
    _name = name;
    _version = version;
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
    std::cout << _name << "\tV_" << _version << "\tLocated in :" << _path << std::endl;
}