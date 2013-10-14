//
//  Peer.h
//  Gnutella
//
//  Created by Thomas Theissier on 03/10/13.
//  Copyright (c) 2013 Thomas Theissier. All rights reserved.
//

#ifndef __Gnutella__Peer__
#define __Gnutella__Peer__

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <dirent.h>
#include "QueryHistory.h"

class Peer {
private:
    std::string _ip;
    std::string _port;
    std::string _path;
    std::vector<std::string> _files;
    std::vector<Peer> _neighbours;
    std::vector<Peer> _connectedPeer;
    std::vector<QueryHistory> _history;
    
public:
    Peer(std::string pathConfig);
    ~Peer();
    Peer(std::string ip, std::string port);
    Peer(std::string ip, std::string port, std::string path, std::vector<Peer> neighbours, std::vector<std::string> files);
    
    int getFilesSize();
    int getNeighboursSize();
    std::string getFile(int index);
    std::string getPath();
    Peer getNeighBour(int index);
    int getNumberOfNeighbours();
    std::string getIp();
    std::string getPort();
    std::string getIdQuery();
    std::string newQuery();
    void addQuery(std::string idQuery);
    void addResultToHistory(std::string idQuery, std::string newPeer);
    int getNumberOfPeerForThisQuery(std::string idQuery);
    std::vector<std::string> getTablePeerForThisQuery(std::string idQuery);
    std::vector<std::string> getTablePeerForThisFile(std::string idQuery);
    void setQueryIsReceived(std::string idQuery);
    bool isQueryDone(std::string idQuery);
    
    void setPeerWithConfigFile(const std::string path);
    void setIp(std::string ip);
    void setPort(std::string port);
    void setPath(std::string path);
    
    void displayNeighbours();
    void addConnectedPeer(std::string ip, std::string port);
    void showYourFiles();
    std::string increIdQuery(std::string idQuery);
    
    bool isMyId(std::string id);
};

void readDirectory(std::string directory, std::vector<std::string> &files);

#endif /* defined(__Gnutella__Peer__) */
