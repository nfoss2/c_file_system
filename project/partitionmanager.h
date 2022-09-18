#ifndef PARTITION_MANAGER_H
#define PARTITION_MANAGER_H

#include "diskmanager.h"
#include "bitvector.h"


class PartitionManager {
  DiskManager *myDM;
 
  /* declare other private members here */


  public:
    char myPartitionName;
    int myPartitionSize;
    BitVector* bv;
    PartitionManager(DiskManager *dm, char partitionname, int partitionsize);
    ~PartitionManager();
    void intToChar(char * buffer, int num, int pos);
    int readDiskBlock(int blknum, char *blkdata);
    int writeDiskBlock(int blknum, char *blkdata);
    int getBlockSize();
    int getFreeDiskBlock();
    int returnDiskBlock(int blknum);
    void bufferReset(char *buffer);
    void initBuffer();
    bool doesPartitionExist();
    void getbv();
    /* declare other public members here */

};
#endif
