//
//  File.h
//  PA3
//
//  Created by forest on 14/10/13.
//  Copyright (c) 2013 TTF. All rights reserved.
//

#ifndef __PA3__File__
#define __PA3__File__

#include <iostream>
#include <string>


class File {
private:
    std::string _name;
    std::string _version;
    std::string _origin;
    std::string _path;
    int _TTR;
    bool isModified;
    
public:
    File(std::string name, std::string version, std::string origin, std::string path, int TTR);
    ~File();
    
    void setVersion(std::string newVersion);
    std::string getName();
    std::string getVersion();
    std::string getPath();
    int getTTR();
    void decrementTTR();
    void resetTTR(int TTR);
    void displayFileInfo();
    bool downFileVersion(File file);
    bool upFileVersion(File file);
    
    void modif(std::string prefixVersion);
};

#endif /* defined(__PA3__File__) */
