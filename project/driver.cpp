
/* This is an example of a driver to test the filesystem */

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "client.h"
using namespace std;

int main()
{
  Disk *d = new Disk(300, 64, const_cast<char *>("DISK1"));
  DiskPartition *dp = new DiskPartition[3];

  dp[0].partitionName = 'A';
  dp[0].partitionSize = 100;
  dp[1].partitionName = 'B';
  dp[1].partitionSize = 75;
  dp[2].partitionName = 'C';
  dp[2].partitionSize = 105;

  char testBuffer[64] = {'h','e','l','l','o'};

  DiskManager *dm = new DiskManager(d, 3, dp);
 

  PartitionManager *pm = new PartitionManager(dm, 'A', 100);
  PartitionManager *pmB = new PartitionManager(dm, 'B', 75);
  PartitionManager *pmC = new PartitionManager(dm, 'C', 105);
  cout << "DM BlockSize: " << dm->getBlockSize() << endl;
  cout << "DM Partition size for A: " << dm->getPartitionSize('A') << endl;
  cout << "DM Partition size for B: " << dm->getPartitionSize('B') << endl;
  cout << "DM Partition size for C: " << dm->getPartitionSize('C') << endl;
  cout << "free disk block: " << pm->getFreeDiskBlock() << endl;
  pm->writeDiskBlock(1,testBuffer);
  cout << "free disk block: " << pm->getFreeDiskBlock() << endl;
  pm->writeDiskBlock(1,testBuffer);
  pm->writeDiskBlock(2,testBuffer);
  cout << "free disk block: " << pm->getFreeDiskBlock() << endl;
  
  pmB->writeDiskBlock(0,testBuffer);

  FileSystem *fs1 = new FileSystem(dm, 'A');
  cout << "File System 'A' created" << endl;
  fs1->createFile(const_cast<char *>("/a"), 2);
  cout << "file '/a' created" << endl;
  fs1->createFile(const_cast<char *>("/b"), 2);
  cout << "file '/b' created" << endl;
  int result = fs1->createFile(const_cast<char *>("/a"), 2);
  cout << "tried to create file '/a' again, should have failed (-1): " << result << endl; //work in progress
  fs1->createDirectory(const_cast<char *>("/e"), 2);
  cout << "directory '/e' created" << endl;
  fs1->createFile(const_cast<char *>("/e/n"), 4);
  cout << "directory '/e/n' created" << endl; //work in progress
  fs1->createDirectory(const_cast<char *>("/e/s"), 4);
  cout << "directory '/e/s' created" << endl;
  fs1->createDirectory(const_cast<char *>("/e/s/f"), 6);
  cout << "directory '/e/s/f' created" << endl;

  fs1->deleteFile(const_cast<char *>("/a"), 2);
  cout << "file /a deleted" << endl;
  fs1->deleteFile(const_cast<char *>("/e/n"), 4);
  cout << "file /e/n deleted" << endl;
  fs1->deleteDirectory(const_cast<char *>("/e/s/f"), 6);
  cout << "directory /e/s/f deleted" << endl;


  //these work so far but need thourough testing
  cout << "a->j: " << fs1->renameFile(const_cast<char *>("/a"), 2, const_cast<char *>("/j"),2) << endl;
  cout << "/e/n -> /e/j: " << fs1->renameFile(const_cast<char *>("/e/n"), 4, const_cast<char *>("/e/j"),4) << endl;
  cout << "rename direc as file (-1) " << fs1->renameFile(const_cast<char *>("/e"), 4, const_cast<char *>("/k"),4) << endl;
  cout << "lock file /j " << fs1->lockFile(const_cast<char *>("/j"),2) << endl;
  cout << "change locked file: " << fs1->renameFile(const_cast<char *>("/j"), 2, const_cast<char *>("/a"),2) << endl;
  cout << "change file to existing name: " << fs1->renameFile(const_cast<char *>("/b"), 2, const_cast<char *>("/j"),2) << endl;

  cout << "rename directory e->k: " << fs1->renameDirectory(const_cast<char *>("/e"), 2, const_cast<char *>("/k"),2) << endl;
  cout << "rename missing direct: " << fs1->renameDirectory(const_cast<char *>("/p"), 2, const_cast<char *>("/v"),2) << endl; 
  fs1->createDirectory(const_cast<char *>("/g"), 2);
  cout << "directory '/e/e/e/e/k/f' created" << endl;
  //Tests for lock and unlock openFile(char *filename, int fnameLen, char mode, int lockId)
  cout << "file j opened(-3 expected)" << fs1->openFile(const_cast<char *>("/j"),2,'w',2) << endl;
  cout << "unlock j(0 expected)" << fs1->unlockFile(const_cast<char *>("/j"),2,2) << endl;
  cout << "file j opened(2 expected)" << fs1->openFile(const_cast<char *>("/j"),2,'w',2) << endl;
  cout << "file j closed(0 expected)" << fs1->closeFile(2) << endl;


  return 0;
  // FileSystem *fs1 = new FileSystem(dm, 'A');
  FileSystem *fs2 = new FileSystem(dm, 'B');
  FileSystem *fs3 = new FileSystem(dm, 'C');
  Client *c1 = new Client(fs1);
  Client *c2 = new Client(fs1);
  Client *c3 = new Client(fs1);
  Client *c4 = new Client(fs2);
  Client *c5 = new Client(fs2);
  
  c1->myFS->createFile(const_cast<char *>("/a"), 2);
  c1->myFS->createFile(const_cast<char *>("/b"), 2);
  c2->myFS->createFile(const_cast<char *>("/a"), 2);
  c4->myFS->createFile(const_cast<char *>("/a"), 2);
  int fd = c2->myFS->openFile(const_cast<char *>("/b"), 2, 'w', -1);
  c2->myFS->writeFile(fd, const_cast<char *>("aaaabbbbcccc"), 12);
  c2->myFS->closeFile(fd);

  return 0;
}
