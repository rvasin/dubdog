# dubdog
An ultimate CLI duplicate file finder and remover!

This is a demo project to demonstrate my skills in C++11/14/17/20.
I intentionally put all code into just one main.cpp file because it's simple to review it by page scrolling.

While created as demo project it still could be really useful in to finding and removing duplicate files.

Watch the video tutorial on YouTube:
https://www.youtube.com/watch?v=qdoLNcz_zYg

Build it with 
```
 g++ main.cpp -o dubdog -O3 -s -llibboost_iostreams-mt -lssl -lcrypto
```

Usage:
```
dubdog path extensions [options]

Options:

-a [md5]|crc32    hash algorithm
-v                view only (don't ask for file removal)
```

extensions - is comma separated list of file's extensions.

For example when running on Windows you may run:
```
dubdog 'C:\Books' pdf,djvu,epub,fb2
```

Current version compares files by file's MD5 hash. Boost and OpenSSL libraries are used for reading the files and computing the hashes.
