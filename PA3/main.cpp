//
//  main.cpp
//  Gnutella
//
//  Created by Thomas Theissier on 03/10/13.
//  Copyright (c) 2013 Thomas Theissier. All rights reserved.
//

#include "Peer.h"
#include <pthread.h>
#include <mutex>
/*      SOCKET INCLUDE      */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define QUERYSIZE 256// Never used
#define FILESIZE 1024// Never used
/****************************/
#include <stdio.h>
#include <stdlib.h>
#define SIZEHISTORY 10

using namespace std;

typedef struct
{
    string idMessage;
    string type;
    string fileName;
    string version;
    string TTL;
    string sock;
}Query;


bool getCmd(Query &query, const std::string myCout);
void* server(void* data);
void* handleQueryClient(void* data);
void* client(void* data);
bool updateQueryHistory(Query query);
void* modifyFile(void* data);
void updateFile(Query myQuery);
void* pullThread(void* data);
string queryHistory[SIZEHISTORY]={""};
bool pullBased = false;

/****** FUNCTIONS ******/
void searchQuery(void* data);
void downloadQuery(void *data);
void invalidate(void* data);
void sendFile(void* data);
void recvFile(void *data);
void broadcastModif(void* data);
bool sendQuery(string ip, string port, Query myQuery);
/**********************/

Peer *myPeer;

int main(int argc, const char * argv[])
{
    Peer firstPeer("/Users/thomastheissier/Desktop/PA3/PA3/CONFIG.txt");///Users/thomastheissier/Desktop/PA2/PA2/CONFIG.txt");
    myPeer = &firstPeer;
    firstPeer.displayNeighbours();
    firstPeer.showYourFiles();
    
    pthread_t threadClient;
    pthread_t threadServer;
    pthread_t threadPull;
    
    pthread_create(&threadServer, NULL, server, NULL);
    sleep(2);
    pthread_create(&threadClient, NULL, client, NULL);
    pthread_create(&threadPull, NULL, pullThread, NULL);
    
    pthread_join(threadClient, NULL);
    pthread_join(threadServer, NULL);
    pthread_join(threadPull, NULL);
    return 0;
}

void* server(void* data)
{
    SOCKADDR_IN sin;
    SOCKET sock;
    socklen_t recsize = sizeof(sin);
    
    int sock_err;
    
    /* Creation of the socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    /* If the socket is ok */
    if(sock != INVALID_SOCKET)
    {
        printf("Socket %d opened in mode TCP/IP\n", sock);
        string strPort = myPeer->getPort();
        int port = atoi(strPort.c_str());
        /* Configuration */
        sin.sin_addr.s_addr = htonl(INADDR_ANY);  /* All IP Addresses   */
        sin.sin_family = AF_INET;                 /* Set protocol       */
        sin.sin_port = htons(port);               /* Set port           */
        sock_err = ::bind(sock, (SOCKADDR*)&sin, recsize);
        
        /* If sock is working */
        if(sock_err != SOCKET_ERROR)
        {
            /* Listen (mode server) */
            sock_err = listen(sock, 5);
            printf("Port %d is listening...\n", port);
            while(1){
                /* Client socket */
                SOCKADDR_IN csin;
                SOCKET csock;
                socklen_t crecsize = sizeof(csin);
                
                if(sock_err != SOCKET_ERROR)
                {
                    /* Waiting for a client.. */
                    csock = accept(sock, (SOCKADDR*)&csin, &crecsize);
                    printf("A peer is connected with socket %d from %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
                    //                    myPeer->addConnectedPeer(inet_ntoa(csin.sin_addr), to_string(htons(csin.sin_port)));
                    pthread_t t;
                    fflush(stdout);
                    pthread_create(&t, NULL, handleQueryClient, &csock);
                }
                else
                    perror("listen");
            }
        }
        else
            perror("bind");
        
        //        printf("Closing client socket\n");
        //        closesocket(csock);
        printf("Closing server socket..\n");
        closesocket(sock);
        printf("Server socket Closed.\n");
    }
    else
        perror("socket");
    
    return NULL;
}

void* handleQueryClient(void *data){
    Query query;
    int stop = 0;
    vector<string> cutString;
    while (stop == 0) {
        ssize_t bytesBuffer = recv(*(SOCKET*)data, &query, sizeof(query), 0);
        if (bytesBuffer > 0)
            stop = 1;
    }
    query.sock = to_string(*(SOCKET*)data);
    myPeer->addQuery(query.idMessage);
    
    if(updateQueryHistory(query)){
        
        if (query.type == "download") {    // Launch sendFile if fileName found.
            for (int i = 0; i < myPeer->getFilesNumber(); i++) {
                if (myPeer->getFileName(i) == query.fileName && (myPeer->getFileVersion(i) == query.version || query.version == "NA")) {
                      sendFile(&query);
                    break;
                }
            }
        }
        else if (query.type == "search") {     // Search file requested by a Peer.
              searchQuery(&query);
        }
        else if (query.type == "invalidate"){
              invalidate(&query);
            
        }
        else {
            cout << "Unknown query !" << endl;
        }
    }
    else
        cout << "Query already sent.\n";
    closesocket(*(SOCKET*)data);
    return NULL;
}

void sendFile(void* data){
    Query myQuery = *(Query*) data;
    string strPath = myPeer->getPathFiles(myQuery.fileName);
    SOCKET sock = atoi(myQuery.sock.c_str());
    char fs_name[100] = "";
    strcpy(fs_name, strPath.c_str());
    strcat(fs_name, myQuery.fileName.c_str());
    
    char sdbuf[1024];
    //myMutex.lock();
    printf("[Client] Sending %s to the peer...\n", fs_name);
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
        printf("ERROR: File %s not found.\n", fs_name);
        exit(1);
    }
    else{
        string buffer = "YES";
        send(sock, &buffer, sizeof(buffer), 0);
    }
    
    bzero(sdbuf, 1024);
    long fs_block_sz;
    while((fs_block_sz = fread(sdbuf, sizeof(char), 1024, fs)) > 0)
    {
        if(send(sock, sdbuf, 1024, 0) < 0)
        {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
            break;
        }
        bzero(sdbuf, 1024);
    }
    if(send(sock, "END", sizeof("END"), 0) < 0)
        printf("ERROR\n");
    printf("Ok File %s is sent !\n", fs_name);
    //myMutex.unlock();
}

void recvFile(void* data){
    //myMutex.lock();
    printf("[Client] Receiveing file from the peer and saving it ...\n");
    Query myQuery = *(Query*) data;
    string strPath = myPeer->getPathDownload();
    SOCKET sock = atoi(myQuery.sock.c_str());
    char fr_name[100] = "";
    strcpy(fr_name, strPath.c_str());
    strcat(fr_name, myQuery.fileName.c_str());
    
    FILE *fr = fopen(fr_name, "a");
    if(fr == NULL)
        printf("File %s Cannot be opened.\n", fr_name);
    else
    {
        char revbuf[1024];
        bzero(revbuf, 1024);
        long fr_block_sz = 0;
        while(1)
        {
            fr_block_sz = recv(sock, revbuf, 1024, 0);
            if(fr_block_sz>0)
            {
                if(revbuf[0]=='E' && revbuf[1]=='N' && revbuf[2]=='D' && revbuf[3]=='\0')
                {
                    break;
                }
                else
                {
                    long write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                    if(write_sz < fr_block_sz)
                    {
                        printf("File write failed.\n");
                    }
                    bzero(revbuf, 1024);
                }
            }
        }
        if(fr_block_sz < 0)
        {
            if (errno == EAGAIN)
            {
                printf("recv() timed out.\n");
            }
            else
            {
                fprintf(stderr, "recv() failed due to errno = %d\n", errno);
            }
        }
        printf("Ok received from the peer!\n");
        // myMutex.unlock();
        myPeer->setQueryIsReceived(myQuery.idMessage);
        fclose(fr);
    }
}

void invalidate(void* data){
    //Forward invalidate msg to neighboors.
    Query myQuery = *(Query*) data;
    broadcastModif(&myQuery);
    
    File thisFile(myQuery.fileName, myQuery.version, "", "", 0);
    if(myPeer->haveWrongFileVersion(thisFile)){
        updateFile(myQuery);
    }
}

void* pullThread(void* data){
    
    while(1){
        sleep(10000);
        while (pullBased) {
#warning implement pullBased here
            // 1. Decrement all TTR's files;
            // 2. query version file for each file which have TTR=0;
            vector<File> filesToVerify = myPeer->decrementTTRFiles();
            // 3. Download (void updateFile(Query myQuery)) file which have a wrong version;
        }
    }
    
    return NULL;
}

void updateFile(Query myQuery){
    myQuery.type = "download";
    myQuery.idMessage = myPeer->newQuery();
    std::vector<std::string> splitString;
    std::istringstream iss(myQuery.version);
    copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), back_inserter(splitString));
    SOCKET sock;
    SOCKADDR_IN sin;
    /* Création de la socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    /* Configuration de la connexion */
    sin.sin_addr.s_addr = inet_addr((splitString.at(0)).c_str());
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(splitString.at(1).c_str()));
    
    /* Si le client arrive à se connecter */
    if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){
        printf("Connected to %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
        if(send(sock, &myQuery, sizeof(myQuery), 0) == SOCKET_ERROR)
            printf("Send failed.\n");
        else{
            string buffer;
            while (buffer != "YES") {
                recv(sock, &buffer, sizeof(buffer), 0);
                if(buffer == "YES"){
                    myQuery.sock = to_string(sock);
                    recvFile(&myQuery);
                    break;
                }
            }
        }
    }
    else
        printf("Connection failed.\n");
    
    closesocket(sock);
}

void searchQuery(void* data){       //Decrement TTL and Forward Query
    clock_t start, end;
    start = clock();
    Query myQuery = *(Query*) data;
    int TTL = atoi(myQuery.TTL.c_str());
    if(TTL > 0) {
        TTL--;
        myQuery.TTL = to_string(TTL);
        for (int i = 0; i < myPeer->getNumberOfNeighbours(); i++) {
            Peer neighbour = myPeer->getNeighBour(i);
            if(!neighbour.isQuerySender(myQuery.idMessage)){
                SOCKET sock;
                SOCKADDR_IN sin;
                /* Creation socket */
                sock = socket(AF_INET, SOCK_STREAM, 0);
                /* Connection config */
                sin.sin_addr.s_addr = inet_addr((neighbour.getIp()).c_str());
                sin.sin_family = AF_INET;
                sin.sin_port = htons(atoi(neighbour.getPort().c_str()));
                
                /* If the client connects */
                if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){
                    printf("Connected to %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
                    if(send(sock, &myQuery, sizeof(myQuery), 0) == SOCKET_ERROR)
                        printf("Send failed.\n");
                    else{
                        char buffer[64] = "";
                        while (strcmp(buffer, "end")) {
                            if(recv(sock, &buffer, sizeof(buffer), 0) != SOCKET_ERROR){
                                if(strcmp(buffer, "") && strcmp(buffer, "end")){
                                    myPeer->addResultToHistory(myQuery.idMessage, buffer);
                                }
                            }
                        }
                    }
                }
                else
                    printf("Connection failed\n");
                
                closesocket(sock);
            }
        }
    }
    
    if(!myPeer->isMyId(myQuery.idMessage)){// If searchQuery is not mine
        
        for (int i = 0; i < myPeer->getFilesNumber(); i++) {
            string nameOfFile = myPeer->getFileName(i);
            if (nameOfFile == myQuery.fileName) {//I add my adress if have the file.
                string info = myPeer->getIp()+" "+myPeer->getPort()+" "+myQuery.fileName;
                myPeer->addResultToHistory(myQuery.idMessage, info);
                break;
            }
        }
        vector<string> tablePeer = myPeer->getTablePeerForThisQuery(myQuery.idMessage);
        cout << tablePeer.size() << " Results for query : '" << myQuery.idMessage << "' of type : " << myQuery.type << endl;
        SOCKET destSock;
        destSock = atoi(myQuery.sock.c_str());
        char buffer[64];
        for(int i=0; i<(int)tablePeer.size(); i++){      // Then I reply results of my research.
            strcpy(buffer, tablePeer.at(i).c_str());
            if(send(destSock, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
                printf("Send failed.\n");
            else
                cout << "Result " << i << " : " << buffer << endl;
        }
        strcpy(buffer, "end");
        if(send(destSock, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
            printf("Send failed.\n");
        closesocket(destSock);//Close socket of the peer who asked for a search
    }
    else {
        vector<string> tablePeer = myPeer->getTablePeerForThisQuery(myQuery.idMessage);
        cout << tablePeer.size() << " Results for query : '" << myQuery.idMessage << "' of type : " << myQuery.type << endl;
        for(int i=0; i<(int)tablePeer.size(); i++)
            cout << "Result " << i << " : " << tablePeer.at(i) << endl;
    }
    end = clock()-start;
    cout << "It took " << (double)end/(double)CLOCKS_PER_SEC << " seconds for this search query.\n";
}

void* client(void* data)
{
    Query myQuery;
    while(getCmd(myQuery, "]")){
        if(myQuery.type == "search"){
            searchQuery(&myQuery);
        }
        else if(myQuery.type == "download"){
            downloadQuery(&myQuery);
        }
        else if (myQuery.type == "modify"){
            myQuery.version = myPeer->modifyFile(myQuery.fileName);
            if(!pullBased){
                broadcastModif(&myQuery);
            }
        }
        else if (myQuery.type == "lookup"){
            //myMutex.lock();
            myPeer->showYourFiles();
            //myMutex.unlock();
        }
        else if (myQuery.type == "pull"){
            //            pullPush.lock();
            pullBased = true;
            cout << "Pull-based ON; Push-based OFF\n";
            //            pullPush.unlock();
        }
        else if (myQuery.type == "push"){
            //            pullPush.lock();
            pullBased = false;
            cout << "Push-based ON; Pull-based OFF\n";
            //            pullPush.unlock();
        }
    }
    return NULL;
}

void broadcastModif(void* data){
    //    pullPush.lock();
    cout << "Broadcasting invalidate...\n";
    Query myQuery = *(Query*) data;
    myQuery.type = "invalidate";
    int TTL = atoi(myQuery.TTL.c_str());
    if(TTL > 0) {
        TTL--;
        myQuery.TTL = to_string(TTL);
        for (int i = 0; i < myPeer->getNumberOfNeighbours(); i++) {
            Peer neighbour = myPeer->getNeighBour(i);
            if (!neighbour.isFileOwner(myQuery.version)) {
                if(!sendQuery(neighbour.getIp(), neighbour.getPort(), myQuery))
                    cout << "Connection to neighboor " << i << " failed.\n";
            }
        }
    }
    cout << "Done!\n";
    //    pullPush.unlock();
}

void downloadQuery(void* data){
    Query myQuery = *(Query*) data;
    vector<string> tablePeer = myPeer->getTablePeerForThisFile(myQuery.fileName);
    if(tablePeer.size() != 0){
        for (int i = 0; i < tablePeer.size(); i++) {
            if (!myPeer->isQueryDone(myQuery.idMessage)) {
                std::vector<std::string> splitString;
                std::istringstream iss(tablePeer.at(i));
                copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), back_inserter(splitString));
                
                SOCKET sock;
                SOCKADDR_IN sin;
                
                sock = socket(AF_INET, SOCK_STREAM, 0);
                
                sin.sin_addr.s_addr = inet_addr((splitString.at(0)).c_str());
                sin.sin_family = AF_INET;
                sin.sin_port = htons(atoi(splitString.at(1).c_str()));
                
                if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){
                    printf("Connected to %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
                    if(send(sock, &myQuery, sizeof(myQuery), 0) == SOCKET_ERROR)
                        printf("Send failed.\n");
                    else{
                        string buffer;
                        while (buffer != "YES") {
                            recv(sock, &buffer, sizeof(buffer), 0);
                            if(buffer == "YES"){
                                myQuery.sock = to_string(sock);
                                recvFile(&myQuery);
                                break;
                            }
                        }
                    }
                }
                else
                    printf("Connection failed.\n");
                
                closesocket(sock);
            }
        }
    }
    else
        cout << "No Peers available to download this file.\nTry to search it.\n";
}

#warning The function below should work!!
bool sendQuery(string ip, string port, Query myQuery){
    bool myBool = true;
    
    SOCKET sock;
    SOCKADDR_IN sin;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    sin.sin_addr.s_addr = inet_addr(ip.c_str());
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(port.c_str()));
    
    if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR){
        printf("Connected to %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
        if(send(sock, &myQuery, sizeof(myQuery), 0) == SOCKET_ERROR)
            printf("Send failed.\n");
    }
    else
        myBool = false;// Connection Failed!
    
    closesocket(sock);
    
    return myBool;
}

bool getCmd(Query &query, const std::string myCout) {
    
    query.idMessage = "";
    query.fileName = "";
    query.sock = "";
    query.TTL = "";
    query.type = "";
    query.version = "";
    
    cout << myCout;
    vector<string> vect;
    string sentence;
    getline(cin, sentence);
    istringstream iss(sentence);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(vect));
    if(vect.at(0) == "exit" || (vect.size() == 0))
        return false;
    else if(vect.size() == 2){
        query.type = vect.at(0);
        query.fileName = vect.at(1);
        query.version = "NA";
        query.TTL = "2";
        query.idMessage = myPeer->newQuery();
    }
    return true;
}

bool updateQueryHistory(Query query){
    for(int i=0; i<SIZEHISTORY; i++)
//        if(queryHistory[i] == query.idMessage)
//            return false;
    
    for(int i=SIZEHISTORY-1; i>0; i--)
        queryHistory[i] = queryHistory[i-1];
    
    queryHistory[0] = query.idMessage;
    
    return true;
}