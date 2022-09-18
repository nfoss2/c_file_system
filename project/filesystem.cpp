#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "string.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include <math.h>
using namespace std;

FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{
  myfileSystemName = fileSystemName;
  myfileSystemSize = dm->getPartitionSize(fileSystemName);
  myPM = new PartitionManager(dm, fileSystemName, myfileSystemSize);
  if (!doesFileSystemExist())
  {
    // create root directory
    char rootBuffer[64];
    int pos = 0;
    for (int i = 0; i < 10; i++)
    {
      rootBuffer[pos] = '0'; //will hold filename
      myDM->intToChar(rootBuffer, 0, (pos + 1));
      rootBuffer[pos + 5] = '0'; //will hold f or d (file or dir)
      pos += 6;
    }                                    // filling in info (all empty right now)
    myDM->intToChar(rootBuffer, 0, pos); // very last 4, pointer to continuation
    myPM->writeDiskBlock(1, rootBuffer);
  }

  locked = new int[myfileSystemSize];
  for (int i = 0; i < myfileSystemSize; i++)
  {
    locked[i] = 0;
  }
}

/*
  This operation creates a new file whose name is pointed to by filename of size
  fnameLen characters. The createFile function returns
  -1 if the file already exists,
  -2 if there is not enough disk space,
  -3 if invalid filename,
  -4 if the file cannot be created for some other reason,
  and 0 if the file is created successfully.
*/
int FileSystem::createFile(char *filename, int fnameLen)
{
  char iNodeFile[64];
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;
  if (myPM->getFreeDiskBlock() == -1) return -2; //no space
  if (fnameLen < 2 || fnameLen % 2 != 0) return -3; //len doesn't make sense
  if (isFileName(filename, fnameLen) == -1) return -3; //not a real name
  pointer = findFile(filename, fnameLen);
  if (pointer != 0) return -1; //file already exists
  if (fnameLen > 2) { //getting parent dir
    char temp[fnameLen-2];
    for (int i = 0; i < (fnameLen-2);i++) {
      temp[i] = filename[i];
    }
    currentDir = findDir(temp, fnameLen-2);
    if (currentDir == 0) return -4; //can't find parent dir...
  }

  // now we know the parent dir (currentDir)
  // create file
  // create iNodeFile
  iNodeFile[0] = filename[fnameLen - 1]; // file name
  iNodeFile[1] = 'f';                    // file 'f' not directory 'd'
  myDM->intToChar(iNodeFile, 0, 2);      // current file size is 0
  myDM->intToChar(iNodeFile, 0, 6);      // direct address 1
  myDM->intToChar(iNodeFile, 0, 10);     // direct address 2
  myDM->intToChar(iNodeFile, 0, 14);     // direct address 3
  myDM->intToChar(iNodeFile, 0, 18);     // indirect address
  iNodeFile[22] = '0';                   //no attribute yet
  iNodeFile[23] = '0';                   //no attribute yet
  //just for looks:
  for (int i = 24; i < 64; i++) iNodeFile[i] = '#';

  int location = myPM->getFreeDiskBlock();   // location for file inode
  myPM->writeDiskBlock(location, iNodeFile); // write out the file inode

  // change INodeDir -> currentDir to point to INodeFile
  myPM->readDiskBlock(currentDir, buffer);
  // we need to figure out the next open place to put a pointer in this directory
  int dirLoc = currentDir;
  // cout << "dirLoc: " << dirLoc << endl;
  // cout << "before spotFinder: ";
  // for (int i = 0; i < 64; i++) {
  //   cout << buffer[i];
  // } cout << endl;
  int spot = spotFinder(buffer, dirLoc);
  if (spot == -1)
  {
    // there isn't room to expand the directory, problem!!
    myPM->returnDiskBlock(location); // erase the file inode
    return -2;
  }
  // cout << "dirLoc: " << dirLoc << endl;
  // cout << "after spotFinder: ";
  // for (int i = 0; i < 64; i++) {
  //   cout << buffer[i];
  // } cout << endl;
  currentDir = dirLoc; // in case it changed
  myDM->intToChar(buffer, location, spot);
  buffer[spot - 1] = filename[fnameLen - 1];
  buffer[spot + 4] = 'f';
  myPM->writeDiskBlock(currentDir, buffer);

  return 0;
}

/*
  This operation creates a new directory whose name is pointed to by dirname.
  This function returns
  -1 if the directory already exists,
  -2 if there is not enough disk space,
  -3 if invalid directory name,
  -4 if the directory cannot be created for some other reason,
  and 0 if the directory is created successfully.
*/
int FileSystem::createDirectory(char *dirname, int dnameLen)
{
  char iNodeDir[64];
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;
  if (myPM->getFreeDiskBlock() == -1)
  {
    return -2; // there is not enough disk space
  }
  if (isFileName(dirname, dnameLen) == -1) return -3;
  pointer = findDir(dirname, dnameLen);
  if (pointer != 0) return -1; //file already exists
  if (dnameLen > 2) {
    char temp[dnameLen-2];
    for (int i = 0; i < (dnameLen-2);i++) {
      temp[i] = dirname[i];
    }
    currentDir = findDir(temp, dnameLen-2);
    if (currentDir == 0) return -4; //can't find parent dir...
  }

  // now we know what directory to put the dir (currentDir) and that it does not already exist
  // create directory
  // create iNodeDir
  int pos = 0;
  for (int i = 0; i < 10; i++)
  {
    iNodeDir[pos] = '0'; //will hold file/dir name
    myDM->intToChar(iNodeDir, 0, (pos + 1));
    iNodeDir[pos + 5] = '0'; //will hold f or d (file or dir)
    pos += 6;
  }                                  // filling in info (all empty right now)
  myDM->intToChar(iNodeDir, 0, 60); // very last 4
  int location = myPM->getFreeDiskBlock();  // location for dir inode

  myPM->writeDiskBlock(location, iNodeDir); // write out the dir inode

  // adding a pointer to new dir to the parent
  myPM->readDiskBlock(currentDir, buffer);
  // we need to figure out the next open place to put a pointer in this directory
  int dirLoc = currentDir;
  // cout << "dirLoc: " << dirLoc << endl;
  // cout << "before spotFinder: ";
  // for (int i = 0; i < 64; i++) {
  //   cout << buffer[i];
  // } cout << endl;
  int spot = spotFinder(buffer, dirLoc);
  if (spot == -1)
  {
    // there isn't room to expand the directory, problem!!
    myPM->returnDiskBlock(location); // erase the dir inode
    return -2;
  }
  // cout << "dirLoc: " << dirLoc << endl;
  // cout << "after spotFinder: ";
  // for (int i = 0; i < 64; i++) {
  //   cout << buffer[i];
  // } cout << endl;



  currentDir = dirLoc; // in case it changed
  myDM->intToChar(buffer, location, spot);
  buffer[spot - 1] = dirname[dnameLen - 1];
  buffer[spot + 4] = 'd';
  myPM->writeDiskBlock(currentDir, buffer);

  return 0;
}

int FileSystem::lockFile(char *filename, int fnameLen)
{
  int pointer, i;
  if (isFileName(filename, fnameLen) == -1) return -3;
  pointer = findFile(filename, fnameLen);
  if (pointer < 1)
  {
    return -2;
  }
  if (locked[pointer] != 0)
  {
    return -1;
  }
  if (isOpened(filename, fnameLen) != 0)
  {
    return -3;
  }

  locked[pointer] = lockID;
  return lockID++;

  return -4; // place holder so there is no warnings when compiling.
}

int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{
  int pointer, i;
  if (isFileName(filename, fnameLen) == -1) return -2;
  pointer = findFile(filename, fnameLen);
  if (pointer == 0) return -2; //file doesn't exist
  if (lockId < 0 || lockId > myfileSystemSize)
  {
    return -1;
  }
  if (locked[pointer] == 0)
  {
    return -2;
  }
  if (locked[pointer] == lockId)
  {
    locked[pointer] = 0;
    return 0;
  }
  return -2;
}

/*
  This operation deletes the file whose name is pointed to by filename. A file
  that is currently in use (opened or locked by a client) cannot be deleted. It returns
  -1 if the file does not exist,
  -2 if the file is in use or locked,
  -3 if the file cannot be deleted for any other reason, and 
  0 if the file is deleted successfully.       
*/
int FileSystem::deleteFile(char *filename, int fnameLen)
{
  // char iNodeDir[64];
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;

  if (isFileName(filename, fnameLen) == -1) return -3;

  //check if file exists
  pointer = findFile(filename, fnameLen);
  if (pointer == 0) return -1;
  if (fnameLen > 2) {
    char temp[fnameLen-2];
    for (int i = 0; i < (fnameLen-2);i++) {
      temp[i] = filename[i];
    }
    currentDir = findDir(temp, fnameLen-2);
    if (currentDir == 0) return -3; //can't find parent dir...
  }

  //now we have a pointer to the file inode (pointer)
  //and its parent dir (currentDir)
  //check if file is in use
  if (isOpened(filename, fnameLen) != 0)
  {
    return -2;
  }
  //chech if file is locked
  if (locked[pointer] != 0)
  {
    return -2;
  }

  vector<int> locations = fileLocations(pointer);
  myPM->readDiskBlock(pointer,buffer);
  int temp = myDM->charToInt(buffer,18);
  locations.push_back(temp);
  for (int i = 0; i < locations.size(); i++)
  {
    myPM->returnDiskBlock(locations[i]);
  }
  myPM->returnDiskBlock(pointer);

  myPM->readDiskBlock(currentDir, buffer);
  //finding the location in the currentDir to wipe
  int tempDir = currentDir;
  int spot = getSpot(currentDir, pointer, 'f');
  while (spot < 1)
  {
    currentDir = tempDir;
    if (spot == 0)
    {
      //error
      return -3;
    }
    if (spot == -1)
    {
      tempDir = myDM->charToInt(buffer, 60); //pointer to continuation
      myPM->readDiskBlock(tempDir, buffer);
      spot = getSpot(tempDir, pointer, 'f');
    }
  }
  currentDir = tempDir;
  if (isDirEmpty(tempDir)) {
    myPM->returnDiskBlock(tempDir);
    myPM->readDiskBlock(currentDir, buffer);
    myDM->intToChar(buffer, 0, 60);
    myPM->writeDiskBlock(currentDir, buffer);

  }
  else {
    myPM->readDiskBlock(tempDir, buffer);
    myDM->intToChar(buffer, 0, spot);
    buffer[spot - 1] = '0';
    buffer[spot + 4] = '0';
    myPM->writeDiskBlock(tempDir, buffer);
  }

  return 0;
}

/*
  This operation deletes the directory whose name is pointed to by dirname. Only
  an empty directory can be deleted. This function returns 
  -1 if the directory does not exist, 
  -2 if the directory is not empty, 
  -3 if the directory cannot bec deleted for any other reason, 
  and 0 if the directory is deleted successfully.
*/
int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{
  // char iNodeDir[64];
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;
  if (isFileName(dirname, dnameLen) == -1) return -3;
  pointer = findDir(dirname, dnameLen);
  if (pointer == 0) return -1; //dir doesn't exist
  if (dnameLen > 2) {
    char temp[dnameLen-2];
    for (int i = 0; i < (dnameLen-2);i++) {
      temp[i] = dirname[i];
    }
    currentDir = findDir(temp, dnameLen-2);
    if (currentDir == 0) return -3; //can't find parent dir...
  }

  //now we have a pointer to the dir inode (pointer)
  if (!isDirEmpty(pointer))
  {
    return -2;
  }
  bool temp = true;
  int tempPointer;
  char tempBuffer[64];
  myPM->readDiskBlock(pointer, tempBuffer);
  vector<int> locations;
  locations.push_back(pointer);
  while (temp)
  {
    tempPointer = myDM->charToInt(tempBuffer, 60);
    if (tempPointer == 0)
    {
      //no continuation of directory
      temp = false;
    }
    else
    {
      //there is a continuation
      locations.push_back(tempPointer);
      myPM->readDiskBlock(tempPointer, tempBuffer);
    }
  }
  for (int j = 0; j < locations.size(); j++)
  {
    myPM->returnDiskBlock(locations[j]);
  }

  //now we need to alter the currentDir to wipe the pointer to deleted dir
  myPM->readDiskBlock(currentDir, buffer);
  //finding the location in the currentDir to wipe

  int tempDir = currentDir;
  int spot = getSpot(currentDir, pointer, 'd');
  while (spot < 1)
  {
    currentDir = tempDir;
    if (spot == 0)
    {
      //error
      return -3;
    }
    if (spot == -1)
    {
      tempDir = myDM->charToInt(buffer, 60); //pointer to continuation
      myPM->readDiskBlock(tempDir, buffer);
      spot = getSpot(tempDir, pointer, 'd');
    }
  }
  currentDir = tempDir;


  myDM->intToChar(buffer, 0, spot);
  buffer[spot - 1] = '0';
  buffer[spot + 4] = '0';
  myPM->writeDiskBlock(currentDir, buffer);

  return 0;
}

int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{
  FileTable createTable;
  if (isFileName(filename, fnameLen)) return -4;
  int pointer = findFile(filename, fnameLen);
  //Check if file exists
  if (pointer < 1)
  {
    return -1;
  }
  //Check if file is locked
  if (locked[pointer] != 0 && locked[pointer] != lockId)
  {
    return -3;
  }
  else if (lockId != -1 && locked[pointer] == 0)
  {
    return -3;
  }
  //Verify mode
  switch (mode)
  {
  case 'r':
    createTable.mode = 'r';
    createTable.filename = filename;
    createTable.fnameLen = fnameLen;
    opened[fileID] = createTable;
    return fileID++;
    break;
  case 'w':
    createTable.mode = 'w';
    createTable.filename = filename;
    createTable.fnameLen = fnameLen;
    opened[fileID] = createTable;
    return fileID++;
    break;
  case 'm':
    createTable.mode = 'm';
    createTable.filename = filename;
    createTable.fnameLen = fnameLen;
    opened[fileID] = createTable;
    return fileID++;
    break;
  default:
    return -2;
  }
  return pointer;
  return -1; // place holder so there is no warnings when compiling.
}

int FileSystem::closeFile(int fileDesc)
{
  if (fileDesc < 0 || fileDesc >= myfileSystemSize || opened.find(fileDesc)->second.mode == 'c')
  {
    return -1; // place holder so there is no warnings when compiling.
  }
  else
  {
    opened.find(fileDesc)->second.mode = 'c';
    return 0;
  }
  return -2;
}

int FileSystem::readFile(int fileDesc, char *data, int len)
{
  int finalLength = 0;
  int endofread = opened[fileDesc].rw;
  if (opened.find(fileDesc) == opened.end())
  {
    //not found
    return -1;
  }
  if (len < 0)
  {
    return -2;
  }
  if (opened[fileDesc].mode == 'r' || opened[fileDesc].mode == 'm')
  {
    //file is correct, begin read
    int pointer = findFile(opened[fileDesc].filename, opened[fileDesc].fnameLen);
    vector<int> locations = fileLocations(pointer);
    int numBlocksNeeded = (len / 64) + 1;
    //read file size
    int filesize;
    char buffer[64];
    myPM->readDiskBlock(pointer, buffer);
    filesize = myDM->charToInt(buffer, 2);
    endofread = opened[fileDesc].rw + len;
    if (opened[fileDesc].rw == filesize)
    {
      return 0;
    }

    if (opened[fileDesc].rw + len > filesize)
    {
      finalLength = filesize - opened[fileDesc].rw;
      endofread = filesize;
    }
    else
    {
      finalLength = len;
    }
    int totalcount = finalLength;
    int startpoint = opened[fileDesc].rw % 64;
    char tempBuffer[64];
    int nthloc = opened[fileDesc].rw / 64;
    int j = 0;
    //actually write it out
    for (int i = numBlocksNeeded; i > 0; i--)
    {
      myPM->readDiskBlock(locations[nthloc], tempBuffer);

      for (int k = startpoint; k < 64; k++)
      {
        if (totalcount == 0)
        {
          break;
        }
        data[j] = tempBuffer[k];
        j++;
        totalcount--;
      }
      startpoint = 0;
      nthloc++;
    }
  }
  else
  {
    return -3; //not permitted
  }
  opened[fileDesc].rw = endofread;
  return finalLength;
}

/*
  This operation performs write on a file whose file descriptor is filedesc. 
  len is the number of bytes to be written into the buffer pointed to by data. These operations return 
  -1 if the file descriptor is invalid, 
  -2 if length is negative, 
  -3 if the operation is not permitted, 
  and number of bytes written if successful. 
  The write operation operates from the byte pointed to by the rw pointer. 
  The write operation overwrites the existing data in the file and may increase the size of the file. 
  After the write is done, the rw pointer is updated to point to the byte following the last byte written.
*/
int FileSystem::writeFile(int fileDesc, char *data, int len)
{
  //check fileDesc
  if (opened.find(fileDesc) == opened.end())
  {
    //not found
    return -1;
  } //else found:
  //check if operation is permitted
  if (opened[fileDesc].mode == 'c' || opened[fileDesc].mode == 'r')
  {
    return -3;
  }
  //check if len is negative
  if (len < 0)
  {
    return -2;
  }

  int pointer = findFile(opened[fileDesc].filename, opened[fileDesc].fnameLen);

  //find current file size
  int filesize;
  char buffer[64];
  myPM->readDiskBlock(pointer, buffer);

  filesize = myDM->charToInt(buffer, 2);

  int endofwrite = opened[fileDesc].rw + len;

  //figure out how many blocks we need
  double number = ((double)endofwrite) / 64;
  int numBlocksNeeded = (int)ceil(number);

  //get list of locations of the file
  vector<int> locations = fileLocations(pointer);
  if (locations.size() < numBlocksNeeded)
  {
    locations = addBlocks(pointer, locations, numBlocksNeeded);
    if (locations.size() != numBlocksNeeded)
    {
      //max file size exceeded or something else went wrong
      return -3;
    }
  }

  //buffer may have changed  during addBlocks
  myPM->readDiskBlock(pointer, buffer);

  //find actual location of rw value
  //decide if i need to increase the file size

  if (filesize < endofwrite)
  {
    //increase file size
    myDM->intToChar(buffer, endofwrite, 2);
    myPM->writeDiskBlock(pointer, buffer);
  }

  int totalcount = len;
  int startpoint = opened[fileDesc].rw % 64;
  char tempBuffer[64];
  int nthloc = opened[fileDesc].rw / 64;
  int j = 0;
  //actually write it out
  for (int i = numBlocksNeeded - nthloc; i > 0; i--)
  {
    myPM->readDiskBlock(locations[nthloc], tempBuffer);
    for (int k = startpoint; k < 64; k++)
    {
      if (totalcount == 0)
      {
        break;
      }
      tempBuffer[k] = data[j];
      j++;
      totalcount--;
    }
    myPM->writeDiskBlock(locations[nthloc], tempBuffer);
    startpoint = 0;
    nthloc++;
  }
  opened[fileDesc].rw = endofwrite;
  return len; // place holder so there is no warnings when compiling.
}

int FileSystem::appendFile(int fileDesc, char *data, int len)
{
  if (opened.find(fileDesc) == opened.end())
    return -1;
  if (opened[fileDesc].mode == 'r')
    return -3;

  char buff[64];

  int pointer = findFile(opened[fileDesc].filename, opened[fileDesc].fnameLen);
  myPM->readDiskBlock(pointer, buff);
  int filesize = myDM->charToInt(buff, 2);
  opened[fileDesc].rw = filesize;

  int r = writeFile(fileDesc, data, len);
  return r;
}

int FileSystem::truncFile(int fileDesc, int offset, int flag)
{

  int filesize;
  int currentDir = 1;
  char buffer[64];
  if (opened.find(fileDesc) == opened.end())
    return -1;
  if (opened[fileDesc].mode == 'r')
    return -3;
  if (opened[fileDesc].mode != 'r' && opened[fileDesc].mode != 'w' && opened[fileDesc].mode != 'm') {
    return -1;
  }
  // for ( int i = 0; i < opened[fileDesc].fnameLen; i++) {
  //   cout << opened[fileDesc].filename[i];
  // } cout << endl;
  // cout << "made it here1" << endl;
  int pointer = findFile(opened[fileDesc].filename, opened[fileDesc].fnameLen);
  vector<int> locations = fileLocations(pointer);
  myPM->readDiskBlock(pointer, buffer);
  filesize = myDM->charToInt(buffer, 2);
  
  int result = seekFile(fileDesc, offset, flag);
  if (result < 0)
  {
    //error
    return result;
  }

  if (filesize == opened[fileDesc].rw)
  {
    return 0; //nothing to delete
  }
  // myPM->readDiskBlock(pointer, buffer);
  //what is the filesize going to be at the end
  int futurefs = opened[fileDesc].rw; //bc everything after rw is going to be deleted
  int deleted = filesize - futurefs;  //amount to be deleted
  double number = ((double)futurefs) / 64;
  int numBlocksNeeded = (int)ceil(number);
  //see if we need to return any blocks
  //
  if (numBlocksNeeded < locations.size())
  {
    locations = removeBlocks(pointer, locations, (locations.size() - numBlocksNeeded)); //also updates locations
  }
  myPM->readDiskBlock(pointer, buffer); //reread bc it probably changed
  //didn't make it here

  //now we have the correct amount of blocks
  //may still need to delete some in the last block
  //delete from rw to end of last block:
  char tempBuffer[64];
  myPM->readDiskBlock(locations[locations.size() - 1], tempBuffer);
  for (int h = opened[fileDesc].rw; h < 64; h++)
  {
    tempBuffer[h] = '#';
  }
  myPM->writeDiskBlock(locations[locations.size() - 1], tempBuffer);

  //change the listed file size
  myDM->intToChar(buffer, futurefs, 2);
  myPM->writeDiskBlock(pointer, buffer);

  return deleted;
}

/*
  This operation modifies the rw pointer of the file whose file descriptor is
  filedesc. The rw pointer is moved offset bytes forward if flag = 0. Otherwise,
  it is set to byte number offset in the file. This operation returns 
  -1 if the file descriptor, offset or flag is invalid, 
  -2 if an attempt to go outside the file bounds is made (end of file or beginning of file), 
  and 0 if successful. 
  A negative offset is valid only when flag is zero.
*/
int FileSystem::seekFile(int fileDesc, int offset, int flag)
{
  //check fileDesc
  if (opened.find(fileDesc) == opened.end())
  {
    //not found
    return -1;
  } //else found:

  //getting file size
  int filesize;
  int pointer = findFile(opened[fileDesc].filename, opened[fileDesc].fnameLen);
  char buffer[64];
  myPM->readDiskBlock(pointer, buffer);
  filesize = myDM->charToInt(buffer, 2);

  if (flag == 0)
  {
    //checking if offset is too large or too small.
    if (((opened[fileDesc].rw + offset) > filesize) || ((opened[fileDesc].rw + offset) < 0))
    {
      //offset if too large/too small
      return -2;
    }
    //change the rw pointer
    opened[fileDesc].rw += offset;
  }
  else
  {
    if ((offset < 0) || (offset > filesize))
    {
      return -1; //invalid offset
    }
    //change the rw pointer
    opened[fileDesc].rw = offset;
  }
  return 0;
}

int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{
  char iNodeFile[64];
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;

  // various checks
  if (isFileName(filename1, fnameLen1) != 0 || isFileName(filename2, fnameLen2) != 0) return -1; // not a proper file name
  if (compareChars(filename1, filename2)) return -3; // are they the same names
  pointer = findFile(filename1, fnameLen1);
  if (pointer == 0) return -2;
  if (findFile(filename2, fnameLen2) != 0) return -3;
  if (locked[pointer] != 0) return -4; // file is locked, do nothing
  if (isOpened(filename1, fnameLen1) != 0) return -4; // file is open, do nothing

  // file exists on disk, prepare to rename
  myPM->readDiskBlock(pointer, iNodeFile);

  // set the file name to it's new name inside the buffer
  iNodeFile[0] = filename2[fnameLen2 - 1];
  // write the new name to the buffer;
  myPM->writeDiskBlock(pointer, iNodeFile);
  //now we need to change the name in the parent directory
  //first step is to get the pointer to the parent directory
  if (fnameLen1 > 2) {
    char temp[fnameLen1-2];
    for (int i = 0; i < (fnameLen1-2);i++) {
      temp[i] = filename1[i];
    }
    currentDir = findDir(temp, fnameLen1-2);
    // if (currentDir == 0) return -4; //can't find parent dir...
  }
  else currentDir = 1;
  
  //now we have the parent dir (currentDir)
  //we need to find the spot where the old file name is
  int spot = getSpot(currentDir, pointer, 'f');
  myPM->readDiskBlock(currentDir, buffer);
  while (spot < 1)
  {
    if (spot == 0)
    {
      //error
      return -4;
    }
    if (spot == -1)
    {
      currentDir = myDM->charToInt(buffer, 60); //pointer to continuation
      spot = getSpot(currentDir, pointer, 'f');
    }
  }

  myPM->readDiskBlock(currentDir, buffer);
  buffer[spot - 1] = filename2[fnameLen2 - 1];
  myPM->writeDiskBlock(currentDir, buffer);

  return 0; // succesfully renamed file

  return -5; // failed for unknown reason
}

int FileSystem::renameDirectory(char *dirname1, int dnameLen1, char *dirname2, int dnameLen2)
{
  char buffer[64];
  int currentDir = 1; // root directory
  int pointer;

  if (isFileName(dirname1, dnameLen1) != 0 || isFileName(dirname2, dnameLen2) != 0)
  {
    return -1; // not a proper directory name
  }
  if (findDir(dirname1, dnameLen1) == 0) return -2;
  if (findDir(dirname2, dnameLen2) != 0) return -3;

  pointer = findDir(dirname1, dnameLen1);
  if (dnameLen1 > 2) {
    char temp[dnameLen1-2];
    for (int i = 0; i < (dnameLen1-2);i++) {
      temp[i] = dirname1[i];
    }
    currentDir = findDir(temp, dnameLen1-2);
    if (currentDir == 0) return -4; //can't find parent dir...
  }

  myPM->readDiskBlock(currentDir, buffer);
  int tempDir = currentDir;
  int spot = getSpot(currentDir, pointer, 'd');
  while (spot < 1)
  {
    currentDir = tempDir;
    if (spot == 0)
    {
      //error
      return -4;
    }
    if (spot == -1)
    {
      tempDir = myDM->charToInt(buffer, 60); //pointer to continuation
      myPM->readDiskBlock(tempDir, buffer);
      spot = getSpot(tempDir, pointer, 'd');
    }
  }
  currentDir = tempDir;
  // cout << "spot: " << spot << endl;
  // cout << "before write: ";
  // for (int i = 0; i < 64; i++) {
  //   cout << buffer[i];
  // } cout << endl;

  myPM->readDiskBlock(currentDir, buffer);
  buffer[spot-1] = dirname2[dnameLen2 - 1];
  myPM->writeDiskBlock(currentDir, buffer);
  return 0;
}

/*
  This operation retreives the attribute 'b' bookmarked (1=bookmarked, 0=not bookmarked), or 'p' priority 1 (highest) - 5 (lowest).
  Based on the character variable 'attribute' this operation returns:
  -3 if attributes not set
  -2 if invalid file given
  -1 if invalid attribute given
  and the appropriate attribute number (0 or 1 for 'b', 1-5 for 'p') otherwise
*/
int FileSystem::getAttributes(char *filename, int fnameLen, char attribute)
{
  //bookmarked file = 1, not bookmarked = 0 => index 22
  //priority 1 (highest) - 5 (lowest) => index 23
  int pointer = findFile(filename, fnameLen);
  if (pointer <= 0)
  {
    return -2;
  }
  char buffer[64];
  myPM->readDiskBlock(pointer, buffer);

  if (attribute == 'b')
  {
    if(buffer[22] == 0 || buffer[22] == 1){
      return buffer[22];
    }
    else{
      return -3;
    }
  }
  if (attribute == 'p')
  {
    if(buffer[23] >= 1 && buffer[23] <= 5){
      return buffer[23];
    }
    else{
      return -3;
    }
  }
  return -1; //didnt match either attribute
}

/*
  This operation sets the attribute 'b' bookmarked (1=bookmarked, 0=not bookmarked), or 'p' priority 1 (highest) - 5 (lowest).
  Based on the character variable 'attribute' this operation returns:
  -3 if invalid value given
  -2 if invalid file given
  -1 if invalid attribute given
  and 0 otherwise
  note default for 'b' is 0, 'p' is 5 (lowest priority)
*/
int FileSystem::setAttributes(char *filename, int fnameLen, char attribute, int value)
{
  //bookmarked file = 1, not bookmarked = 0 => index 22
  //priority 1 (highest) - 5 (lowest) => index 23
  int pointer = findFile(filename, fnameLen);
  if (pointer <= 0)
  {
    return -2;
  }
  char buffer[64];
  myPM->readDiskBlock(pointer, buffer);

  if (attribute == 'b')
  {
    if (value == 0 || value == 1)
    {
      buffer[22] = (char)value;
      myPM->writeDiskBlock(pointer, buffer);
      return 0;
    }
    else{
      return -3;
    }
  }
  if (attribute == 'p')
  {
    if (value >= 1 && value <= 5)
    {
      buffer[23] = (char)value;
      myPM->writeDiskBlock(pointer, buffer);
      return 0;
    }
    else{
      return -3;
    }
  }
  return -1; //didnt match either attribute
}

int FileSystem::getSubFile(int blocknum, char file)
{
  char buffer[64];
  // cout << "blocknum: " << blocknum << " subDir: " << subDir << endl;
  myPM->readDiskBlock(blocknum, buffer);
  int pointer = 0;
  int pos = 0;
  char tempBuffer[64];
  for (int i = 0; i < 10; i++)
  {
    if (buffer[pos] == file && buffer[pos+5] == 'f')
    {
      return myDM->charToInt(buffer, (pos + 1)); //pointer to file
    }
    pos += 6;
  }
  pointer = myDM->charToInt(buffer, 60);
  if (pointer == blocknum) return 0;
  if (pointer != 0)
  {
    return getSubFile(pointer, file);
  }
  return 0;
}

int FileSystem::getSubDir(int blocknum, char subDir)
{
  char buffer[64];
  // cout << "blocknum: " << blocknum << " subDir: " << subDir << endl;
  myPM->readDiskBlock(blocknum, buffer);
  int pointer = 0;
  int pos = 0;
  char tempBuffer[64];
  for (int i = 0; i < 10; i++)
  {
    if (buffer[pos] == subDir && buffer[pos+5] == 'd')
    {
      return myDM->charToInt(buffer, (pos + 1)); //pointer to subDir
    }
    pos += 6;
  }
  pointer = myDM->charToInt(buffer, 60);
  if (pointer == blocknum) return 0;
  if (pointer != 0)
  {
    return getSubDir(pointer, subDir);
  }
  return 0;
}

int FileSystem::getSpot(int blocknum, int pointer, char type)
{
  char buffer[64];
  myPM->readDiskBlock(blocknum, buffer);
  int pos = 1;
  char tempBuffer[64];
  for (int i = 0; i < 10; i++)
  {
    if (myDM->charToInt(buffer, pos) == pointer && buffer[pos+4] == type)
    {
      return pos;
    }
    pos += 6;
  }
  int temp = myDM->charToInt(buffer, 60);
  if (temp != 0)
  {
    return -1; //spot is in the continuation of the directory
  }
  return 0; //didn't find it
}

bool FileSystem::isDirEmpty(int blocknum)
{
  char buffer[64];
  myPM->readDiskBlock(blocknum, buffer);
  int pointer;
  int pos = 0;
  for (int i = 0; i < 10; i++)
  {
    if (buffer[pos] != '0')
    {
      return false; //not empty
    }
    pos += 6;
  }
  pointer = myDM->charToInt(buffer, 60); // pos = 61 here
  if (pointer == blocknum) return true;
  if (pointer != 0)
  {
    return isDirEmpty(pointer);
  }
  return true;
}

int FileSystem::spotFinder(char *dirContents, int &dirLoc)
{
  int pos = 0;
  int pointer;
  char tempBuffer[64];
  for (int i = 0; i < 10; i++)
  {
    if (dirContents[pos] == '0') //looking at spot for file/dir name
    {                            // empty spot
      return pos + 1;            //spot to write pointer to
    }
    pos += 6;
  }
  // end of that block
  // we need to look for a spot in the continuation
  pointer = myDM->charToInt(dirContents, 60);
  if (pointer != 0)
  {
    dirLoc = pointer;
    // there could be a spot in the continuation of directory
    myPM->readDiskBlock(dirLoc, dirContents);
    return spotFinder(dirContents, dirLoc);
  }
  // there was not a spot in this directory so we need to create a continuation
  int newDirLoc = myPM->getFreeDiskBlock();
  if (newDirLoc == -1)
  {
    // problem!!
    return -1;
  }
  // setting the pointer to next dir inode
  myDM->intToChar(dirContents, newDirLoc, 60);
  myPM->writeDiskBlock(dirLoc, dirContents); // write it out

  dirLoc = newDirLoc;
  pos = 0;
  for (int i = 0; i < 10; i++)
  {
    dirContents[pos] = '0'; //file dir name
    myDM->intToChar(dirContents, 0, (pos + 1));
    dirContents[pos + 5] = '0'; //file or dir
    pos += 6;
  }                                          // clearing out dirbuffer
  myDM->intToChar(dirContents, 0, pos);      //last position
  myPM->writeDiskBlock(dirLoc, dirContents); // write it out
  return 1;                                  // return the first pos
}

int FileSystem::findFile(char *filename, int fnameLen)
{
  int pointer = 0;
  int currentDir = 1;
  for (int i = 0; i < fnameLen; i++)
  {
    if (i % 2 == 1)
    { // if index is even, i.e.  position 0,2,... should all be '/'
      if (i == fnameLen-1) {
        //if its the file character:
        pointer = getSubFile(currentDir, filename[i]);
        if (pointer == 0)
        { // could not find file
          return 0;
        }
      }
      else {
        // it is a valid character, now we check if the directory exists/where to find it
        pointer = getSubDir(currentDir, filename[i]);
        if (pointer == 0)
        { // could not find directory
          return 0;
        }
        currentDir = pointer;
      }
    }
  }

  //pointer = getSubDir(currentDir, filename[fnameLen - 1]);
  return pointer; // filename is correct, return a pointer to it
}

int FileSystem::findDir(char *dirname, int dnameLen)
{
  int pointer = 0;
  int currentDir = 1;
  for (int i = 0; i < dnameLen; i++)
  {
    if (i % 2 == 1)
    {
      //if its the file character:
      pointer = getSubDir(currentDir, dirname[i]);
      if (pointer == 0)
      { // could not find file
        return 0;
      }
      currentDir = pointer;
    }
  }
  //pointer = getSubDir(currentDir, filename[fnameLen - 1]);
  return pointer; // filename is correct, return a pointer to it
}

int FileSystem::isFileName(char *filename, int fnameLen)
{
  for (int i = 0; i < fnameLen; i++)
  {
    if (i % 2 == 0)
    { // if index is even, i.e.  position 0,2,... should all be '/'
      if (filename[i] != '/')
      {
        return -1;
      }
    }
    else if (!isalpha(filename[i]))
    {
      return -1;
    }
  }
  return 0;
}

vector<int> FileSystem::fileLocations(int blocknum)
{
  vector<int> locations;
  char buffer[64];
  myPM->readDiskBlock(blocknum, buffer);
  int pointer;
  //direct address
  pointer = myDM->charToInt(buffer, 6);
  if (pointer != 0)
  {
    locations.push_back(pointer);
  }
  else
  {
    return locations;
  }
  //direct address
  pointer = myDM->charToInt(buffer, 10);
  if (pointer != 0)
  {
    locations.push_back(pointer);
  }
  else
  {
    return locations;
  }
  //direct address
  pointer = myDM->charToInt(buffer, 14);
  if (pointer != 0)
  {
    locations.push_back(pointer);
  }
  else
  {
    return locations;
  }
  //indirect address
  pointer = myDM->charToInt(buffer, 18);
  if (pointer == 0)
  {
    return locations; //no indirect address
  }
  else
  {
    myPM->readDiskBlock(pointer, buffer);
    int pos = 0;
    for (int i = 0; i < 16; i++)
    {
      if (buffer[pos] != '#')
      {
        pointer = myDM->charToInt(buffer, pos);
        locations.push_back(pointer);
      }
      pos += 4;
    }
  }
  return locations;
}

int FileSystem::isOpened(char *fname, int fnameLen)
{
  int n = 0;

  for (auto it : opened)
  {
    if (isSameName(it.second.filename, it.second.fnameLen, fname, fnameLen)) {
      if (it.second.mode != 'c') n++;
    }
  }
  return n;
}

vector<int> FileSystem::addBlocks(int inodeLoc, vector<int> locations, int num)
{
  vector<int> newLocations = locations;
  //checking the max
  if (locations.size() == 19)
  {
    return locations;
  }
  char buffer[64];
  char claim[64];
  for (int i = 0; i < 64; i++)
  {
    claim[i] = '#';
  }
  myPM->readDiskBlock(inodeLoc, buffer);
  int free;
  int temp = num - newLocations.size();
  switch (newLocations.size())
  {
  case 0:
    free = myPM->getFreeDiskBlock();
    if (free == -1)
    {
      return newLocations;
    }
    myPM->writeDiskBlock(free, claim);
    //add to first direct pointer
    myDM->intToChar(buffer, free, 6);
    newLocations.push_back(free);
    temp--;
    //check num & break if 0
    if (temp == 0)
    {
      break;
    }
  case 1:
    free = myPM->getFreeDiskBlock();
    if (free == -1)
    {
      return newLocations;
    }
    myPM->writeDiskBlock(free, claim);
    //add to second direct pointer
    myDM->intToChar(buffer, free, 10);
    newLocations.push_back(free);
    temp--;
    //check num & break if 0
    if (temp == 0)
    {
      break;
    }
  case 2:
    free = myPM->getFreeDiskBlock();
    if (free == -1)
    {
      return newLocations;
    }
    myPM->writeDiskBlock(free, claim);
    //add to third direct pointer
    myDM->intToChar(buffer, free, 14);
    newLocations.push_back(free);
    temp--;
    //check num & break if 0
    if (temp == 0)
    {
      break;
    }
  case 3:
    //add an indirect pointer
    int indirect;
    indirect = myPM->getFreeDiskBlock();
    if (indirect == -1)
    {
      return newLocations;
    }
    myPM->writeDiskBlock(indirect, claim);
    //add pointer to indirect block
    myDM->intToChar(buffer, indirect, 18);
  default:
    //loop add to indirect pointer
    indirect = myDM->charToInt(buffer, 18);
    int pos = (newLocations.size()-3)*4;
    char tempBuffer[64];
    myPM->readDiskBlock(indirect, tempBuffer);
    for (int i = 0; i < 16; i++)
    {
      free = myPM->getFreeDiskBlock();
      if (free == -1)
      {
        return newLocations;
      }
      myPM->writeDiskBlock(free, claim);
      myDM->intToChar(tempBuffer, free, pos);
      newLocations.push_back(free);
      temp--;
      myPM->writeDiskBlock(indirect, tempBuffer);
      if (temp == 0)
      {
        break;
      }
      pos += 4;
    }
  }
  myPM->writeDiskBlock(inodeLoc, buffer);
  return newLocations;
}

vector<int> FileSystem::removeBlocks(int inodeLoc, vector<int> locations, int num)
{
  //num is the num that need to be deleted
  vector<int> oldLocations = locations;
  char buffer[64];
  char tempBuffer[64];
  myPM->readDiskBlock(inodeLoc, buffer);
  int indirectPointer;
  int temp = num;
  int block;
  int pos;

  while (oldLocations.size() > 3)
  {
    pos = 0;
    //there are indirect locations
    indirectPointer = myDM->charToInt(buffer, 18);
    myPM->readDiskBlock(indirectPointer, tempBuffer);
    block = oldLocations[oldLocations.size() - 1];
    myPM->returnDiskBlock(block); //return it
    for (int i = 0; i < 16; i++)
    {
      if (myDM->charToInt(tempBuffer, pos) == block)
      {
        myDM->intToChar(tempBuffer, 0, pos);
        oldLocations.pop_back();
        temp--; //deleted one location;
      }
      pos += 4;
    }
    myPM->writeDiskBlock(indirectPointer, tempBuffer);
    if (oldLocations.size() == 3)
    {
      myPM->returnDiskBlock(indirectPointer);
      myDM->intToChar(buffer, 0, 18);
      myPM->writeDiskBlock(inodeLoc, buffer); //got rid of indirect pointer
    }
    if (temp == 0)
    {
      return oldLocations;
    }
  }
  switch (oldLocations.size())
  {
  case 3:
    block = oldLocations[oldLocations.size() - 1];
    myDM->intToChar(buffer, 0, 14); //6 10 14
    myPM->returnDiskBlock(block);   //return it
    oldLocations.pop_back();
    temp--;
    if (temp == 0)
    {
      break;
    }
  case 2:
    block = oldLocations[oldLocations.size() - 1];
    myDM->intToChar(buffer, 0, 10); //6 10 14
    myPM->returnDiskBlock(block);   //return it
    oldLocations.pop_back();
    temp--;
    if (temp == 0)
    {
      break;
    }
  case 1:
    block = oldLocations[oldLocations.size() - 1];
    myDM->intToChar(buffer, 0, 6); //6 10 14
    myPM->returnDiskBlock(block);  //return it
    oldLocations.pop_back();
    temp--;
    if (temp == 0)
    {
      break;
    }
  }
  myPM->writeDiskBlock(inodeLoc, buffer);
  return oldLocations;
}

bool FileSystem::doesFileSystemExist()
{
  char buffer[64];
  myPM->readDiskBlock(1, buffer);
  if (buffer[0] != '#')
  {
    return true;
  }
  return false;
}

bool FileSystem::compareChars(char *first, char *second)
{
  int lenFirst = strlen(first);
  int lenSec = strlen(second);
  if (lenFirst != lenSec)
    return false;
  for (lenSec = 0; lenSec < lenFirst; lenSec++)
  {
    if (first[lenSec] != second[lenSec])
      return false;
  }
  return true;
}

bool FileSystem::isSameName(char *filename1, int fnameLen1, char *filename2, int fnameLen2) {
  if (fnameLen1 != fnameLen2) return false;
  for (int i = 1; i < fnameLen2; i+=2) {
    if (filename1[i] != filename2[i]) return false;
  }
  return true;
}
