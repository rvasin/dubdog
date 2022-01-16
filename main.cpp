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
#include <openssl/md5.h>
//#include <boost/locale.hpp>
//#include <boost/filesystem/path.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;

typedef unsigned char * md5hash;

string CalcMD5(const string& filename)
{
   unsigned char result[MD5_DIGEST_LENGTH];
   boost::iostreams::mapped_file_source src(filename);
   MD5((unsigned char*)src.data(), src.size(), result);

   std::ostringstream sout;
   sout<<std::hex<<std::setfill('0');
   for(long long c: result)
   {
       sout<<std::setw(2)<<(long long)c;
   }
   return sout.str();
}


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
   //std::locale::global(boost::locale::generator().generate(""));
   //boost::filesystem::path::imbue(std::locale());
   auto t0 = chrono::high_resolution_clock::now();
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

   map<string,shared_ptr<DupList>> candidates;

   using rdi = std::filesystem::recursive_directory_iterator;
   for (const auto& entry : rdi(FilePath)) {
      string ext = entry.path().extension().string();
      if (entry.is_regular_file() && regex_match(ext, mask)) {
         //cout << entry.path().string() << endl;
         string md5sum = CalcMD5(entry.path().string());
         auto& dup = candidates[md5sum];
         if (dup == nullptr) {
               shared_ptr<DupList> newdup(new DupList());
               candidates[md5sum] = newdup;
               dup = candidates[md5sum];
         }
         dup->fsize = entry.file_size();
         dup->lst.push_back(entry.path().string());

      }
   }
   auto t1 = chrono::high_resolution_clock::now();
   int candcount{0};
   for (const auto& [md5, dup] : candidates) {
      if (dup->lst.size() > 1) candcount++;
   }
   cout << "Found " << candcount << " groups of duplicates in ";
   cout << setprecision(4) << chrono::duration_cast<chrono::milliseconds>(t1-t0).count()/1000.0 << " secs." << endl;
   cout << "Printing duplicates:" << endl;
   int no = 1;
   vector<string> flist;
   for (const auto& [md5, dup] : candidates) {
      if (dup->lst.size() > 1) {
         cout << "hash: " << md5 << ", size: " <<  dup->fsize << endl;
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
