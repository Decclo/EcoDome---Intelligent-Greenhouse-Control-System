#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;

inline bool exists_test3 (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

int main()
{
    cout << exists_test3("./Data") << endl;
}