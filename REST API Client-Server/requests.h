#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string (cookies and auth_token can be NULL if not needed)
char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookie_count, char* auth_token );
// computes and returns a POST request string (cookies and auth_token can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type,
                            char *body_data, char **cookies, int cookie_count, char* auth_token);
// computes and returns a DELETE request string (cookies and auth_token can be NULL if not needed)
char *compute_delete_request(char *host, char *url, char *query_params,
                            char **cookies, int cookie_count, char* auth_token );

#endif
