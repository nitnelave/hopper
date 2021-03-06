#include "test_utils/files.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdexcept>

namespace test {

testing::AssertionResult walk_directory(const char* folder,
                                        const TestFunction& tester) {
  DIR* dir;
  struct dirent* ent;
  if ((dir = opendir(folder)) == nullptr) {
    perror("opendir");
    exit(1);
  }
  /* print all the files and directories within directory */
  while ((ent = readdir(dir)) != nullptr) {
    if (ent->d_name[0] == '.') continue;
    std::string full_name = ent->d_name;
    full_name = folder + ("/" + full_name);
    struct stat statbuf;
    if (stat(full_name.c_str(), &statbuf) == -1) {
      perror("stat");
      exit(1);
    }
    testing::AssertionResult res(true);
    switch (statbuf.st_mode & S_IFMT) {
      case S_IFDIR:
        res = walk_directory(full_name.c_str(), tester);
        if (!res) return res;
        break;
      case S_IFREG:
        res = tester(full_name);
        if (!res) return res << "\nIn file: " << full_name << '\n';
        break;
      default:
        break;
    }
  }
  closedir(dir);
  return testing::AssertionSuccess();
}

}  // namespace test
