# CurlDownloadManager

## How to gen

1. gen_vs_proj.bat
2. cd build
3. use vs2017 to open CurlDownloadManager.sln


## How to build

### jsoncpp

[jsoncpp/1.8.0](https://github.com/open-source-parsers/jsoncpp.git) 
1. mkdir builld
2. cd build
3. cmake .. -G "Visual Studio 15 2017 Win64"
4. cd ..

### openssl

[openssl-1.1.1d-how_to_build](https://blog.csdn.net/liang19890820/article/details/51658574)
1. [Download ActivePerl](http://www.activestate.com/activeperl/downloads)
2. [Download Nasm](http://www.nasm.us/)
3. perl Configure VC-WIN64A no-asm no-shared --prefix=D:\OpenSSL //static library
4. nmake
5. nmake test
6. nmake install

### zlib

[zlib-1.2.11](https://pkgs.org/download/zlib-devel)
Please visit zlib-1.2.11/win32/Makefile.msc

### curl

[curl/7.68.0](https://curl.haxx.se/download.html)
Please visit curl-7.68.0/winbuild/BUILD.WINDOWS.txt

