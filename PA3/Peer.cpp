//
//  Peer.cpp
//  Gnutella
//
//  Created by Thomas Theissier on 03/10/13.
//  Copyright (c) 2013 Thomas Theissier. All rights reserved.
//

#include "Peer.h"
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define PORT 50000

using namespace std;


Peer::Peer(string pathConfig) {
    setPeerWithConfigFile(pathConfig);
    readDirectory(_path, _files);
}

Peer::~Peer() {
    //destructor
}

Peer::Peer(string ip, string port, string path, vector<Peer> neighbours, vector<string> files) {
    _ip = ip;
    _port = port;
    _files = files;
    _neighbours = neighbours;
    _path = path;
}

Peer::Peer(string ip, string port) {
    _ip = ip;
    _port = port;
}

string Peer::increIdQuery(string idQuery){
    int id = atoi(&idQuery.at(idQuery.size()-1));
    id++;
    idQuery = idQuery.substr(0, idQuery.size()-1);
    idQuery += to_string(id);
    return idQuery;
}

void Peer::showYourFiles() {
    for (int i = 0; i < _files.size(); i++) {
        cout << _files[i] << endl;
    }
}

void Peer::setPeerWithConfigFile(const std::string path){
    ifstream fichier(path, ios::in);  // Read mode opening "/Users/forest/Documents/CmdLine/CmdLine/CONFIG.txt"
    vector<string> vect;
    if(fichier)  // If file is opened.
    {
        string contenu = "Init";
        while(contenu != ""){                       //Start reading the file
            getline(fichier, contenu);
            
            if(contenu == "PEERCONFIG\r"){            //Get peer's config
                vect.clear();
                string str;
                getline(fichier,str);
                contenu = str.substr(0, str.size()-1);
                istringstream iss(contenu);
                copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(vect));
                if(vect.size() == 3){
                    setIp(vect.at(0));
                    setPort(vect.at(1).c_str());
                    setPath(vect.at(2));
                }
            }
            else if (contenu == "NEIGHBOURS\r"){      //Get neighbours' config
                string str = "Init";
                while(str != ""){
                    vect.clear();
                    string str;
                    getline(fichier,str);
                    if(str != ""){
                        contenu = str.substr(0, str.size()-1);
                        istringstream iss(contenu);
                        copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(vect));
                        if(vect.size() == 2){
                            Peer newNeighbour(vect.at(0), vect.at(1).c_str());
                            _neighbours.push_back(newNeighbour);
                        }
                    }
                    else
                        break;
                }
                contenu = "";
            }
        }
        cout << "Configuration done!\n";
        fichier.close();
    }
    else
        cerr << "Error opening config file." << endl;
}
void Peer::displayNeighbours(){
    for(int i=0; i<_neighbours.size(); i++)
        cout << "Neighbour " << i << " ip : " << _neighbours.at(i)._ip << ", port : " << _neighbours.at(i)._port << endl;
}

int Peer::getFilesSize() { return (int)_files.size(); }
int Peer::getNeighboursSize() { return (int)_neighbours.size(); }
string Peer::getFile(int index) { return _files[index]; }
string Peer::getPath(){return _path;}
void Peer::setIp(std::string ip){_ip = ip;}
void Peer::setPort(std::string port){_port = port;}
void Peer::setPath(std::string path){_path = path;}
Peer Peer::getNeighBour(int index){return _neighbours.at(index);}
int Peer::getNumberOfNeighbours(){return (int)_neighbours.size();}
std::string Peer::getIp(){return _ip;}
std::string Peer::getPort() {return _port;}

std::string Peer::newQuery(){
    string newIdQuery;
    if(_history.size()>0){
        string lastIdQuery = _history.at(_history.size()-1).getIdQuery();
        newIdQuery = increIdQuery(lastIdQuery);
    }
    else
        newIdQuery = _ip+" "+_port+" "+to_string(0);
    
    QueryHistory newQuery(newIdQuery);
    _history.push_back(newQuery);
    return newIdQuery;
}

void Peer::addQuery(string idQuery){
    QueryHistory newQuery(idQuery);
    _history.push_back(newQuery);
}

void Peer::addResultToHistory(string idQuery, string newPeer){
    for(int i=0; i<(int)_history.size(); i++){
        if(_history.at(i).getIdQuery() == idQuery){
            _history.at(i).addPeerToTable(newPeer);
            break;
        }
    }
}

int Peer::getNumberOfPeerForThisQuery(std::string idQuery){
    for(int i=0; i<(int)_history.size(); i++)
        if(_history.at(i).getIdQuery() == idQuery)
            return _history.at(i).getNumberOfPeer();
    
    return -1;
}

std::vector<std::string> Peer::getTablePeerForThisQuery(std::string idQuery){
    vector<string> vect;
    for(int i=0; i<(int)_history.size(); i++)
        if(_history.at(i).getIdQuery() == idQuery)
             return _history.at(i).getTablePeer();
    
    return vect;
}

std::vector<std::string> Peer::getTablePeerForThisFile(std::string fileName){
    vector<string> vect;
    for(int i=0; i<(int)_history.size(); i++)
        if(_history.at(i).getFileName() == fileName)
            return _history.at(i).getTablePeer();
    
    return vect;
}

void Peer::setQueryIsReceived(std::string idQuery) {
    for (int i = 0; i < (int)_history.size(); i++) {
        if (_history.at(i).getIdQuery() == idQuery) {
            _history.at(i).setIfReceived(true);
            break;
        }
    }
}

bool Peer::isQueryDone(std::string idQuery) {
    for (int i = 0; i < (int)_history.size(); i++)
        if (_history.at(i).getIdQuery() == idQuery)
            return _history.at(i).getIfReceived();
    
    return false;
}

void Peer::addConnectedPeer(std::string ip, std::string port){
    Peer newPeer(ip, port);
    _connectedPeer.push_back(newPeer);
}

bool Peer::isMyId(string id){
    vector<string> idMessage;
    istringstream iss(id);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(idMessage));
    if(idMessage.size() == 0)
        return false;
    if(idMessage.at(0) == _ip && idMessage.at(1) == _port)
        return true;
    return false;
}


/************** END OF CLASS IMPLEMENTATION ************************/


void readDirectory(string directory, vector<std::string> &files) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(directory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (strcmp(ent->d_name, ".localized")) {
                if (strcmp(ent->d_name, ".DS_Store")) {
                    if ((strcmp(ent->d_name, "."))) {
                        if ((strcmp(ent->d_name, ".."))) {
                            string nameOfFile = string(ent->d_name);
                            files.push_back(nameOfFile);
                        }
                    }
                }
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
}