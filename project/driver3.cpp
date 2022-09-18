
/* Driver 3*/

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

  DiskManager *dm = new DiskManager(d, 3, dp);
  FileSystem *fs1 = new FileSystem(dm, 'A');
  FileSystem *fs2 = new FileSystem(dm, 'B');
  FileSystem *fs3 = new FileSystem(dm, 'C');
  Client *c1 = new Client(fs1);
  Client *c2 = new Client(fs2);
  Client *c3 = new Client(fs3);

  int i, r, l1, l2, f1, f2, f3, f4, f5;
  char buf1[37], buf2[64], buf3[100], buf4[600], buf5[1200];
  char rbuf1[37], rbuf2[64], rbuf3[100], rbuf4[600], rbuf5[1200];
  

  cout <<"begin driver 3\n";

  cout <<"rename test on fs1 (need driver1 first)\n";
  r = c1->myFS->renameFile(const_cast<char *>("/$"), 2, const_cast<char *>("/a"), 2);
  cout << "rv from renameFile /$ to /x is " << r <<(r==-1 ? " correct invalid name": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/x"), 2, const_cast<char *>("/a"), 2);
  cout << "rv from renameFile /x to /a is " << r <<(r==-2 ? " correct file doesn't exist": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/A"), 2, const_cast<char *>("/a"), 2);
  cout << "rv from renameFile /A to /a is " << r <<(r==-3 ? " correct /A already exists": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/A"), 2, const_cast<char *>("/x"), 2);
  cout << "rv from renameFile /A to /x is " << r <<(r==0 ? " correct": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/A"), 2, const_cast<char *>("/x"), 2);
  cout << "rv from renameFile /A to /x is " << r <<(r==-2 ? " correct /A doesn't exist": " fail") <<endl;
  f1 = c1->myFS->openFile(const_cast<char *>("/A"), 2, 'w', -1);
  cout << "rv from openFile /A is " << f1 << (f1==-1 ? " correct": " fail")<<endl;

  f1 = c1->myFS->openFile(const_cast<char *>("/a"), 2, 'w', -1);
  cout << "rv from openFile /a is " << f1 << (f1>0 ? " correct": " fail")<<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/a"), 2, const_cast<char *>("/y"), 2);
  cout << "rv from renameFile /a to /y is " << r <<(r==-4 ? " correct /a is open": " fail")  <<endl;
  r = c1->myFS->closeFile(f1);
  cout << "rv from closeFile /a is " << r <<(r==0 ? " correct": " fail") <<endl;

  l2 = c1->myFS->lockFile(const_cast<char *>("/b"), 2);
  cout << "rv from lockFile /b is " << l2 << (l2>0 ? " correct": " failure") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/b"), 2, const_cast<char *>("/g"), 2);
  cout << "rv from renameFile /b to /g is " << r <<(r==-4 ? " correct locked": " fail")  <<endl;
  r = c1->myFS->unlockFile( const_cast<char *>("/b"),2,l2);
  cout << "rv from unlockFile /b is " << r << (r==0 ? " correct ": " failure") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/b"), 2, const_cast<char *>("/g"), 2);
  cout << "rv from renameFile /b to /g is " << r <<(r==0 ? " correct": " fail")  <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/b"), 2, const_cast<char *>("/y"), 2);
  cout << "rv from renameFile /b to /y is " << r << (r==-2? " correct": " fail") <<endl;

  r = c1->myFS->createFile(const_cast<char *>("/g"), 2);
  cout << "rv from createFile /g is " << r <<(r==-1 ? " correct ": " fail") <<endl;
  r = c1->myFS->createFile(const_cast<char *>("/b"), 2);
  cout << "rv from createFile /b is " << r <<(r==0 ? " correct ": " fail") <<endl;
  r = c1->myFS->renameFile(const_cast<char *>("/b"), 2, const_cast<char *>("/h"), 2);
  cout << "rv from renameFile /b to /h is " << r <<(r==0 ? " correct": " fail")  <<endl;



  cout <<"\ndelete testing \n";
  r = c1->myFS->deleteFile(const_cast<char *>("/$"), 2);
  cout << "rv from deleteFile /$ is " << r <<(r==-3 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("b"), 1);
  cout << "rv from deleteFile b is " << r <<(r==-3 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/bb"), 3);
  cout << "rv from deleteFile /bb is " << r <<(r==-3 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/k"), 2);
  cout << "rv from deleteFile /k is " << r <<(r==-1 ? " correct k doesn't exist":" fail")<<endl;

  r = c1->myFS->deleteFile(const_cast<char *>("/c"), 2);
  cout << "rv from deleteFile /c is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/e"), 2);
  cout << "rv from deleteFile /e is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/e"), 2);
  cout << "rv from deleteFile /e is " << r <<(r==-1 ? " correct":" fail")<<endl;

  f1 = c1->myFS->openFile(const_cast<char *>("/e"), 2, 'w', -1);
  cout << "rv from openFile /e is " << f1 << (f1==-1 ? " correct e was deleted": " fail")<<endl;
  f1 = c1->myFS->openFile(const_cast<char *>("/g"), 2, 'w', -1);
  cout << "rv from openFile /g is " << f1 << (f1>0 ? " correct": " fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/g"), 2);
  cout << "rv from deleteFile /g is " << r <<(r==-2 ? " correct /g is locked":" fail")<<endl;
  r = c1->myFS->closeFile(f1);
  cout << "rv from closeFile /g is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/g"), 2);
  cout << "rv from deleteFile /g is " << r <<(r==0 ? " correct":" fail")<<endl;

  l2 = c1->myFS->lockFile(const_cast<char *>("/C"), 2);
  cout << "rv from lockFile /C is " << l2 << (l2>0 ? " correct": " fail") <<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/C"), 2);
  cout << "rv from deleteFile /C is " << r <<(r==-2 ? " correct /C is locked":" fail")<<endl;
  r = c1->myFS->unlockFile( const_cast<char *>("/C"),2,l2);
  cout << "rv from unlockFile /C is " << r << (r==0 ? " correct ": " fail") <<endl;
  r = c1->myFS->deleteFile(const_cast<char *>("/C"), 2);
  cout << "rv from deleteFile /C is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->createFile(const_cast<char *>("/c"), 2);
  cout << "rv from createFile /c is " << r <<(r==0 ? " correct ": " fail") <<endl;


  cout <<"\n truncate tests\n";
  for (i = 0; i < 37; i++) buf1[i] = 's';
  for (i = 0; i < 64; i++) buf2[i] = 'b';
  for (i = 0; i < 100; i++) buf3[i] = 'K';
  for (i = 0; i < 600; i++) buf4[i] = 'M';

  f1 = c1->myFS->openFile(const_cast<char *>("/D"), 2, 'w', -1);
  cout << "rv from openFile /D w is " << f1 << (f1>0 ? " correct file openned (w) f1": " fail") <<endl;
  f2 = c1->myFS->openFile(const_cast<char *>("/d"), 2, 'm', -1);
  cout << "rv from openFile /b is " << f2 << (f2>0 ? " Correct file opened (m) f2 ": " failure") <<endl;
  r = c1->myFS->writeFile(f1, buf1, 5);
  cout << "rv from writeFile /a f1 is " << r <<(r==5 ? " Correct wrote 5 s": " failure") <<endl;
  r = c1->myFS->writeFile(f1, buf2, 5);
  cout << "rv from writeFile /a f1 is " << r <<(r==5 ? " Correct wrote 5 b": " failure") <<endl;
  r = c1->myFS->writeFile(f1, buf3, 5);
  cout << "rv from writeFile /a f1 is " << r <<(r==5 ? " Correct wrote 5 K": " failure") <<endl;
  r = c1->myFS->writeFile(f1, buf4, 5);
  cout << "rv from writeFile /a f1 is " << r <<(r==5 ? " Correct wrote 5 M": " failure") <<endl;
  r = c1->myFS->appendFile(f1, buf1, 5);
  cout << "rv from appendFile /a f1 is " << r <<(r==5 ? " Correct wrote 5 s": " failure") <<endl;
  r = c1->myFS->writeFile(f2, buf4, 600);
  cout << "rv from writeFile /b f2 is " << r <<(r==600 ? " Correct wrote 600 M": " failure") <<endl;
  r = c1->myFS->writeFile(f2, buf4, 552);
  cout << "rv from writeFile /b f2 is " << r <<(r==552 ? " Correct wrote 552 M (full!)": " failure") <<endl;
  r = c1->myFS->closeFile(f1);
  cout << "rv from closeFile /a fs1 is " << r << (r==0 ? " correct":" fail")<<endl;

  f1 = c1->myFS->openFile(const_cast<char *>("/D"), 2, 'r', -1);
  cout << "rv from openFile /D r is " << f1 << (f1>0 ? " correct file opened (r) f1": " fail") <<endl;

  r = c1->myFS->truncFile(f1, 0, 0);
  cout << "rv from truncFile /D f1 is " << r << (r==-3 ? " correct mode is r":" fail")<<endl;
  r = c1->myFS->truncFile(3012, 0, 0);
  cout << "rv from truncFile ?? f1 is " << r << (r==-1 ? " correct no open file":" fail")<<endl;
  r = c1->myFS->truncFile(f2, -10 , -1);
  cout << "rv from truncFile /a f3 is " << r << (r==-1 ? " Correct failed to set rw to -10": " failure") <<endl;
  r = c1->myFS->closeFile(f1);
  cout << "rv from closeFile /a f1 is " << r << (r==0 ? " correct":" fail")<<endl;
  f1 = c1->myFS->openFile(const_cast<char *>("/D"), 2, 'm', -1);
  cout << "rv from openFile /D w is " << f1 << (f1>0 ? " correct file openned (m) f1": " fail") <<endl;



  r = c1->myFS->truncFile(f1, 25, 1);
  cout << "rv from truncFile /D f1, seek 20 is " << r << (r==0 ? " correct size and rw are same":" fail")<<endl;
  r = c1->myFS->seekFile(f1, 0, 1);
  cout << "rv from seekFile /D is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f1, rbuf4, 25);
  cout << "rv from readFile /D is " << r << (r==25 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;

  r = c1->myFS->truncFile(f1, 20, 1);
  cout << "rv from truncFile /D f1, seek 20 is " << r << (r==5 ? " correct deleted last 5":" fail")<<endl;
  r = c1->myFS->seekFile(f1, 0, 1);
  cout << "rv from seekFile /D is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f1, rbuf4, 25);
  cout << "rv from readFile /D is " << r << (r==20 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;

  r = c1->myFS->truncFile(f1, 15, 1);
  cout << "rv from truncFile /D f1, seek 15 is " << r << (r==5 ? " correct deleted last 5":" fail")<<endl;
  r = c1->myFS->seekFile(f1, 0, 1);
  cout << "rv from seekFile /D is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f1, rbuf4, 25);
  cout << "rv from readFile /D is " << r << (r==15 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;

  r = c1->myFS->truncFile(f1, -15, 0);
  cout << "rv from truncFile /D f1, removed " << r << (r==15 ? " correct emptied file":" fail")<<endl;
  r = c1->myFS->seekFile(f1, 0, 1);
  cout << "rv from seekFile /D is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f1, rbuf4, 25);
  cout << "rv from readFile /D is " << r << (r==0 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 1151, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==1 ? " correct 1 byte removed":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /d is " << r << (r==1151 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 1088, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==63 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /d is " << r << (r==1088 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 1024, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==64 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /d is " << r << (r==1024 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 256, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==768  ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /d is " << r << (r==256 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 192, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==64 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf4, 600);
  cout << "rv from readFile /d is " << r << (r==192 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 64, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==128 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf3, 100);
  cout << "rv from readFile /d is " << r << (r==64 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf3[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 32, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==32 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf2, 64);
  cout << "rv from readFile /d is " << r << (r==32 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf2[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 8, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==24 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf1, 30);
  cout << "rv from readFile /d is " << r << (r==8 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf1[i];
  cout << endl;

  r = c1->myFS->truncFile(f2, 1, 1);
  cout << "rv from truncFile /d f2, removed " << r << (r==7 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf1, 30);
  cout << "rv from readFile /d is " << r << (r==1 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf1[i];
  cout << endl;


  cout <<"\n multi file open tests\n";
  f3 = c1->myFS->openFile(const_cast<char *>("/d"), 2, 'm', -1);
  cout << "rv from openFile /d is " << f3 << (f3>0 ? " Correct file opened (m) f3 ": " failure") <<endl;
  
  r = c1->myFS->writeFile(f2, buf1, 4);
  cout << "rv from writeFile /d f2 is " << r <<(r==4 ? " Correct wrote 4 s": " failure") <<endl;
  r = c1->myFS->seekFile(f3, 5, 1);
  cout << "rv from seekFile /d is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->writeFile(f3, buf2, 5);
  cout << "rv from writeFile /d f3 is " << r <<(r==5 ? " Correct wrote 5 b": " failure") <<endl;
  r = c1->myFS->appendFile(f2, buf1, 5);
  cout << "rv from appendFile /d f2 is " << r <<(r==5 ? " Correct wrote 5 s": " failure") <<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d f2 is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf1, 30);
  cout << "rv from readFile /d f2 is " << r << (r==15 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf1[i];
  cout << endl;
  r = c1->myFS->truncFile(f3, 10, 1);
  cout << "rv from truncFile /d f3, removed " << r << (r==5 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d f2 is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf2, 30);
  cout << "rv from readFile /d f2 is " << r << (r==10 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf2[i];
  cout << endl;

  r = c1->myFS->appendFile(f2, buf4, 5);
  cout << "rv from appendFile /d f2 is " << r <<(r==5 ? " Correct wrote 5 M": " failure") <<endl;
  r = c1->myFS->truncFile(f3, 10, 1);
  cout << "rv from truncFile /d f3, removed " << r << (r==5 ? " correct":" fail")<<endl;
  r = c1->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /d f2 is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c1->myFS->readFile(f2, rbuf2, 30);
  cout << "rv from readFile /d f2 is " << r << (r==10 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf2[i];
  cout << endl;

  r = c1->myFS->closeFile(f1);
  cout << "rv from closeFile /D f1 is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->closeFile(f2);
  cout << "rv from closeFile /d f2 is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c1->myFS->closeFile(f3);
  cout << "rv from closeFile /d f3 is " << r << (r==0 ? " correct":" fail")<<endl;
  cout <<"End of driver 3\n";
  return 0;
}
