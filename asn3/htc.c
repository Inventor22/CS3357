#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "udp_client.h"
#include "udp_sockets.h"
#include "syslog.h";
#include "stdbool.h";

int main() {
    openlog("rftp", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);
    setlogmask(LOG_UPTO(LOG_DEBUG));

    //htc [-t|--type TYPE] [-p|--param NAME=VALUE ...] SERVER PATH
    char c;
    char* port = "80"; // standard HTTP port
    char* serverStr = NULL;
    char *name = NULL, *lname = NULL, *level = NULL;
    char* temp = NULL;
    char* type = NULL;
    char* path = NULL;

    while (true) {
        static struct option long_options[] =
        {
            {"type", required_argument, 0, 'p'},
            {"param", required_argument, 0, 't'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "t:p:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'p':
                *temp = strtok(optarg, "=");
                if (strcmp(temp, "name") == 0)
                {
                    *name = strtok(temp, "=");
                    syslog(LOG_DEBUG, "name: %s", *name);
                }
                else if (strcmp(temp, "lname") == 0)
                {
                    *lname = strtok(temp, "=");
                    syslog(LOG_DEBUG, "lname: %s", *lname);
                }
                else if (strcmp(temp, "level") == 0)
                {
                    *level = strtok(temp, "=");
                    syslog(LOG_DEBUG, "level: %s", *level);
                }
                else {
                    syslog(LOG_INFO, "Invalid param");
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                *type = optarg;
                syslog(LOG_DEBUG, "type: %s", *type);
                break;
            case '?':
                break;
        }
    }

    syslog(LOG_DEBUG, "argc: %d, optind: %d", argc, optind);
    if (argc-optind != 2) {
        syslog(LOG_DEBUG, "Server or path not specified!");
        exit(EXIT_FAILURE);
    }
    *serverStr = argv[optind];
    *path = argv[optind+1];

    host_t server;   // Server address
    message_t* msg;  // Message to send/receive

    // Create a socket for communication with the server
    int sockfd = create_client_socket(serverStr, port, &server);

    // Create a message, and initialize its contents
    msg = create_message();
    msg->length = strlen("hello");
    memcpy(msg->buffer, "hello", msg->length);

    // Send the message to the server, and free its memory
    int retval = send_message(sockfd, msg, &server);
    free(msg);

    // If we couldn't send the message, exit the program
    if (retval == -1) {
        close(sockfd);
        perror("Unable to send to socket");
        exit(EXIT_FAILURE);
    }

    // Read the server's reply
    msg = receive_message(sockfd, &server);

    if (msg != NULL) {
        // Add NULL terminator and print reply
        msg->buffer[msg->length] = '\0';
        printf("Reply from server %s: %s\n", server.friendly_ip, msg->buffer);

        // Free the memory allocated to the message
        free(msg);
    }

    // Close the socket
    close(sockfd);
    exit(EXIT_SUCCESS);
}
