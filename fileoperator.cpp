#include "fileoperator.h"

// Constructor, gets the current server root dir as a parameter
fileoperator::fileoperator(std::string dir) {
    this->completePath.push_front(dir); // Insert the root dir at the beginning
}

// Destructor, let the cleaner rage
fileoperator::~fileoperator() {
    this->closeWriteFile(); // Close the file (if any is open)
    this->completePath.clear();
}

// Relative directories only, strict
bool fileoperator::changeDir(std::string newPath, bool strict) {
    if (strict) // When using strict mode, the function only allows one subdirectory and not several subdirectories, e.g. like sub/subsub/dir/ ...
        getValidDir(newPath); // check error cases, e.g. newPath = '..//' , '/home/user/' , 'subdir' (without trailing slash), etc...
    // If change to a higher directory is requested
    if ( (newPath.compare("..") == 0) || (newPath.compare("../") == 0) ) {
        // If we are already in the server root dir, prohibit the change to a higher directory
        if (this->completePath.size() <= 1) {
            std::cerr << "Error: Change beyond server root requested (prohibited)!" << std::endl;
            return (EXIT_FAILURE); // 1
        } else { // change is permitted, now do it!
            this->completePath.pop_back(); // Remove the last dir we were in and return to the lower one
            return (EXIT_SUCCESS); // 0
        }
    }
    // The change is the local directory !?
    if ( (newPath.compare(".") == 0) || (newPath.compare("./") == 0)) {
        std::cout << "Change to same dir requested (nothing done)!" << std::endl;
        return (EXIT_SUCCESS); // 0 (?)
    }
// std::cout << "dir " << this->getCurrentWorkingDir().append(newPath) << std::endl;
    // Normal (sub-)directory given
    if (this->dirCanBeOpenend(this->getCurrentWorkingDir().append(newPath))) {
        this->completePath.push_back(newPath); // Add the new working dir to our path list
        return (EXIT_SUCCESS); // 0
    } else {
        return (EXIT_FAILURE); // 1
    }
}

// check error cases, e.g. newPath = '..//' , '/home/user/' , 'subdir' (without trailing slash), etc... and return a clean, valid string in the form 'subdir/'
void fileoperator::getValidDir(std::string &dirName) {
    std::string slash = "/";
    size_t foundSlash = 0;
    while ( (foundSlash = dirName.find_first_of(slash),(foundSlash)) != std::string::npos) {
//        std::cout << " / @ " << foundSlash << std::endl;
        dirName.erase(foundSlash++,1); // Remove all slashs
    }
    dirName.append(slash); // Trailing slash is good and required, append it
}

//filename ../../e.txt -> e.txt , prohibit deletion out of valid environment
void fileoperator::getValidFile(std::string &fileName) {
    std::string slash = "/";
    size_t foundSlash = 0;
    while ( (foundSlash = fileName.find_first_of(slash),(foundSlash)) != std::string::npos) {
//        std::cout << " / @ " << foundSlash << std::endl;
        fileName.erase(foundSlash++,1); // Remove all slashs
    }
}

// Strips out the string for the server root, e.g. <root>/data/file -> data/file
void fileoperator::stripServerRootString(std::string &dirOrFileName) {
    size_t foundRootString = 0;
    if ((dirOrFileName.find_first_of(SERVERROOTPATHSTRING) ) == foundRootString ) {
        int rootStringLength = ((std::string)SERVERROOTPATHSTRING).length();
        dirOrFileName = dirOrFileName.substr( rootStringLength, dirOrFileName.length() - rootStringLength);
    }
}

// Creates a directory with the specified name in the current working directory
bool fileoperator::createDirectory(std::string &dirName, bool strict) {
    if (strict) // When using strict mode, the function only allows one subdirectory and not several subdirectories, e.g. like sub/subsub/dir/ ...
        getValidDir(dirName); // check error cases, e.g. newPath = '..//' , '/home/user/' , 'subdir' (without trailing slash), etc...
    // Prohibit deletion of dir ../ beyond server root
    if ((dirName.compare("../") == 0) && (this->completePath.size() < 2)) {
        dirName = "./";
        std::cerr << "Error: Deletion of dir beyond server root requested (prohibited)!" << std::endl;
        return true;
    }

    umask(0);
    return ((mkdir(this->getCurrentWorkingDir().append(dirName).c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}

// Read a block from the open file
char* fileoperator::readFileBlock(unsigned long &sizeInBytes) {
    // get length of file
    this->currentOpenReadFile.seekg(0, std::ios::end);
    std::ifstream::pos_type size = this->currentOpenReadFile.tellg();
    sizeInBytes = (unsigned long)size;

    this->currentOpenReadFile.seekg (0, std::ios::beg);
    // allocate memory
    char* memblock = new char [size];
    // read data as a block
    this->currentOpenReadFile.read (memblock,size);

    // delete[] memblock;
    std::cout << "Reading " << size << " Bytes" << std::endl;
    this->currentOpenReadFile.close();
    return memblock;
}

/// @WARNING, @KLUDGE: Concurrent file access not catched
int fileoperator::readFile(std::string fileName) {
    stripServerRootString(fileName);
    this->currentOpenReadFile.open(fileName.c_str(), std::ios::in|std::ios::binary); // modes for binary file  |std::ios::ate
    if (this->currentOpenReadFile.fail()) {
        std::cout << "Reading file '" << fileName << "' failed!" << std::endl; //  strerror(errno) <<
        return (EXIT_FAILURE);
    }
    if (this->currentOpenReadFile.is_open()) {
        return (EXIT_SUCCESS);
    }
    std::cerr << "Unable to open file '" << fileName << " '" << std::endl; // << strerror(errno)
    return (EXIT_FAILURE);
}

int fileoperator::beginWriteFile(std::string fileName) {
    stripServerRootString(fileName);
    this->currentOpenFile.open(fileName.c_str(), std::ios::out|std::ios::binary|std::ios::app); // output file
    if(!this->currentOpenFile) {
        std::cerr << "Cannot open output file '" << fileName << "'" << std::endl;
        return (EXIT_FAILURE);
    }
    std::cout << "Beginning writing to file '" << fileName << "'" << std::endl;
    return (EXIT_SUCCESS);
}

/// @WARNING, @KLUDGE: Concurrent file access not catched
int fileoperator::writeFileBlock(std::string content) {
    if(!this->currentOpenFile) {
        std::cerr << "Cannot write to output file" << std::endl;
        return (EXIT_FAILURE);
    }
    std::cout << "Appending to file" << std::endl;
    (this->currentOpenFile) << content;
    return (EXIT_SUCCESS);
}

// File is closed when disconnecting
int fileoperator::closeWriteFile() {
    if (this->currentOpenFile.is_open()) {
        std::cout << "Closing open file" << std::endl;
        this->currentOpenFile.close();
    }
}

int fileoperator::writeFileAtOnce(std::string fileName, char* content) {
    stripServerRootString(fileName);
    std::ofstream myFile(fileName.c_str(), std::ios::out|std::ios::binary); // output file |std::ios::app
    if(!myFile) {
        std::cerr << "Cannot open output file '" << fileName << "'" << std::endl;
        return (EXIT_FAILURE);
    }
    myFile << content;
    myFile.close();
    return (EXIT_SUCCESS);
}

// Same as unix touch command
// Avoid touch ../file beyond server root!
bool fileoperator::createFile(std::string &fileName, bool strict) {
    if (strict)
        this->getValidFile(fileName); // Avoid touch ../file beyond server root!
    try {
        std::ofstream fileout;
        fileout.open(this->getCurrentWorkingDir().append(fileName).c_str(), std::ios::out|std::ios::binary|std::ios::app);
        fileout.close();
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

/*
 * Deletes a file on the server given by its name
 *
 * @param fileName the name of the file to delete
 */
// Avoid rm ../file beyond server root!
bool fileoperator::deleteFile(std::string fileName, bool strict) {
    if (strict)
        this->getValidFile(fileName); // Avoid rm ../file beyond server root!
    if (remove(this->getCurrentWorkingDir().append(fileName).c_str()) != 0 ) {
        std::cerr << "Error deleting file '" << fileName << "'" << std::endl;
        return (EXIT_FAILURE);
    } else {
        std::cout << "File '" << fileName << "' deleted" << std::endl;
        this->deletedFiles.push_back(fileName);
        return (EXIT_SUCCESS);
    }
}

/*
 * Deletes a directory on the server given by its name
 * recursive, since a directory can only be deleted if it is empty
 * 
 * @param dirName the name of the directory to delete
 * @param cancel should be initially false, determines when to break
 * @return if error occurred (error = true, success = false)
 */
bool fileoperator::deleteDirectory(std::string dirName, bool cancel, std::string pathToDir) {
    // If error was encountered in a previous recursion abort deletion process
    if (cancel) {
        return true;
    }
    getValidDir(dirName);

    std::vector<std::string>* directories = new std::vector<std::string>();
    std::vector<std::string>::iterator dirIterator;
    std::vector<std::string>* files = new std::vector<std::string>();
    std::vector<std::string>::iterator fileIterator;
    
    pathToDir.append(dirName);

// std::cout << "Browse " << pathToDir << std::endl;
    this->browse(pathToDir,*directories,*files,false);

    // Now walk over all files in the current directory and delete them
    fileIterator = files->begin();
    while(fileIterator != files->end() ) {
// std::cout << "Removing file " << pathToDir + (*fileIterator) << std::endl;
        cancel = (this->deleteFile(pathToDir + (*fileIterator++), false) || cancel);
    }

    // Now delete all subdirectories in the current directory
    dirIterator = directories->begin();
    while( dirIterator != directories->end() ) {
        // If not ./ and ../
        if ( ((*(dirIterator)).compare(".") == 0) || ((*(dirIterator)).compare("..") == 0) ) {
            dirIterator++;
            continue;
        }
// std::cout << "Recursive Call rmdir("<< (*dirIterator).append("/") << "," << pathToDir << ")" << std::endl;
        // Only one error must occur and the cancel becomes true and aborts the deletions
        cancel = (this->deleteDirectory((*(dirIterator++)).append("/"), cancel, pathToDir) || cancel);
    }

    // If there are no subdirectories and files (which should not be there anymore since we deleted them previously), delete the directory
    if ( (pathToDir.compare(".") != 0) && (pathToDir.compare("..") != 0) ) {
        if ( rmdir(this->getCurrentWorkingDir().append(pathToDir).c_str()) < 0 ) { // The actual delete command
            std::cerr << "Failed deleting directory '" << this->getCurrentWorkingDir(false).append(pathToDir) << "'" << std::endl; // 0 == success, -1 == error (errno-> EACCES access denies, ENOENT path not found)
        } else {
            std::cout << "Directory '" << pathToDir << "' deleted" << std::endl;
            this->deletedDirectories.push_back(pathToDir);
        }
    }
    return cancel; // false = no error, true = error
}

//struct stat {
//  dev_t  st_dev     /* (P) Device, auf dem die Datei liegt */
//  ushort st_ino     /* (P) i-node-Nummer */
//  ushort st_mode    /* (P) Dateityp  */
//  short  st_nlink   /* (P) Anzahl der Links der Datei  */
//  ushort st_uid     /* (P) Eigentuemer-User-ID (uid)  */
//  ushort st_gid     /* (P) Gruppen-ID (gid)  */
//  dev_t  st_rdev    /* Major- und Minornumber, falls Device */
//  off_t  st_size    /* (P) Größe in Byte  */
//  time_t st_atime   /* (P) Zeitpunkt letzter Zugriffs  */
//  time_t st_mtime   /* (P) Zeitpunkt letzte Änderung  */
//  time_t st_ctime   /* (P) Zeitpunkt letzte Statusänderung */
//};
//
//struct tm {
//  int tm_sec; /* seconds */
//  int tm_min; /* minutes */
//  int tm_hour; /* hours */
//  int tm_mday; /* day of the month */
//  int tm_mon; /* month */
//  int tm_year; /* year */
//  int tm_wday; /* day of the week */
//  int tm_yday; /* day in the year */
//  int tm_isdst; /* daylight saving time */
//};
/*
 *
 * @returns vector<string> result(0) = AccessRights, (1) = Group, (2) = Owner, (3) = LastModificationTime, (4) = Size
 */
std::vector<std::string> fileoperator::getStats(std::string fileName, struct stat Status) {
    std::vector<std::string> result;
    // Check if existent, accessible
    if (stat(this->getCurrentWorkingDir().append(fileName).c_str(), &Status) != 0) {
        std::cerr << "Error when issuing stat() on '" << fileName << "'!" << std::endl;
        return result; // Bail out with no results
    }
    struct passwd *pwd;
    struct group *grp;
    std::string tempRes;

/*
S_IRUSR	00400	owner has read permission
S_IWUSR	00200	owner has write permission
S_IXUSR	00100	owner has execute permission
S_IRGRP	00040	group has read permission
S_IWGRP	00020	group has write permission
S_IXGRP	00010	group has execute permission
S_IROTH	00004	others have read permission
S_IWOTH	00002	others have write permisson
S_IXOTH	00001	others have execute permission
 */
    // User flags
    int usr_r = ( Status.st_mode & S_IRUSR ) ? 1 : 0;
    int usr_w = ( Status.st_mode & S_IWUSR ) ? 1 : 0;
    int usr_x = ( Status.st_mode & S_IXUSR ) ? 1 : 0;
    int usr_t = ( usr_r << 2 ) | ( usr_w << 1 ) | usr_x;
    // Group flags
    int grp_r = ( Status.st_mode & S_IRGRP ) ? 1 : 0;
    int grp_w = ( Status.st_mode & S_IWGRP ) ? 1 : 0;
    int grp_x = ( Status.st_mode & S_IXGRP ) ? 1 : 0;
    int grp_t = ( grp_r << 2 ) | ( grp_w << 1 ) | grp_x;
    // Other flags
    int oth_r = ( Status.st_mode & S_IRGRP ) ? 1 : 0;
    int oth_w = ( Status.st_mode & S_IWGRP ) ? 1 : 0;
    int oth_x = ( Status.st_mode & S_IXGRP ) ? 1 : 0;
    int oth_t = ( oth_r << 2 ) | ( oth_w << 1 ) | oth_x;

    IntToString(usr_t*100 + grp_t*10 + oth_t, tempRes);
    result.push_back(tempRes); // AccessRights, Unix file/dir permissions
    // Try to read out the group name if available, else return group id
    if ((grp = getgrgid(Status.st_gid)) != NULL) {
        result.push_back((std::string)grp->gr_name); // Group name
    } else {
        IntToString(Status.st_gid, tempRes);
        result.push_back(tempRes); // Group id
    }
    // Try to read out owner owner name if available, else return owner id
    if ((pwd = getpwuid(Status.st_uid)) != NULL) {
        result.push_back((std::string)pwd->pw_name); // Owner name
    } else {
        IntToString(Status.st_uid, tempRes);
        result.push_back(tempRes); // Owner id
    }
    // The time of last modification
    struct tm *date;
    date = localtime(&Status.st_mtime); // LastModificationTime
    char buffer[20]; // 19 should suffices actually: 'DD.MM.YYYY HH:mm:SS'
    sprintf (buffer, "%d.%d.%d %d:%d:%d", date->tm_mday, ((date->tm_mon)+1), ((date->tm_year)+1900), date->tm_hour, date->tm_min, date->tm_sec);
    result.push_back((std::string)buffer); // LastModificationTime
    // If file, get the filesize, else (if dir) get the count of files and subdirectories
    unsigned long tempSize = ((S_ISDIR(Status.st_mode)) ? this->getDirSize(fileName) : (intmax_t)Status.st_size);
    IntToString(tempSize, tempRes);
    result.push_back(tempRes); // Size (Bytes)
    // vector<string> AccessRight | Group | Owner | LastModificationTime | Size
    return result;
}

void fileoperator::IntToString(int i, std::string& res) {
    std::ostringstream temp;
    temp << i;
    res = temp.str();
}

/*
 * Returns the directory the given file resides in (only directory and no full path, required?)
 */
std::string fileoperator::getParentDir() {
    std::list<std::string>::reverse_iterator lastDirs;
    unsigned int i = 0;
    // Walk to second-last entry for parent dir (from end of list on)
    for (lastDirs = this->completePath.rbegin(); ((lastDirs != this->completePath.rend()) && (++i <= 2)); ++lastDirs) {
        if (i == 2) { // Yes this is the second-last entry (could also be that only one dir exists because we are in the server root dir
            return ((++lastDirs == this->completePath.rend()) ? SERVERROOTPATHSTRING : (*(--lastDirs))); // Avoid divulging the real server root path
        }
    }
    return SERVERROOTPATHSTRING; // Otherwise, the server root dir is the parent directory (e.g. "./")
//    return this->completePath.front(); // Otherwise, the server root dir is the parent directory (e.g. "./")
}

/*
 * Returns the count of the subdirectories+file entries in a specified directory
 *
 * @param dirName the directory
 */
unsigned long fileoperator::getDirSize(std::string dirName) {
    getValidDir(dirName);
    std::vector<std::string> directories;
    std::vector<std::string> files;
    this->browse(dirName,directories,files);
    // directories -2 because of "." and ".." directories, which are not be considered
    return ((directories.size()-2) + files.size());
}

// Returns the path to the current working dir starting from the server root dir
std::string fileoperator::getCurrentWorkingDir(bool showRootPath) {
    std::string fullpath = "";
    std::list<std::string>::iterator singleDir;
    for (singleDir = this->completePath.begin(); singleDir != this->completePath.end(); ++singleDir) {
        // If first entry (= server root)
        if ( singleDir == this->completePath.begin() ) {
            // If the real root path should be hidden to client
            if (!showRootPath) {
                fullpath.append(SERVERROOTPATHSTRING);                
            } else { // If server root path should be shown (server-internal)
                // The lowest path already is chdir()'d into, so we do not need it
//                fullpath.append("./");
//                fullpath.append(*(singleDir));
            }
        } else {
            fullpath.append(*(singleDir));
        }
    }
//    std::cout << "Path '" << fullpath << "'" << std::endl;
    return fullpath;
}

// Lists all files and directories in the specified directory and returns them in a string vector
void fileoperator::browse(std::string dir, std::vector<std::string> &directories, std::vector<std::string> &files, bool strict) {
    if (strict) {// When using strict mode, the function only allows one subdirectory and not several subdirectories, e.g. like sub/subsub/dir/ ...
        getValidDir(dir);
        if ((dir.compare("../") == 0) && (this->completePath.size() < 2)) { // Prohibit ../ beyond server root
            dir = "./";
            std::cerr << "Error: Change beyond server root requested (prohibited)!" << std::endl;
        }
    }
    if (dir.compare("./") != 0) {
        dir = this->getCurrentWorkingDir().append(dir);
    } else {
        dir = this->getCurrentWorkingDir(true);
//        std::cout << "Yes" << std::endl;
    }
    std::cout << "Browsing '" << dir << "'" << std::endl;
    DIR *dp;
    struct dirent *dirp;
    if (this->dirCanBeOpenend(dir)) {
        try {
            dp = opendir(dir.c_str());
            while ((dirp = readdir(dp)) != NULL) {
                // Prohibit ../ only if the current working dir is already the server root directory
                if (((std::string)dirp->d_name).compare("..") == 0 && this->completePath.size() < 2)
                    continue;
//                if( ((std::string)dirp->d_name).compare(".") == 0 || ((std::string)dirp->d_name).compare("..") == 0 ) continue;
                /// @WARNING, @KLUDGE: symbolic links to directories not supported ?!
                if( dirp->d_type == DT_DIR ) {
                    directories.push_back(std::string(dirp->d_name));
                } else { // Otherwise it must be a file (no special treatment for devs, nodes, etc.)
                    files.push_back(std::string(dirp->d_name));
                }
            }
            closedir(dp);
        } catch (std::exception e) {
            std::cerr << "Error (" << e.what() << ") opening '" << dir << "'" << std::endl;
        }
    } else {
        std::cout << "Error: Directory '" << dir << "' could not be opened!" << std::endl;
    }
}

// Returns true if dir can be opened
bool fileoperator::dirCanBeOpenend(std::string dir) {
    DIR *dp;
    bool canBeOpened = false;
    canBeOpened = ((dp = opendir(dir.c_str())) != NULL); // Anything else than NULL is good
    closedir(dp);
    return canBeOpened;
}

// Clears out the list of deleted files
void fileoperator::clearListOfDeletedFiles() {
    this->deletedFiles.clear();
}

// Clears out the list of deleted directories
void fileoperator::clearListOfDeletedDirectories() {
    this->deletedDirectories.clear();
}

// Returns the list of successfully deleted files
std::vector<std::string> fileoperator::getListOfDeletedFiles() {
    return this->deletedFiles;
}

// Returns the list of successfully deleted directories
std::vector<std::string> fileoperator::getListOfDeletedDirectories() {
    return this->deletedDirectories;
}

// Check if the specified directory is below the server root directory and return true if so
bool fileoperator::dirIsBelowServerRoot(std::string dirName) {
    this->getValidDir(dirName);
    return ((dirName.compare("../") == 0) && (this->completePath.size() < 2));
}
