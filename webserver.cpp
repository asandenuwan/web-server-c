#include"webserver.h"

webserver::webserver(std::string ip,int port){
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
        std::cerr<<"WSAStartup failed"<<std::endl;
        exit(-1);
    }
    this->port=port;
    this->ipAddress=ip;

    // Create socket for server
    serverSocket=socket(AF_INET,SOCK_STREAM,0);
    if(serverSocket==INVALID_SOCKET){
        std::cerr<<"Socket creation failed"<<std::endl;
        this->~webserver();
        exit(-1);
    }

    // Set up server address structure
    SerIpAdd.sin_family=AF_INET;
    SerIpAdd.sin_port=htons(port);
    SerIpAdd.sin_addr.s_addr=inet_addr(ip.c_str());

    // Bind the socket to the address and port
    int c=bind(serverSocket,(sockaddr*)&SerIpAdd,sizeof(SerIpAdd));
    if(c!=0){
        std::cerr<<"bind failed"<<std::endl;
        this->~webserver();
        exit(-1);
    }

}
webserver::~webserver(){
    closesocket(serverSocket);
    closesocket(clientSocket);
    // Cleanup Winsock
    WSACleanup();
}

void webserver::run(int sec){
    DWORD timeout = sec*1000; 
    int c=setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    if(c!=0){
        std::cerr<<"time out setting error\n";
    }
    else{
        this->startServer();
    }
    
}

void webserver::startServer(){
    int c=listen(serverSocket,10);

    if (c==SOCKET_ERROR){
        std::cerr<<"listen failed"<<std::endl;
    }
    while (true){
        std::cout<<"Server started on http://"<<ipAddress<<":"<<port<<std::endl;

        //get incoming connection from client
        clientSocket=accept(serverSocket,(sockaddr*)&ClientIpAdd,&clientSize);

        if(clientSocket==INVALID_SOCKET){
            std::cerr<<"accept failed"<<std::endl;

        }
        std::cout<<inet_ntoa(ClientIpAdd.sin_addr)<<":"<<ntohs(ClientIpAdd.sin_port)<<" connected"<<std::endl;    
        int f=0; 
    
        std::string data;
        char buffer[4048]={0};
        // Receive data from the client
        int c=recv(clientSocket,buffer,sizeof(buffer),0);

        if(c==SOCKET_ERROR){
          
            if(WSAGetLastError()==WSAETIMEDOUT){
                std::cout<<"ERROR : time out\n";
                break;
            }
            else{
                f++;
                std::cerr<<"ERROR : recv failed "<<f<<std::endl;   
                if(f>=3){
                    std::cerr<<"ERROR : recv failed"<<std::endl;   
                    break;
                }
                Sleep(1000);
                continue;
            }
        }

        data= std::string(buffer);

        std::cout<<"####################### reqwest ##################\n"<<data<<"##########################\n";
        std::vector<std::string> filedata =this->getDataFromRequest(data);

        data=this->getFileData(filedata[0]);
        if(data!=""){
            std::string response=("HTTP/1.1 200 OK\r\n"
                "Content-Type: "+filedata[1]+"\r\n"
                "Content-Length: "+std::to_string(data.size())+"\r\n"
                "\r\n"+data+"\r\n");
            this->send(response);
        }
        else{
            // If the file is not found, send a 404 response
            std::string response=("HTTP/1.1 404 Not Found\r\n"
                "Content-type: text/html\r\n"
                "Content-length: 0\r\n\r\n");
            this->send(response);
        }
    }
}


void webserver::send(std::string data){
    // Send data to the client
    std::cout<<"_______________respond______________\n "<<data<<"_________________________________________\n";
    int c=::send(clientSocket,data.c_str(),data.size(),0);
    if(c==SOCKET_ERROR){
        std::cerr<<"send failed"<<std::endl;
    }
}

std::string webserver::getFileData(std::string filename){
    std::string type;
    for (size_t i = 0; i < filename.size(); i++){
        if(filename[i]=='.'){
            i++;
            for(i=i;i<filename.size();i++){
                type.push_back(filename[i]);
            }
            break;
        }
    }
    std::cout<<"type of file:"<<type<<"!\n";
    bool isText=false;
    std::vector<std::string> textList={"html","css","txt"};
    for(std::string t:textList){
        if(t==type){
            isText=true;
            break;
        }
    }
    std::string data;
    if(isText){
        data= this->getTextFile(filename,type);
    }
    else{
        data= this->getByiteFile(filename,type);
    }
    return data;
}

std::string webserver::getTextFile(std::string filename,std::string type){//eror
    std::ifstream file;
    std::string content="";
    file.open(filename);
    std::string data;
    if(file.is_open()){
        std::cout<<"text file open\n"; 
        while (std::getline(file,data)){
            content.append(data+"\n");
        }
        std::cout<<"t file>>> {"<<content<<"}!"<<std::endl;
    }
    else std::cout<<"ERROR : text file is  not open"<<std::endl;
    file.close();
    return content;
}

std::string webserver::getByiteFile(std::string filename,std::string type){//error
    std::ifstream file(filename,std::ios::binary);
    std::string data,content="";
    if(!file.is_open()){
        std::cout<<"ERROR  : binary file not open\n";
        return "";
    }
    while(std::getline(file,data)){
        content.append(data+"\n");
    }
    std::cout<<"b file>>>{"<<content<<"}!"<<std::endl;
    file.close();
    return content;
}

std::vector<std::string> webserver::getDataFromRequest(std::string data){
    std::string filename;
    for (size_t i = 0; i <data.size(); i++){
        if(data[i]=='/'){
            i++;
            for(i=i;i<data.size();i++){
                if(data[i]==' ')break;
                filename.push_back(data[i]);
            }
            break;
        }
    }
    if(filename=="")filename="index.html";
    std::cout<<"file name: !"<<filename<<"!"<<std::endl;
    
    std::string type;
    for(int i=0;i<filename.size();i++){
        if(filename[i]=='.'){
            i++;
            for(i=i;i<filename.size();i++){
                type.push_back(filename[i]);
            }
            break;
        }
    }   

    
    // select the correct type
    std::string retype;
    
    if(type=="html")retype="text/html";
    else if(type=="css")retype="text/css";
    else if(type=="txt")retype="text/plain";
    else if(type=="ico")retype="image/x-icon";
    else if(type=="jpeg")retype="image/jpeg";
    else if(type=="png")retype="image/png";
    else if(type=="jpg")retype="image/jpeg";
    else if(type=="gif")retype="image/gif";
    else if(type=="svg")retype="image/svg+xml";
    else if(type=="webp")retype="image/webp";
    else if(type=="mp3")retype="audio/mpeg";
    else if(type=="wav")retype="audio/wav";
    else if(type=="ogg")retype="audio/ogg";
    else if(type=="mp4")retype="video/mp4";
    else if(type=="mkv")retype="video/x-matroska";
    else if(type=="webm")retype="video/webm";
    else if(type=="ogg")retype="video/ogg";
    else if(type=="mp4")retype="video/mp4";
    else if(type=="mkv")retype="video/x-matroska";
    else if(type=="webm")retype="video/webm";
    else if(type=="ogg")retype="video/ogg";
    else if(type=="mp4")retype="video/mp4";
    else if(type=="mkv")retype="video/x-matroska";
    else if(type=="webm")retype="video/webm";
    else retype="application/octet-stream";

   
    
    std::cout<<"content type: !"<<retype<<"!\n";
    return std::vector<std::string>{filename,retype};
}