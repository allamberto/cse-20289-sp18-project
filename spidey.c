/* spidey: Simple HTTP Server */

#include "spidey.h"

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>

/* Global Variables */
char *Port	      = "9898";
char *MimeTypesPath   = "/etc/mime.types";
char *DefaultMimeType = "text/plain";
char *RootPath	      = "www";

/**
 * Display usage message and exit with specified status code.
 *
 * @param   progname    Program Name
 * @param   status      Exit status.
 */
void usage(const char *progname, int status) {
    fprintf(stderr, "Usage: %s [hcmMpr]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -h            Display help message\n");
    fprintf(stderr, "    -c mode       Single or Forking mode\n");
    fprintf(stderr, "    -m path       Path to mimetypes file\n");
    fprintf(stderr, "    -M mimetype   Default mimetype\n");
    fprintf(stderr, "    -p port       Port to listen on\n");
    fprintf(stderr, "    -r path       Root directory\n");
    exit(status);
}

/**
 * Parse command-line options.
 *
 * @param   argc        Number of arguments.
 * @param   argv        Array of argument strings.
 * @param   mode        Pointer to ServerMode variable.
 * @return  true if parsing was successful, false if there was an error.
 *
 * This should set the mode, MimeTypesPath, DefaultMimeType, Port, and RootPath
 * if specified.
 */
 bool parse_options(int argc, char *argv[], ServerMode *mode) {
     char *progname = argv[0];
     int argind = 1;
     while (argind < argc && strlen(argv[argind]) > 1 && argv[argind][0] == '-') {
         char *arg = argv[argind++];
         switch (arg[1]) {
             case 'h':
                 usage(progname, EXIT_SUCCESS);
                 break;
             case 'c':
                 arg = argv[argind++];
                 if (streq(arg, "SINGLE")){
                     *mode = SINGLE;
                 }
                 else if (streq(arg, "FORKING")){
                     *mode = FORKING;
                 }
                 else{
                     usage(progname, EXIT_FAILURE);
                 }
                 break;
             case 'm':
                 MimeTypesPath = argv[argind++];
                 break;
             case 'M':
                 DefaultMimeType = argv[argind++];
                 break;
             case 'p':
                 Port = argv[argind++];
                 break;
             case 'r':
                 RootPath = argv[argind++];
                 break;
             default:
                 usage(progname, EXIT_FAILURE);
                 break;
         }
     }

     return true;
 }

 /**
  * Parses command line options and starts appropriate server
  **/
 int main(int argc, char *argv[]) {
     ServerMode mode = SINGLE;

     /* Parse command line options */
     parse_options(argc, argv, &mode);

     /* Listen to server socket */
     int server_sock  = socket_listen(Port);
     if (server_sock < 0){
         fprintf(stderr, "socket_listen error: fd was -1");
         close(server_sock);
         return EXIT_FAILURE;
     }

     /* Determine real RootPath */
     RootPath = realpath(RootPath, NULL);
     //RootPath = "/afs/nd.edu/user39/alamber2/cse-20289-sp18-assignments/cse-20289-sp18-project/www/text/g.txt";
     log("Listening on port %s", Port);
     debug("RootPath        = %s", RootPath);
     debug("MimeTypesPath   = %s", MimeTypesPath);
     debug("DefaultMimeType = %s", DefaultMimeType);
     debug("ConcurrencyMode = %s", mode == 1 ? "Forking" : "Single");

     /* Start either forking or single HTTP server */
     switch(mode){
       case SINGLE:
        single_server(server_sock);
        break;
       case FORKING:
        forking_server(server_sock);
        break;
       case UNKNOWN:
        usage(argv[0], EXIT_FAILURE);
        break;
        default:
        single_server(server_sock);
     }

     free(RootPath);
     log("End of Test\n");
     return EXIT_SUCCESS;
 }

 /* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
