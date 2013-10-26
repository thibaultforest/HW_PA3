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
#include "File.h"

typedef struct
{
    std::string idMessage;
    std::string type;
    std::string fileName;
    std::string version;
    std::string TTL;
    std::string sock;
}Query;

class Peer {
    private:
    std::string _ip;
    std::string _port;
    int _TTR;
    std::string _pathMyFiles;
    std::string _pathDownloads;
    std::vector<File> _files;
    std::vector<Peer> _neighbours;
    std::vector<Peer> _connectedPeer;
    std::vector<QueryHistory> _history;
    
    public:
    Peer(std::string pathConfig);
    ~Peer();
    Peer(std::string ip, std::string port);
    
    void setPeerWithConfigFile(const std::string path);
    
    void setPathMyfiles(std::string path);
    void setPathDownloads(std::string path);
    void showYourFiles();
    void readDirectory(std::string directory);
    void displayNeighbours();
    
    std::string modifyFile(std::string fileName);
    
    int getFilesNumber();
    std::string getFileName(int index);
    std::string getFileVersion(int index);
    std::string getPathFiles(std::string name);
    std::string getPathDownload();
    bool haveWrongFileVersion(File file);
    int haveUpgradeFileVersion(File file);
    bool isQuerySender(std::string idMessage);
    bool isFileOwner(std::string version);
    
    Peer getNeighBour(int index);
    int getNumberOfNeighbours();
    
    void setIp(std::string ip);
    std::string getIp();
    void setPort(std::string port);
    std::string getPort();
    void setTTR(int TTR);
    int getTTR();
    void resetTTRWithFileName(std::string fileName);
    bool addFileToPeer(Query queryDownload);
    std::string getVersionWithFileName(std::string fileName);
    
    std::string getIdQuery();
    std::string newQuery();
    void addQuery(std::string idQuery);
    void addResultToHistory(std::string idQuery, std::string newPeer);
    int getNumberOfPeerForThisQuery(std::string idQuery);
    std::vector<std::string> getTablePeerForThisQuery(std::string idQuery);
    std::vector<std::string> getTablePeerForThisFile(std::string idQuery);
    void setQueryIsReceived(std::string idQuery);
    bool isQueryDone(std::string idQuery);
    
    std::string increIdQuery(std::string idQuery);
    
    bool isMyId(std::string id);
    
    std::vector<File> decrementTTRFiles();
};


#endif /* defined(__Gnutella__Peer__) */
