# dubdog
An ultimate CLI duplicate file finder and remover!

This is a demo project to demonstrate my skills in C++11/14/17/20.
I intentionally put all code into just one main.cpp file because it's simple to review it by page scrolling.

While created as demo project it still could be really useful in to finding and removing duplicate files.

Build it with 
```
g++ main.cpp -o dubdog
```

Usage:
```
dubdog path extensions
```

extensions - is comma separated list of file's extensions.

For example when running on Windows you may run:
```
dubdog 'C:\Books' pdf,djvu,epub,fb2
```

Current version compares files only by file's size (not MD5 or similar message digest). 
Thus we careful to use it on real files.

