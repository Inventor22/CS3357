#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "udp_client.h"
#include "syslog.h";
#include "stdbool.h";
#include "udp_sockets.h"

int main(int argc, char **argv) {
    openlog("rftp", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);
    setlogmask(LOG_UPTO(LOG_DEBUG));

    //htc [-t|--type TYPE] [-p|--param NAME=VALUE ...] SERVER PATH
    char c;
    char* port = "80"; // standard HTTP port
    char* serverStr = NULL;
    char* type = NULL;
    char* path = NULL;

    // typedef struct {
    // 	char* key;
    // 	char* value;
    // } KeyValue;

    //KeyValue keyValue[100];

    char* params[100];
    int index = 0;
    int paramsLength = 0;

    while (true) {
        static struct option long_options[] =
        {
            {"type", required_argument, 0, 't'},
            {"param", required_argument, 0, 'p'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "t:p:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 'p':
            	params[index] = optarg;
            	paramsLength += strlen(params[index]);
                // keyValue[keyValueIndex].key   = strtok(optarg, "=");
                // keyValue[keyValueIndex].value = strtok(optarg, "=");
                // paramsLength += strlen(keyValue[keyValueIndex].key);
                //paramsLength += strlen(keyValue[keyValueIndex].value);
                //paramsLength++; // for the '=' operator
                index++;
                break;
            case 't':
                type = optarg;
                syslog(LOG_DEBUG, "type: %s", type);
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
    serverStr = argv[optind];
    path = argv[optind+1];

    host_t server;   // Server address
    message_t* msg;  // Message to send/receive

    // Create a socket for communication with the server
    int sockfd = create_client_socket(serverStr, port, &server);

    // Create a message, and initialize its contents
    msg = create_message();
    //msg->length = strlen("hello");
    //memcpy(msg->buffer, "hello", msg->length);

    // Build up header string
    // size_t request_len = strlen("GET  HTTP/1.1\r\nHOST: \r\n\r\n")
    //         + strlen(serverStr) + strlen(path);
    // char *request = malloc(sizeof(char)*request_len);
    // sprintf(request, "GET %s HTTP/1.1\r\nHOST: %s\r\n\r\n", path, serverStr);


    // TODO: Figure out where the get arguments actually go
    // Just append for a get request???

    if (strcmp(type, "GET") == 0)
    {
    	/*
    	# Send a GET request to http://1.1.1.1/form.cgi?fname=jeff&lname=shantz&level=phd
		$ ./htc -t GET -p name=jeff -p lname=shantz -p level=phd 1.1.1.1 /form.cgi
		(HTML response displayed)
		*/
		const int sz = 4096;
    	char requestBuf[sz];

		/// Form request
snprintf(sendline, MAXSUB, 
     "GET %s HTTP/1.0\r\n"  // POST or GET, both tested and works. Both HTTP 1.0 HTTP 1.1 works, but sometimes 
     "Host: %s\r\n"     // but sometimes HTTP 1.0 works better in localhost type
     "Content-type: application/x-www-form-urlencoded\r\n"
     "Content-length: %d\r\n\r\n"
     "%s\r\n", page, host, (unsigned int)strlen(poststr), poststr);




    	sprintf(requestBuf, "GET %s HTTP/1.1\r\nHOST:http://%s", path, serverStr);

    	bool first = true;
    	for (int i = 0; i < paramsLength; ++i)
    	{
    		if (first) {
    			strcat(requestBuf, "?");
    			strcat(requestBuf, params[i]);
    			first = false;
    		} else {
    			strcat(requestBuf, "&");
    			strcat(requestBuf, params[i]);
    		}
    	}
    	strcat(requestBuf, "\r\n\r\n");
    	msg->length = strlen(requestBuf);
    	memcpy(msg->buffer, requestBuf, msg->length);

    	printf("Requesting: %s\n", path);
    }
    else if (strcmp(type, "POST"))
    {

    }

    // Null strings that we don't need anymore
    // Don't free program args...
    if (path)
    {
        path = NULL;
    }
    if (serverStr)
    {
        serverStr = NULL;
    }

    int chars_sent = 0;
    int chars_total = 0;
    for (chars_total = 0; chars_total < request_len; chars_total += chars_sent)
    {
        msg->length = UDP_MSS < request_len - chars_total ? UDP_MSS : request_len - chars_total;
        if (msg->length == 0)
            break;
        memcpy(msg->buffer, &request[chars_total], (size_t)msg->length);
        // TODO: Check that the return value is actually the number of bytes sent
        chars_sent = send_message(sockfd, msg, &server);
        // If we couldn't send the message, exit the program
        if (chars_sent == -1) {
            close(sockfd);
            perror("Unable to send to socket"); // TODO: Log with syslog! (Andrew: I've done it before, I'll do it...)
            exit(EXIT_FAILURE);
        }
        if (chars_sent == 0)
            break;
    }

    // Free the message
    free(msg);

    // Read the server's reply
    msg = receive_message(sockfd, &server);

    if (msg != NULL) {
        // Add NULL terminator and print reply
        msg->buffer[msg->length] = '\0';
        printf("Reply from server %s: %s\n", server.friendly_ip, msg->buffer);

        // TODO: Check the return code
        // If 2XX, print the message

        // If !2XX, print "Error: status code XXX"

        // Free the memory allocated to the message
        free(msg);
    }

    // Close the socket
    close(sockfd);
    exit(EXIT_SUCCESS);
}
