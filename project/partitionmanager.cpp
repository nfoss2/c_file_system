#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "bitvector.h"
#include <iostream>
using namespace std;

PartitionManager::PartitionManager(DiskManager *dm, char partitionname, int partitionsize)
{
  myDM = dm;
  myPartitionName = partitionname;
  myPartitionSize = partitionsize;
  char buff[64];
  /* If needed, initialize bit vector to keep track of free and allocted
     blocks in this partition */
  bv = new BitVector(myPartitionSize);
  if (!doesPartitionExist())
  {
    // create a bitvector for the partition based on size of the partition
    char myBuffer[64];
    bufferReset(myBuffer);

    //for(int i = 0; i < myPartitionSize; i++) cout << bv->testBit(i);

    // cout << bv->testBit(myPartitionSize);
    //  for (int i = 0; i < myPartitionSize; i++) cout << bv->testBit(i);
    //  copy the bitvector into the buffer and write it out.
    bv->setBit(0);
    bv->setBit(1);
    bv->getBitVector((unsigned int *)myBuffer);
    myDM->writeDiskBlock(myPartitionName, 0, myBuffer);
    //we need a root directory added to block one for each partition here or in file system
  }
  else
  {
    BitVector *dmBV = new BitVector(myPartitionSize);
    char buffer[64];
    bufferReset(buffer);
    myDM->readDiskBlock(myPartitionName, 0, buffer);
    dmBV->setBitVector((unsigned int *)buffer);
    bv = dmBV;
    // getbv();
  }
}

PartitionManager::~PartitionManager()
{
}

void PartitionManager::intToChar(char *buffer, int num, int pos)
{
  string tmp;
  tmp = to_string(num);
  tmp.insert(0, 4 - tmp.size(), '0');
  char const *input = tmp.c_str();
  for (int i = 0; i < 4; i++)
  {
    buffer[pos + i] = input[i];
  }
}

/*
 * return blocknum, -1 otherwise
 */
int PartitionManager::getFreeDiskBlock()
{
  char buff[64];
  int temp;
  for (int i = 1; i < myPartitionSize; i++)
  {
    temp = bv->testBit(i);
    if (temp == 0)
    {
      //bufferReset(buff);
      //bv->setBit(i);
      //bv->getBitVector((unsigned int*) buff);
      //myDM->writeDiskBlock(myPartitionName,0,buff); //update bitvector
      //bufferReset(buff);
      //writeDiskBlock(i,buff); //write buffer to disk
      return i;
    }
  }
  return -1;
}

/*void PartitionManager::initBuffer()
{
  char buffer[64];
  unsigned int num = 0;
   // Initialize the buffer to be all '#'s
  for(int i = 0; i < 64; i++) {
    buffer[i]= '#';
  }
  // Output buffer
  //bv->setBitVector((unsigned int *)buffer);
  //bv->getBitVector((unsigned int *)buffer);
  //myDM->writeDiskBlock(myPartitionName, 0, buffer);
}*/

void PartitionManager::bufferReset(char *buffer)
{
  // Initialize the buffer to be all '#'s
  for (int i = 0; i < 64; i++)
  {
    buffer[i] = '#';
  }
}

/*
 * return 0 for sucess, -1 otherwise
 */
int PartitionManager::returnDiskBlock(int blknum)
{
  /* write the code for deallocating a partition block */
  char buff[64];
  bufferReset(buff);
  int test = myDM->writeDiskBlock(myPartitionName, blknum, buff);
  if (test == 0)
  {
    bv->resetBit(blknum);
    test = bv->testBit(blknum);
    bufferReset(buff);
    bv->getBitVector((unsigned int *)buff);
    myDM->writeDiskBlock(myPartitionName, 0, buff);

    if (test == 0)
    {
      return 0;
    }
  }
  return -1;
}

int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata)
{
  char buff[64];

  bufferReset(buff);
  bv->setBit(blknum);
  bv->getBitVector((unsigned int *)buff);
  myDM->writeDiskBlock(myPartitionName, 0, buff); //update bitvector


  return myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::getBlockSize()
{
  return myDM->getBlockSize();
}

bool PartitionManager::doesPartitionExist()
{
  char buffer[64];
  readDiskBlock(1, buffer);
  if (buffer[0] == '#')
  {
    return false;
  }
  return true;
}

void PartitionManager::getbv()
{
  char buffer[64];
  for (int i = 2; i < myPartitionSize; i++)
  {
    readDiskBlock(i, buffer);
    if (buffer[0] != '#')
    {
      bv->setBit(i);
    }
  }
  return;
}