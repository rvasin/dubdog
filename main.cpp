#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <filesystem>
#include <vector>
#include <map>
#include <queue>
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
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include <boost/cstdint.hpp>  // for boost::uint16_t


using namespace std;

enum class HashAlgo {haMD5, haCRC32, haFSO};

// command line options:
HashAlgo ha = HashAlgo::haMD5;
bool ViewOnly = false;
int ThreadsCount = 0;

string CalcHash(const string& filename)
{
   ostringstream sout;

   boost::iostreams::mapped_file_source src(filename);

   if (ha==HashAlgo::haCRC32) {
      boost::crc_32_type result;
      result.process_bytes(src.data(), src.size());
      sout<<result.checksum();
   } else {
      // default: MD5
      unsigned char result[MD5_DIGEST_LENGTH];
      MD5((unsigned char*)src.data(), src.size(), result);
      sout<<hex<<setfill('0');
      for(long long c: result) {
          sout<<setw(2)<<(long long)c;
      }
   }

   return sout.str();
}

class DupList
{
public:
   uintmax_t fsize;
   vector<string> lst;
};

map<string,shared_ptr<DupList>> candidates;
queue<filesystem::directory_entry> jobs;
mutex m;

void ProcessJob()
{
   while (jobs.size()>0) {
      string filename;
      filesystem::directory_entry entry;
      {
         scoped_lock lock(m);
         entry = jobs.front();
         filename = entry.path().string();
         jobs.pop();
      }
      try {
         string md5sum;
         if (ha==HashAlgo::haFSO) {
            md5sum = to_string(entry.file_size());
         } else {
            md5sum = CalcHash(filename);
         }
         {
            scoped_lock lock(m);
            auto& dup = candidates[md5sum];
            if (!dup) {
                  dup = make_shared<DupList>(DupList());
                  candidates[md5sum] = dup;
            }
            dup->fsize = entry.file_size();
            dup->lst.push_back(filename);
         }
      }
      catch (...) {}
   }
}

int main(int argc, char *argv[])
{
   if (argc < 3) {
      cout << "Usage:" << endl;
      cout << "   dupdog path masks [options]" << endl << endl;
      cout << "Options:" << endl;
      cout << "    -a [md5]|crc32|fso    hash algorithm" << endl;
      cout << "    -t count              threads count" << endl;
      cout << "    -v                    view only (don't ask for file removal)" << endl << endl;
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

   // parse other command line params
   string param;
   for (int i=3; i<argc; i++) {
      param = argv[i];
      switch (param[1]) {
      case 'a': {
         string alg = static_cast<string>(argv[i+1]);
         if (alg=="crc32") {
            ha=HashAlgo::haCRC32;
         } else if (alg=="fso") {
            ha=HashAlgo::haFSO;
         }
         break;
      }
      case 't': {
         ThreadsCount = stoi(argv[i+1]);
         break;
      }
      case 'v': {
         ViewOnly=true;
         break;
      }
      }
   }

   using rdi = filesystem::recursive_directory_iterator;
   try {
   for (const auto& entry : rdi(FilePath)) {
      string ext = entry.path().extension().string();
      if (entry.is_regular_file() && regex_match(ext, mask)) {
         //cout << entry.path().string() << endl;
         jobs.push(entry);
      }
   }
   }
   catch (exception& e) {
      cout << "Problem with scanning:" << endl << e.what() << endl;
      return 1;
   }
   if (!ThreadsCount) ThreadsCount = thread::hardware_concurrency()*2;
   vector<shared_ptr<thread>> threads;
   for (int i = 0; i < ThreadsCount; i++) {
       threads.emplace_back(make_shared<thread>(ProcessJob));
   }
   for (auto &th : threads) {
     th->join();
   }
   auto t1 = chrono::high_resolution_clock::now();
   int candcount{0};
   for (const auto& [md5, dup] : candidates) {
      if (dup->lst.size() > 1) candcount++;
   }
   cout << "Found " << candcount << " groups of duplicates in ";
   cout << setprecision(4) << chrono::duration_cast<chrono::milliseconds>(t1-t0).count()/1000.0 << " secs." << endl;
   if (candcount) {
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
      if (!ViewOnly) {
         cout << "Which files do you want to remove? Write down files numbers separated by space and press Enter" << endl;
         cout << "Or just press Enter if you don't want to remove any files." << endl;
         string filenumbers;
         cout << ">";
         getline(cin, filenumbers);

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
      }
   }
   cout << "Done!" << endl;
   return 0;
}
