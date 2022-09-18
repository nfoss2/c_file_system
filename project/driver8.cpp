
/* Driver 8*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include "client.h"

using namespace std;

/*
  This driver will test the getAttributes() and setAttributes()
  functions. You need to complete this driver according to the
  attributes you have implemented in your file system, before
  testing your program.
  
  
  Required tests:
  get and set on the fs1 on a file
    and on a file that doesn't exist
    and on a file in a directory in fs1
    and on a file that doesn't exist in a directory in fs1

 fs2, fs3
  on a file both get and set on both fs2 and fs3

  samples are provided below.  Use them and/or make up your own.


*/

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
  Client *c4 = new Client(fs1);
  Client *c5 = new Client(fs2);



  int r;


  //int FileSystem::setAttribute(char *filename, int fnameLen, char attribute, int value) 
  //                                                          b:bookarkmed p:priority
 
/*------------------------Filesystem 1 tests-----------------------------------*/
  cout << "Begin driver8 - attribute tests" << endl;
  
  cout << "/*------------------------Filesystem 1 tests-----------------------------------*/" << endl;

  cout << "\n get/set on files in directories" << endl;

  r = c1->myFS->setAttributes(const_cast<char *>("/e/f"), 4,'b',1); // Bookmarked file, should return 0
  cout << "rv from setAttributes /e/f fs1 is " << r << (r==0 ? " correct bookmark set to 1": " fail") <<endl;
  r = c4->myFS->setAttributes(const_cast<char *>("/e/b"), 4,'p',5); // Set priority to 5, should return 0
  cout << "rv from setAttributes /e/b fs1 is " << r << (r==0 ? " correct priority set to 5": " fail") <<endl;
  r = c1->myFS->getAttributes(const_cast<char *>("/e/f"), 4,'b');   // Check bookmark, should return 1 
  cout << "rv from getAttributes /e/f fs1 is " << r << (r==1 ? " correct": " fail") <<endl;
  r = c4->myFS->getAttributes(const_cast<char *>("/e/b"), 4,'p');   // Check priority, should return 5
  cout << "rv from getAttributes /e/b fs1 is " << r << (r==5 ? " correct": " fail") <<endl;

  cout<< "\n get/set on files that dont exist" << endl;

  r = c1->myFS->setAttributes(const_cast<char *>("/e/q"), 4,'p',1);
  cout << "rv from getAttributes /e/q fs1 is " << r << (r==-2 ? " correct /e/q doesn't exist": " fail") <<endl;
  r = c1->myFS->getAttributes(const_cast<char *>("/e/q"), 4,'b');
  cout << "rv from getAttributes /e/q fs1 is " << r << (r==-2 ? " correct": " fail") <<endl;

  r = c1->myFS->getAttributes(const_cast<char *>("/p"), 2,'p');
  cout << "rv from getAttributes /p fs1 is " << r << (r==-2 ? " correct /p doesn't exist": " fail") <<endl;
  r = c4->myFS->setAttributes(const_cast<char *>("/p"), 2,'b',2);
  cout << "rv from setAttributes /p fs1 is " << r << (r==-2 ? " correct": " fail") <<endl;
  
cout << "\n/*------------------------Filesystem 2 tests-----------------------------------*/\n" << endl;

  r = c2->myFS->setAttributes(const_cast<char *>("/z"), 2,'b',5);   //Will fail, cant set bookmark to 5
  cout << "rv from setAttributes /f fs2 is " << r << (r==-3 ? " correct invalid bookmark value": " fail") <<endl;
  r = c5->myFS->setAttributes(const_cast<char *>("/z"), 2,'p',3);
  cout << "rv from setAttributes /z fs2 is " << r << (r==0 ? " correct priority set to 3": " fail") <<endl;
  r = c2->myFS->getAttributes(const_cast<char *>("/z"), 2,'b');     //Will fail, bookmark not set
  cout << "rv from getAttributes /f fs2 is " << r << (r==-3 ? " correct bookmark not set": " fail") <<endl;
  r = c5->myFS->getAttributes(const_cast<char *>("/z"), 2,'p');
  cout << "rv from getAttributes /z fs2 is " << r << (r==3 ? " correct": " fail") <<endl;

          
cout << "\n/*------------------------Filesystem 3 tests-----------------------------------*/\n" << endl;

  r = c3->myFS->setAttributes(const_cast<char *>("/o/o/o/a/l"), 10,'p',2);
  cout << "rv from setAttributes /o/o/o/a/l fs3 is " << r << (r==0 ? " correct priority set to 2": " fail") <<endl;
  r = c3->myFS->setAttributes(const_cast<char *>("/o/o/o/a/d"), 10,'a',20); //Will fail, attribute 'a' invalid
  cout << "rv from setAttributes /o/o/o/a/d fs3 is " << r << (r==-1 ? " correct attribute 'a' invalid": " fail") <<endl;
  r = c3->myFS->getAttributes(const_cast<char *>("/o/o/o/a/l"), 10,'p');
  cout << "rv from setAttributes /o/o/o/a/l fs3 is " << r << (r==2 ? " correct": " fail") <<endl;
  r = c3->myFS->getAttributes(const_cast<char *>("/o/o/o/a/d"), 10,'a');    //Will fail, attribute 'a' invalid
  cout << "rv from setAttributes /o/o/o/a/d fs3 is " << r << (r==-1 ? " correct": " fail") <<endl;


  cout << "End of driver 8\n" << endl;
  return 0;
}
