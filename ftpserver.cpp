/**
 * File:   ftpserver.cpp
 * @author: Jan Hendriks
 * @copyright 2010 University of Bonn, Germany
 *
 * Created on 1. Dezember 2009, 12:46
 *
 * Available commands:
 * - browse                             - lists all files and directories in the current server working directory
 * - ls <dir>                           - lists all files and directories in the specified directory
 * - download <file>                    - sends the binary data of the specified file over the connection the command was received
 * - upload <file>                      - stores all binary data that is received by the connection the command was received
 * - pwd                                - prints the current working directory on the server
 * - cd <dir>                           - changes the current working directory on the server to the specified one
 * - touch <file>                       - create an empty file of the given name
 * - mkdir <dir>                        - creates a directory of the specified name in the current server working dir
 * - rmdir <dir>                        - removes the specified directory and all contents at the server side
 * - delete <file>                      - deletes the specified file at the server side
 * - getparentdir                       - returns the parent directory of the current working dir
 * - getsize <file>                     - returns the size in bytes of the specified file
 * - getsize <dir>                      - returns the content count, the number of subdirectories and files in the specified directory
 * - getaccessright <file|dir>          - returns the numeric unix permission for a specified file or directory
 * - getlastmodificationtime <file|dir> - returns the time of the last modification for a specified file or directory
 * - getowner <file|dir>                - returns the file or directory owner
 * - getgroup <file|dir>                - returns the file or directory group
 * - bye                                - terminates the connection the command is received on
 * - quit                               - same as disconnect
 *
 */

/**
 * @TODO: Find correct FTP *SERVER* commands, see. http://www.nsftools.com/tips/RawFTP.htm and http://www.ipswitch.com/support/ws_ftp-server/guide/v5/A_FTPref3.html
 */

#include "ftpserver.h"

/**
 * This is the main program entry point
 * usage:
 * ftpserver <server root directory> <port to listen on for incoming connections> <telnet mode for use with telnet client, default = false>
 */
int main(int argc, char** argv) {
    unsigned short commandOffset = 1; // For telnet, we need 3 because of the enter control sequence at the end of command (+2 characters)
    unsigned int port = 4242; // Port to listen on (>1024 for no root permissions required)
    std::string dir = "./"; // Default dir
    if (argc < 2) {
        std::cout << "Usage: ftpserver <dir> <port> [telnetmode=no], using default dir '" << dir << "' , port " << port << std::endl;
    } else {
        switch (argc) {
            case 4:
                commandOffset = 3; // If any 3rd parameter is given, the server is started for use with telnet as client
            case 3:
                port = atoi(argv[2]); // Cast str to int, set port
            case 2:
                fileoperator* db = new fileoperator(dir);
                // Test if dir exists
                if (db->dirCanBeOpenend(argv[1])) {
                    dir = argv[1]; // set default server directory
                    db->changeDir(dir, false); // Assume the server side is allowed to change any directory as server root (thus the false for no strict mode)
                } else {
                    std::cout << "Invalid path specified ('" << argv[1] << "'), falling back to '" << dir << "'" << std::endl;
                }
                break;
        }
    }

    servercore* myServer = new servercore(port, dir, commandOffset);

    /// @TODO: some sort of server shutdown command??
    delete myServer; // Close connection, for the good tone

    return (EXIT_SUCCESS);
}
