#include "bitvector.h"

#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H


using namespace std;

class DiskPartition {
  public:
    char partitionName;
    int partitionSize;
    //int superLoc = 0;
    
    //add variables as needed to the data structure here.
};

class DiskManager {
  Disk *myDisk;
  int partCount;
  DiskPartition *diskP;
  

  /* declare other private members here */

  public:
    DiskManager(Disk *d, int partCount, DiskPartition *dp);
    ~DiskManager();
    void intToChar(char * buffer, int num, int pos);
    int charToInt(char * buffer, int pos);
    void initBuffer(char* buffer);
    void bufferReset(char* buffer);
    bool isDiskBlank();
    int readDiskBlock(char partitionname, int blknum, char *blkdata);
    int writeDiskBlock(char partitionname, int blknum, char *blkdata);
    int getBlockSize() {return myDisk->getBlockSize();};
    int getPartitionSize(char partitionname);
    int findPartStart(char partitionname);
    /* declare other public members here  mine*/
  private:
    void buildSuperBlock();
};
#endif
