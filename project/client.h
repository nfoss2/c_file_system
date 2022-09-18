#ifndef CLIENT_H
#define CLIENT_H

class Client {
  public:
    FileSystem *myFS;
    Client(FileSystem *fs){myFS = fs;};
};
#endif
