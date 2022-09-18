
/* Driver 2*/

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
  
  for (i = 0; i < 37; i++) buf1[i] = 's';
  for (i = 0; i < 64; i++) buf2[i] = 'b';
  for (i = 0; i < 100; i++) buf3[i] = 'K';
  for (i = 0; i < 600; i++) buf4[i] = 'M';

  cout <<"begin driver 4\n";

  cout <<"rename test on fs2 and fs3\n";
  r = c2->myFS->renameFile(const_cast<char *>("/$"), 2, const_cast<char *>("/a"), 2);
  cout << "rv from renameFile /$ to /x fs2 is " << r <<(r==-1 ? " correct invalid name": " fail") <<endl;
  r = c3->myFS->renameFile(const_cast<char *>("/9"), 2, const_cast<char *>("/a"), 2);
  cout << "rv from renameFile /9 to /x fs3 is " << r <<(r==-1 ? " correct invalid name": " fail") <<endl;
  r = c2->myFS->renameFile(const_cast<char *>("/B"), 2, const_cast<char *>("/b"), 2);
  cout << "rv from renameFile /B to /b is " << r <<(r==0 ? " correct ": " fail") <<endl;
  r = c3->myFS->renameFile(const_cast<char *>("/B"), 2, const_cast<char *>("/b"), 2);
  cout << "rv from renameFile /B to /b is " << r <<(r==0 ? " correct ": " fail") <<endl;

  r = c2->myFS->renameFile(const_cast<char *>("/B"), 2, const_cast<char *>("/X"), 2);
  cout << "rv from renameFile /B to /X is " << r <<(r==-2 ? " correct": " fail") <<endl;
  r = c3->myFS->renameFile(const_cast<char *>("/B"), 2, const_cast<char *>("/X"), 2);
  cout << "rv from renameFile /B to /X is " << r <<(r==-2 ? " correct": " fail") <<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/B"), 2, 'w', -1);
  cout << "rv from openFile /B is fs2 " << f1 << (f1==-1 ? " correct": " fail")<<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/B"), 2, 'w', -1);
  cout << "rv from openFile /B is fs3 " << f1 << (f1==-1 ? " correct": " fail")<<endl;


  cout <<"\ndirectory edge test on fs2\n";  // then delete b, c, d,e, f,g,h, i,j,k,l
  f3 = c2->myFS->openFile(const_cast<char *>("/j"), 2, 'r', -1);
  cout << "rv from openFile /j r is (f3)" << f3 << (f3>0 ? " correct ": " fail") <<endl;

  r = c2->myFS->deleteFile(const_cast<char *>("/b"), 2);
  cout << "rv from deleteFile /b is " << r <<(r==0 ? " correct":" fail")<<endl;

  r = c2->myFS->deleteFile(const_cast<char *>("/j"), 2);
  cout << "rv from deleteFile /j is " << r <<(r==-2 ? " correct":" fail")<<endl;
  r = c2->myFS->closeFile(f3);
  cout << "rv from closeFile /j fs2 (f3) is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/j"), 2);
  cout << "rv from deleteFile /j is " << r <<(r==0 ? " correct":" fail")<<endl;
  
  r = c2->myFS->deleteFile(const_cast<char *>("/c"), 2);
  cout << "rv from deleteFile /c is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/d"), 2);
  cout << "rv from deleteFile /d is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/e"), 2);
  cout << "rv from deleteFile /e is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/f"), 2);
  cout << "rv from deleteFile /f is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/g"), 2);
  cout << "rv from deleteFile /g is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/h"), 2);
  cout << "rv from deleteFile /h is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/i"), 2);
  cout << "rv from deleteFile /i is " << r <<(r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/j"), 2);
  cout << "rv from deleteFile /j is " << r <<(r==-1 ? " correct":" fail")<<endl;
  r = c2->myFS->deleteFile(const_cast<char *>("/k"), 2);
  cout << "rv from deleteFile /k is " << r <<(r==0 ? " correct":" fail")<<endl;

  r = c2->myFS->deleteFile(const_cast<char *>("/l"), 2);
  cout << "rv from deleteFile /l is " << r <<(r==0 ? " correct":" fail")<<endl;


  cout <<"\n\ntruncate tests on fs2 and fs3\n";
  //truncate tests fs2 and 3
  f2 = c2->myFS->openFile(const_cast<char *>("/z"), 2, 'm', -1);
  cout << "rv from openFile /z m fs2 (f2) is " << f3 << (f3>0 ? " correct ": " fail") <<endl;
  f1 = c2->myFS->openFile(const_cast<char *>("/z"), 2, 'r', -1);
  cout << "rv from openFile /z r fs2 (f1) is " << f3 << (f3>0 ? " correct ": " fail") <<endl;
  f3 = c3->myFS->openFile(const_cast<char *>("/b"), 2, 'm', -1);
  cout << "rv from openFile /b fs3 m is (f3)" << f3 << (f3>0 ? " correct ": " fail") <<endl;


  r = c2->myFS->truncFile(f1, 0, 1);
  cout << "rv from truncFile /z fs2 f1 " << r << (r==-3 ? " correct open to read":" fail")<<endl;

  r = c3->myFS->readFile(f3, rbuf4, 50);
  cout << "rv from readFile /b fs3 (f3) is " << r << (r==40 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;
  r = c3->myFS->truncFile(f3, 0, 1);
  cout << "rv from truncFile /b fs3 (f3) seek to 0, removed " << r << (r==40 ? " correct":" fail")<<endl;
  r = c3->myFS->seekFile(f3, 0, 1);
  cout << "rv from seekFile /b is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c3->myFS->readFile(f3, rbuf4, 50);
  cout << "rv from readFile /b is " << r << (r==0 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf4[i];
  cout << endl;


  r = c2->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /z fs2 f2 is " << r << (r==1152 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;
  r = c2->myFS->truncFile(f2, 1088, 1);
  cout << "rv from truncFile /z f2, removed " << r << (r==128 ? " correct":" fail")<<endl;
  r = c2->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /z is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c2->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /z is " << r << (r==1088 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c2->myFS->truncFile(f2, 1024, 1);
  cout << "rv from truncFile /z f2, removed " << r << (r==64 ? " correct":" fail")<<endl;
  r = c2->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /z is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c2->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /z is " << r << (r==1024 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c2->myFS->truncFile(f2, 192, 1);
  cout << "rv from truncFile /z f2, removed " << r << (r==832  ? " correct":" fail")<<endl;
  r = c2->myFS->seekFile(f2, 0, 1);
  cout << "rv from seekFile /z is " << r <<(r==0 ? " correct ": " fail") << endl;
  r = c2->myFS->readFile(f2, rbuf5, 1152);
  cout << "rv from readFile /z is " << r << (r==192 ? " correct ": " fail") <<endl;
  cout << "Data read is " << endl;
  for (i = 0; i < r; i++) cout << rbuf5[i];
  cout << endl;

  r = c2->myFS->closeFile(f1);
  cout << "rv from closeFile /z f1 is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c2->myFS->closeFile(f2);
  cout << "rv from closeFile /z f2 is " << r << (r==0 ? " correct":" fail")<<endl;
  r = c3->myFS->closeFile(f3);
  cout << "rv from closeFile /b f3 is " << r << (r==0 ? " correct":" fail")<<endl;

  cout <<"End of driver 4\n";
  return 0;
}
