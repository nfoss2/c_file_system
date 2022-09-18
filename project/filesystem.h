#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include <vector>
#include <map>

struct FileTable
{
  FileTable(){rw = 0;};
  ~FileTable(){};
  char mode;
  char* filename;
  int fnameLen;
  int rw;
};

class FileSystem {
  DiskManager *myDM;
  PartitionManager *myPM;
  char myfileSystemName;
  int myfileSystemSize;
  int *locked;
  int lockID = 1;
  int fileID = 1;
  map<int, FileTable> opened;
  
  /* declare other private members here */

  public:
    FileSystem(DiskManager *dm, char fileSystemName);
    int createFile(char *filename, int fnameLen);
    int createDirectory(char *dirname, int dnameLen);
    int lockFile(char *filename, int fnameLen);
    int unlockFile(char *filename, int fnameLen, int lockId);
    int deleteFile(char *filename, int fnameLen);
    int deleteDirectory(char *dirname, int dnameLen);
    int openFile(char *filename, int fnameLen, char mode, int lockId);
    int closeFile(int fileDesc);
    int readFile(int fileDesc, char *data, int len);
    int writeFile(int fileDesc, char *data, int len);
    int appendFile(int fileDesc, char *data, int len);
    int seekFile(int fileDesc, int offset, int flag);
    int truncFile(int fileDesc, int offset, int flag);
    int renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2);
    int renameDirectory(char *dirname1, int dnameLen1, char *dirname2, int dnameLen2);
    int getAttributes(char *filename, int fnameLen, char attribute);
    int setAttributes(char *filename, int fnameLen, char attribute, int value);
    int getSubFile(int blocknum, char subDir);
    int getSubDir(int blocknum, char subDir);
    int getSpot(int blocknum, int pointer, char type);
    bool isDirEmpty(int blocknum);
    int spotFinder(char *dirContents, int &dirLoc);
    int findFile(char *filename, int fnameLen);
    int findDir(char *dirname, int dnameLen);
    int isFileName(char *filename, int fnameLen);
    int isOpened(char *fname, int fnameLen);
    vector<int> fileLocations(int blocknum);
    vector<int> addBlocks(int inodeLoc, vector<int> locations, int num);
    vector<int> removeBlocks(int inodeLoc, vector<int> locations, int num);
    bool doesFileSystemExist();
    bool compareChars(char* first, char* second);
    bool isSameName(char *filename1, int fnameLen1, char *filename2, int fnameLen2);

    /* declare other public members here */

};
#endif
