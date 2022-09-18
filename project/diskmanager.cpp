#include "disk.h"
#include "diskmanager.h"
#include "bitvector.h"
#include <bitset>
#include <iostream>
using namespace std;

DiskManager::DiskManager(Disk *d, int partcount, DiskPartition *dp)
{
  myDisk = d;
  partCount = partcount;
  int r = myDisk->initDisk();
  char buffer[64];
  bufferReset(buffer);
  int pos = 0;
  int i;
  int loc;
  if(r == 1){
    //Need to create disk
    diskP = dp;
    buildSuperBlock();
  }
  else{
  //disk exists already
    myDisk->readDiskBlock(0,buffer);
    if(buffer[0] == '#'){ //Disk exists but superblock is empty
      diskP = dp;
      buildSuperBlock();
    }
    else{
      int numParts = charToInt(buffer,pos);
      diskP = new DiskPartition[numParts];
      pos = 4;
      for(i = 0; i < numParts; i++){
        diskP[i].partitionName = buffer[pos++];
        diskP[i].partitionSize = charToInt(buffer,pos);
        pos += 4;
      }
    }
  }
}
/* else  read back the partition information from the DISK1 */

void DiskManager::buildSuperBlock(){
  char buf[64];
  bufferReset(buf);
  int pos = 0;
  int start = 1;
  intToChar(buf,partCount,0);
  pos += 4;
  for(int i = 0; i < partCount; i++){
    buf[pos++] = diskP[i].partitionName;
    intToChar(buf,diskP[i].partitionSize, pos);
    pos += 4;
  }
  myDisk->writeDiskBlock(0,buf);
}


void DiskManager::intToChar(char *buffer, int num, int pos)
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

int DiskManager::charToInt(char *buffer, int pos)
{
  int value = 0;
  int digit;
  char temp;
  int exp = 1000;
  for (int i = pos; i < (pos + 4); i++)
  {
    temp = buffer[i];
    digit = temp - '0';
    value += digit * exp;
    exp = exp / 10;
  }
  return value;
}

void DiskManager::bufferReset(char *buffer)
{
  // Initialize the buffer to be all '#'s
  for (int i = 0; i < 64; i++)
  {
    buffer[i] = '#';
  }
}
/*
void DiskManager::initBuffer(char *buffer)
{
  unsigned int bitvec = 0;
  // Initialize the buffer to be all '#'s
  for (int i = 0; i < 64; i++)
  {
    buffer[i] = '#';
  }
  // Output buffer
  //cout << "buffer: " << buffer << endl;
  BitVector bv(100);
  bv.setBitVector(&bitvec);
  for (int i = 0; i < 100; i++)
    buffer[i] = bv.testBit(i);
  //cout << "buffer: " << buffer << endl;
}
*/
/*
 *   returns: 
 *   0, if the block is successfully read;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds; (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::readDiskBlock(char partitionname, int blknum, char *blkdata)
{
  //check if the partition exists
  int size;
  int i;
  int pStart;
  size = getPartitionSize(partitionname);
  
  if(size == -1){ //partition doesnt exist
    return -3;
  }
  else if(size < blknum){ //blknum is out of bounds
    return -2;
  }
  pStart = findPartStart(partitionname) + blknum;
  return myDisk -> readDiskBlock(pStart, blkdata);
}

int DiskManager::findPartStart(char partitionname){
  int startloc = 1; //superblock is in loc 0 on disk{
  int i;
  for(i = 0; i < partCount; i++){
    if (partitionname == diskP[i].partitionName){
      return startloc;
    }
    else{
      startloc += diskP[i].partitionSize;
    }
  }
  return startloc;
}
/*
 *   returns: 
 *   0, if the block is successfully written;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds;  (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::writeDiskBlock(char partitionname, int blknum, char *blkdata)
{
  //check if the partition exists
  int size;
  int i;
  int pStart;
  size = getPartitionSize(partitionname);
  
  if(size == -1){ //partition doesnt exist
    return -3;
  }
  else if(size < blknum){ //blknum is out of bounds
    return -2;
  }
  pStart = findPartStart(partitionname) + blknum;
  return myDisk -> writeDiskBlock(pStart, blkdata);
}

/*
 * return size of partition
 * -1 if partition doesn't exist.
 */
int DiskManager::getPartitionSize(char partitionname)
{
  //search through diskP for the partitionName
  for (int i = 0; i < partCount; i++)
  {
    //once found, return diskP[theIndex].partitionSize
    //cout << diskP[i].partitionName << endl;
    if (diskP[i].partitionName == partitionname)
    {
      return diskP[i].partitionSize;
    }
  }
  return -1; //partition doesn't exist
}