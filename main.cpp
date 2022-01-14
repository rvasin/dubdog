#include <iostream>
#include <cstdint>
#include <thread>
#include <filesystem>
#include <vector>
#include <map>
#include <regex>
#include <memory>
#include <fstream>
#include <cstring>
#include <iomanip>

using namespace std;


class DupList
{
public:
   uintmax_t fsize;
   vector<string> lst;
};

int main(int argc, char *argv[])
{
   if (argc < 3) {
      cout << "Usage: " << endl;
      cout << "   dupdog path masks" << endl;
      cout << "Example:" << endl;
      cout << "   dupbog C:\\books pdf;djvu;epub;fb2" << endl;
      return 0;
   }
   cout << "Scanning for duplicate files..." << endl;

   string FilePath = argv[1];

   char *exts = argv[2];
   char *ext = strtok(exts, ",");
   string masks = "";
   while (ext) {
      if (masks.size() > 0) masks += "|";
      masks += "\\."+string(ext);
      ext = strtok(nullptr, ",");
   }

   regex mask {"("+masks+")"};
   string ExtMask;

   map<uintmax_t,shared_ptr<DupList>> candidates;

   using rdi = std::filesystem::recursive_directory_iterator;
   for (const auto& entry : rdi(FilePath)) {
      string ext = entry.path().extension().string();
      if (entry.is_regular_file() && regex_match(ext, mask)) {
         auto& dup = candidates[entry.file_size()];
         if (dup == nullptr) {
               shared_ptr<DupList> newdup(new DupList());
               candidates[entry.file_size()] = newdup;
               dup = candidates[entry.file_size()];
         }
         dup->fsize = entry.file_size();
         dup->lst.push_back(entry.path().string());

      }
   }

   cout << "Printing duplicates:" << endl;
   int no = 1;
   vector<string> flist;
   for (const auto& [fsize, dup] : candidates) {
      if (dup->lst.size() > 1) {
         cout << "size: " << fsize << endl;
         for (const auto& f : dup->lst) {
            cout << "#" << no << " " << f << endl;
            flist.push_back(f);
            no++;
         }
      }
   }
   cout << "Which files do you want to remove? Write down files numbers separated by space and press Enter" << endl;
   cout << "Or just press Enter if you don't want to remove any files." << endl;
   string filenumbers;
   cout << ">";
   std::getline(std::cin, filenumbers);

   if (filenumbers.size()>0) {
      char *fn = new char[filenumbers.length() + 1];
      strcpy(fn, filenumbers.c_str());
      char *num;
      num = strtok(fn," ");
      int i;
      while (num) {
         i = stoi(num) ;
         cout << "Removing file: #" << i << " " << flist[i-1] << endl;
         try {
            filesystem::remove(flist[i-1]);
         }
         catch (...) {
            cout << "Cannot remove file" << endl;
         }
         num = strtok(nullptr," ");
      }
      delete [] fn;
   }
   cout << "Done!" << endl;

   return 0;
}
