#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>     
#include <string.h>     
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h>      
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define PORT 8080
#define SERVER_IP "3.8.116.10"


void parse_json_value(char* string, JSON_Value** crt_response) {
    json_value_free(*crt_response);
    *crt_response = json_parse_string(strchr(string, '{'));
}

void parse_json_array(char* string, JSON_Value** crt_response) {
    json_value_free(*crt_response);
    *crt_response = json_parse_string(strchr(string, '['));
}

int main(int argc, char *argv[]){

    char *message;
    char *response;
    int sockfd;

    int done = 0;
    fd_set read_fds, tmp_fds;
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);
    FD_SET(sockfd, &read_fds);
    FD_SET(0, &read_fds);
    char buffer[BUFLEN];
	char *cookie = calloc(BUFLEN, sizeof(char));
    char *token = calloc(BUFLEN, sizeof(char));


    while (done == 0) {
    	tmp_fds = read_fds;
		if (FD_ISSET(0, &tmp_fds)) {
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			if (strcmp(buffer, "register\n") == 0) {
    			sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);

				char *username = calloc(BUFLEN, sizeof(char));
				char *password = calloc(BUFLEN, sizeof(char));

				fprintf(stdout, "username = ");
				fgets(username, BUFLEN - 1, stdin);
				username =(char*) realloc(username, strlen(username) + 1);

				fprintf(stdout, "password = ");
				fgets(password, BUFLEN - 1, stdin);
				password =(char*) realloc(password, strlen(password) + 1);

                username[strlen(username) - 1] = '\0';
                password[strlen(password) - 1] = '\0';

    			JSON_Value *root_value = json_value_init_object();
    			JSON_Object *root_object = json_value_get_object(root_value);
    			char *serialized_string = NULL;
    			json_object_set_string(root_object, "username", username);
    			json_object_set_string(root_object, "password", password);
    			serialized_string = json_serialize_to_string_pretty(root_value);

				message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/register",
                                            "application/json", serialized_string, NULL, 0, NULL);

				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    		
    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"Account created. Welcome!\n");
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);
			} else if (strcmp(buffer, "login\n") == 0) {
				sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);

				char *username = calloc(BUFLEN, sizeof(char));
				char *password = calloc(BUFLEN, sizeof(char));

				fprintf(stdout, "username = ");
				fgets(username, BUFLEN - 1, stdin);
				username =(char*) realloc(username, strlen(username));

				fprintf(stdout, "password = ");
				fgets(password, BUFLEN - 1, stdin);
				password =(char*) realloc(password, strlen(password));

                username[strlen(username) - 1] = '\0';
                password[strlen(password) - 1] = '\0';

    			JSON_Value *root_value = json_value_init_object();
    			JSON_Object *root_object = json_value_get_object(root_value);
    			char *serialized_string = NULL;
    			json_object_set_string(root_object, "username", username);
    			json_object_set_string(root_object, "password", password);
    			serialized_string = json_serialize_to_string_pretty(root_value);

				message = compute_post_request(SERVER_IP, "/api/v1/tema/auth/login",
                                "application/json", serialized_string, NULL, 0, NULL);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);
    			
    			char* cookie_end = calloc(BUFLEN, sizeof(char));
                char* cookie_start = calloc(BUFLEN, sizeof(char));

                cookie_start = strstr(response, "Set-Cookie");
                
                if (cookie_start != NULL) {
                    cookie_end = strchr(cookie_start, ';');
                    strncpy(cookie, cookie_start + 12, cookie_end - cookie_start - 12);
                }

    			JSON_Value *err_response = NULL;
    			parse_json_value(response, &err_response);
    			JSON_Object* crt_obj = json_object(err_response);
 				char* data; 
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"You are now logged in. Enjoy your experience!\n");
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);

			} else if (strcmp(buffer, "enter_library\n") == 0) {
    			sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);	

				message = compute_get_request(SERVER_IP, "/api/v1/tema/library/access", NULL,
                                             &cookie, 1, NULL);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);

    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
                token = (char*)json_object_get_string(crt_obj, "token");

                if (token == NULL) {
                    fprintf(stderr,"No token!\n");
                }

    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"Welcome to the library!\n");
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);
			} else if (strcmp(buffer, "get_books\n") == 0) {
				sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);	

				message = compute_get_request(SERVER_IP, "/api/v1/tema/library/books", NULL,
                                                 NULL, 0, token);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);

    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
                    JSON_Value *resp = NULL;
                    parse_json_array(response, &resp);
                    JSON_Array *books;
                    JSON_Object* book;
                    books = json_value_get_array(resp);
                    if (json_array_get_count(books) <= 0) {
                        fprintf(stdout, "You have no books at the moment.\n");
                    } else {
                    fprintf(stdout,"Here are your books\n");
                    for (int i = 0; i < json_array_get_count(books); i++) {
                        book = json_array_get_object(books, i);
                        fprintf(stdout, "Book ID: %.0lf\n", json_object_get_number(book, "id"));
                        fprintf(stdout, "Book Title: %s\n", json_object_get_string(book, "title"));
                        }
                    }
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);
			} else if (strcmp(buffer, "get_book\n") == 0) {
				sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);

                char *id = calloc(BUFLEN, sizeof(char));
                fprintf(stdout, "id = ");
                fgets(id, BUFLEN - 1, stdin);
                id =(char*) realloc(id, strlen(id));	
                char route[] = "/api/v1/tema/library/books/";
                strcat(route, id);
                route[strlen(route) - 1] = '\0';

				message = compute_get_request(SERVER_IP, route, NULL, NULL, 0, token);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);

    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"Here is your book!\n");
                    char *title;
                    title = (char *)json_object_get_string(crt_obj, "title");
                    fprintf(stdout,"Title: %s\n", title);
                    char *author;
                    author = (char *)json_object_get_string(crt_obj, "author");
                    fprintf(stdout,"Author: %s\n", author);
                    char *genre;
                    genre = (char *)json_object_get_string(crt_obj, "genre");
                    fprintf(stdout,"Genre: %s\n", genre);
                    int page_count;
                    page_count = (int )json_object_get_number(crt_obj, "page_count");
                    fprintf(stdout,"Page Count: %d\n", page_count);

    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);
			} else if (strcmp(buffer, "add_book\n") == 0) {
				sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);

				char *title = calloc(BUFLEN, sizeof(char));
				char *author = calloc(BUFLEN, sizeof(char));
				char *genre = calloc(BUFLEN, sizeof(char));
				char *page_count = calloc(BUFLEN, sizeof(char));
				char *publisher = calloc(BUFLEN, sizeof(char));

				fprintf(stdout, "title = ");
				fgets(title, BUFLEN - 1, stdin);
				title =(char*) realloc(title, strlen(title));

				fprintf(stdout, "author = ");
				fgets(author, BUFLEN - 1, stdin);
				author =(char*) realloc(author, strlen(author));

				fprintf(stdout, "genre = ");
				fgets(genre, BUFLEN - 1, stdin);
				genre =(char*) realloc(genre, strlen(genre));

				fprintf(stdout, "page_count = ");
				fgets(page_count, BUFLEN - 1, stdin);
				page_count =(char*) realloc(page_count, strlen(page_count));

				fprintf(stdout, "publisher = ");
				fgets(publisher, BUFLEN - 1, stdin);
				publisher =(char*) realloc(publisher, strlen(publisher));

                title[strlen(title) - 1] = '\0';
                author[strlen(author) - 1] = '\0';
                genre[strlen(genre) - 1] = '\0';
                page_count[strlen(page_count) - 1] = '\0';
                publisher[strlen(publisher) - 1] = '\0';

    			JSON_Value *root_value = json_value_init_object();
    			JSON_Object *root_object = json_value_get_object(root_value);
    			char *serialized_string = NULL;
    			json_object_set_string(root_object, "title", title);
    			json_object_set_string(root_object, "author", author);
    			json_object_set_string(root_object, "genre", genre);
    			json_object_set_string(root_object, "page_count", page_count);
    			json_object_set_string(root_object, "publisher", publisher);
    			serialized_string = json_serialize_to_string_pretty(root_value);

				message = compute_post_request(SERVER_IP, "/api/v1/tema/library/books",
                                        "application/json", serialized_string, NULL, 0, token);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);

    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"Book added.\n");
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);

			} else if (strcmp(buffer, "delete_book\n") == 0) {
                sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0); 

                char *id = calloc(BUFLEN, sizeof(char));
                fprintf(stdout, "id = ");
                fgets(id, BUFLEN - 1, stdin);
                id =(char*) realloc(id, strlen(id));    

                char route[] = "/api/v1/tema/library/books/";
                strcat(route, id);
                route[strlen(route) - 1] = '\0';

                message = compute_delete_request(SERVER_IP, route, NULL, NULL, 0, token);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);

                JSON_Value *crt_response = NULL;
                char* data; 
                parse_json_value(response, &crt_response);
                JSON_Object* crt_obj = json_object(crt_response);
                data = (char *)json_object_get_string(crt_obj, "error");

                if (data == NULL) {
                    fprintf(stdout,"Deleted book.\n");
                } else {
                    fprintf(stdout,"%s\n", data);
                }

                close_connection(sockfd);
			} else if (strcmp(buffer, "logout\n") == 0) {
    			sockfd = open_connection(SERVER_IP, PORT, AF_INET, SOCK_STREAM, 0);

				message = compute_get_request(SERVER_IP, "/api/v1/tema/auth/logout",
                                                 NULL, &cookie, 1, NULL);
				send_to_server(sockfd, message);
    			response = receive_from_server(sockfd);

    			JSON_Value *crt_response = NULL;
    			char* data; 
    			parse_json_value(response, &crt_response);
    			JSON_Object* crt_obj = json_object(crt_response);
    			data = (char *)json_object_get_string(crt_obj, "error");

    			if (data == NULL) {
      				fprintf(stdout,"Thank you for your time.Bye!\n");
    			} else {
    				fprintf(stdout,"%s\n", data);
    			}

    			close_connection(sockfd);

			} else if (strcmp(buffer, "exit\n") == 0) {
				done = 1;
				break;
			} else {
                fprintf(stdout, "Please input a valid command\n");
            }
		}
    }

    return 0;
}