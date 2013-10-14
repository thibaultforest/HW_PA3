//
//  QueryHistory.cpp
//  PA2
//
//  Created by forest on 06/10/13.
//  Copyright (c) 2013 Thomas Theissier. All rights reserved.
//

#include "QueryHistory.h"

QueryHistory::QueryHistory(std::string idQuery){_idQuery = idQuery;}
QueryHistory::~QueryHistory(){}

std::string QueryHistory::getIdQuery(){return _idQuery;}
std::vector<std::string> QueryHistory::getTablePeer(){return _tablePeer;}

void QueryHistory::addPeerToTable(std::string newPeer){
    bool result = false;
    for (int i = 0; i < _tablePeer.size(); i++) {
        if (newPeer == _tablePeer.at(i)) {
            result = true;
            break;
        }
    }
    if (!result)
        _tablePeer.push_back(newPeer);
}

bool QueryHistory::getIfReceived() { return _ifReceived; }
void QueryHistory::setIfReceived(bool ifReceived) { _ifReceived = ifReceived; }

std::string QueryHistory::getFileName(){
    std::vector<std::string> splitString;
    if(_tablePeer.size()>0){
        std::istringstream iss(_tablePeer.at(0));
        copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), back_inserter(splitString));
        return splitString.at(2);
    }
    return "";
}

int QueryHistory::getNumberOfPeer(){return (int)_tablePeer.size();}