#include <winsock2.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <windows.h>
class webserver{
    private:
        WSADATA wsa;
        sockaddr_in SerIpAdd,ClientIpAdd;
        SOCKET serverSocket, clientSocket;
        int port;
        int clientSize=sizeof(ClientIpAdd);
        std::string ipAddress;
        void send(std::string data);
        std::vector<std::string> getDataFromRequest(std::string data);
        std::string getFileData(std::string filename);
        std::string getTextFile(std::string filename, std::string type);
        std::string getByiteFile(std::string filename, std::string type);
        void startServer();
    public:
        void run(int sec);
        webserver(std::string ip,int port);
        ~webserver();
        
   
};