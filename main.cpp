#include "webserver.h"

int main() {
    while (1){
        // Create a webserver object with the desired IP address and port
        webserver server("127.0.0.1", 8080);
        //time out
        server.run(3);
        // Enter the main loop to handle incoming requests
    }
    return 0;
}