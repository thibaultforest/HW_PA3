//
//  QueryHistory.h
//  PA2
//
//  Created by forest on 06/10/13.
//  Copyright (c) 2013 Thomas Theissier. All rights reserved.
//

#ifndef __PA2__QueryHistory__
#define __PA2__QueryHistory__

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <fstream>

class QueryHistory {
private:
    std::string _idQuery;
    std::vector<std::string> _tablePeer;
    bool _ifReceived;
public:
    QueryHistory(std::string idQuery);
    ~QueryHistory();
    std::string getIdQuery();
    std::vector<std::string> getTablePeer();
    void addPeerToTable(std::string newPeer);
    int getNumberOfPeer();
    bool getIfReceived();
    void setIfReceived(bool ifReceived);
    std::string getFileName();
};

#endif /* defined(__PA2__QueryHistory__) */
