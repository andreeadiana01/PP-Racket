#include <stdlib.h>     
#include <stdio.h>
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookie_count, char* auth_token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);


    sprintf(line, "Host: %s", host);
    compute_message(message, line);


   if (cookies != NULL && cookie_count) {
        strcat(message, "Cookie: ");

        for (int i = 0; i < cookie_count - 1; ++i) {
            sprintf(line, "%s", cookies[i]);
            strcat(message, line);
            strcat(message, "; ");
        }
        sprintf(line, "%s", cookies[cookie_count - 1]);
        compute_message(message, line);
    }

    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    strcat(message, "\r\n");
    free(line);


    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookie_count, char* auth_token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);


    sprintf(line, "Host: %s", host);
    compute_message(message, line);


   if (cookies != NULL && cookie_count) {
        strcat(message, "Cookie: ");

        for (int i = 0; i < cookie_count - 1; ++i) {
            sprintf(line, "%s", cookies[i]);
            strcat(message, line);
            strcat(message, "; ");
        }
        sprintf(line, "%s", cookies[cookie_count - 1]);
        compute_message(message, line);
    }

    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }

    strcat(message, "\r\n");
    free(line);


    return message;
}

char *compute_post_request(char *host, char *url, char* content_type,
 char *body_data, char **cookies, int cookie_count, char* auth_token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));


    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    

    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(body_data));
    compute_message(message, line);


    if (cookies != NULL && cookie_count) {
        strcat(message, "Cookie: ");

        for (int i = 0; i < cookie_count - 1; ++i) {
            sprintf(line, "%s", cookies[i]);
            strcat(message, line);
            strcat(message, "; ");
        }
        sprintf(line, "%s", cookies[cookie_count - 1]);
        compute_message(message, line);
    }

    if (auth_token != NULL) {
        sprintf(line, "Authorization: Bearer %s", auth_token);
        compute_message(message, line);
    }
    strcat(message, "\r\n");


    compute_message(message, body_data);

    free(line);
    return message;
}