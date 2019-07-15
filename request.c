/* request.c: HTTP Request Functions */

#include "spidey.h"

#include <errno.h>
#include <string.h>

#include <unistd.h>

int parse_request_method(Request *r);
int parse_request_headers(Request *r);

/**
 * Accept request from server socket.
 *
 * @param   sfd         Server socket file descriptor.
 * @return  Newly allocated Request structure.
 *
 * This function does the following:
 *
 *  1. Allocates a request struct initialized to 0.
 *  2. Initializes the headers list in the request struct.
 *  3. Accepts a client connection from the server socket.
 *  4. Looks up the client information and stores it in the request struct.
 *  5. Opens the client socket stream for the request struct.
 *  6. Returns the request struct.
 *
 * The returned request struct must be deallocated using free_request.
 **/
Request * accept_request(int sfd) {
    Request *r;
    struct sockaddr raddr;
    socklen_t rlen = sizeof(struct sockaddr);

    // Allocate request struct (zeroed)
    r = calloc(1, sizeof(Request));
    if(!r){
        fprintf(stdout,"Could not allocate request struct: %s\n", strerror(errno));
    }

    //Initializes Headers List
    r->headers = calloc(1, sizeof(struct header));
    if (!r->headers) {
        fprintf(stdout, "Could not initialize Headers List: %s\n", strerror(errno));
    }

    // Accept a client
    r->fd = accept(sfd, &raddr, &rlen);
    if (r->fd < 0)  {
        fprintf(stdout, "Unable to accept client: %s\n",strerror(errno));
        goto fail;
    }

    // Lookup client information
    int data = getnameinfo(&raddr, rlen, r->host, sizeof(r->host), r->port, sizeof(r->port), NI_NAMEREQD);
    if(data != 0){
      fprintf(stderr, "Failed to getnameinfo: %s\n", gai_strerror(data));
      goto fail;
    }

    // Open socket stream
    FILE *file = fdopen(r->fd, "r+");
    if (!file) {
        fprintf(stdout, "Unable to open socket stream: %s\n", strerror(errno));
        close(r->fd);
        goto fail;
    }
    r->file = file;

    log("Accepted request from %s:%s", r->host, r->port);
    return r;

    fail:
      // Deallocate request struct
      free_request(r);
      return NULL;
    }

/**
 * Deallocate request struct.
 *
 * @param   r           Request structure.
 *
 * This function does the following:
 *
 *  1. Closes the request socket stream or file descriptor.
 *  2. Frees all allocated strings in request struct.
 *  3. Frees all of the headers (including any allocated fields).
 *  4. Frees request struct.
 **/
void free_request(Request *r) {
    if (!r) {
        return;
    }

    // Close socket or fd
    if(r->file)
      fclose(r->file);
    else
      close(r->fd);

    // Free allocated strings
    free(r->method);
    free(r->uri);
    free(r->path);
    free(r->query);

    // Free headers
    struct header *header;
    while(r->headers){
      header = r->headers;
      r->headers = r->headers->next;
      free(header->name);
      free(header->value);
      free(header);
    }

    //Free Request
    free(r);
    log("Free Request Complete");

}

/**
 * Parse HTTP Request.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * This function first parses the request method, any query, and then the
 * headers, returning 0 on success, and -1 on error.
 **/
int parse_request(Request *r) {

    // Parse HTTP Request Method
    if(parse_request_method(r) != 0){
      log("Can't parse request method\n");
      return -1;
    }

    // Parse HTTP Requet Headers
    if(parse_request_headers(r) != 0){
      log("Can't parse request headers\n");
      return -1;
    }

    return 0;
}

/**
 * Parse HTTP Request Method and URI.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Requests come in the form
 *
 *  <METHOD> <URI>[QUERY] HTTP/<VERSION>
 *
 * Examples:
 *
 *  GET / HTTP/1.1
 *  GET /cgi.script?q=foo HTTP/1.0
 *
 * This function extracts the method, uri, and query (if it exists).
 **/
int parse_request_method(Request *r) {
    char buffer[BUFSIZ];
    char *method;
    char *uriExtract;
    char *uri;
    char *query;

    // Read line from socket
   if(!fgets(buffer,BUFSIZ,r->file)){
     printf("Parse Request Method Failed\n");
     return EXIT_FAILURE;
   }

   method = strtok(buffer, WHITESPACE);
   if(!method){
     printf("Method is NULL\n");//message with no method
     return -1;
   }
   r->method = strdup(method); //allocate memory in struct

   uriExtract = strtok(NULL, WHITESPACE);
   if(!uriExtract){
     printf("Uri is NULL\n");
     return -1;
   }

   char *trackWhite;
   uri = strtok(uriExtract, "?");
   if(uri)
     query = strtok(NULL, WHITESPACE);
   else{
     trackWhite = skip_nonwhitespace(buffer);
     if(trackWhite)
       uriExtract = skip_whitespace(trackWhite);
     else
       return -1;

     uri = strtok(uriExtract, WHITESPACE);
     query = NULL;
   }

   r->uri = strdup(uri);
   if(query)
     r->query = strdup(query);
   else
     r->query = strdup("");


   debug("HTTP METHOD: %s", r->method);
   debug("HTTP URI:    %s", r->uri);
   debug("HTTP QUERY:  %s", r->query);
   return 0;
}

/**
 * Parse HTTP Request Headers.
 *
 * @param   r           Request structure.
 * @return  -1 on error and 0 on success.
 *
 * HTTP Headers come in the form:
 *
 *  <NAME>: <VALUE>
 *
 * Example:
 *
 *  Host: localhost:8888
 *  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:29.0) Gecko/20100101 Firefox/29.0
 *  Accept: text/html,application/xhtml+xml
 *  Accept-Language: en-US,en;q=0.5
 *  Accept-Encoding: gzip, deflate
 *  Connection: keep-alive
 *
 * This function parses the stream from the request socket using the following
 * pseudo-code:
 *
 *  while (buffer = read_from_socket() and buffer is not empty):
 *      name, value = buffer.split(':')
 *      header      = new Header(name, value)
 *      headers.append(header)
 **/
int parse_request_headers(Request *r) {
    struct header *curr;
    char buffer[BUFSIZ];
    char *name;
    char *value;

    // Parse headers from socket
    while(fgets(buffer, BUFSIZ, r->file) && strlen(buffer) > 2){
      chomp(buffer);
      value = strchr(buffer, ':');
      debug("Value: %s\n", value);
      if(!value){
        return -1;
      }
      value = value + 1;
      value = skip_whitespace(value);
      name = strtok(buffer, ":");
      debug("Name: %s\n", name);
      if (!name){
        return -1;
      }
      curr = (struct header*)malloc(sizeof(*curr));
      curr->value = strdup(value);
      curr->name = strdup(name);
      curr->next = r->headers;
      r->headers = curr;
    }

  if (!r->headers)
    goto fail;

    #ifndef NDEBUG
      for (struct header *header = r->headers; header != NULL; header = header->next) {
    	   debug("HTTP HEADER %s = %s", header->name, header->value);
       }
    #endif
    return 0;

  fail:
    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
