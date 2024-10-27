Caffe uses 'protobuf' format which is provided by Google.

To parse prototxt format, you have to install this protobuf and run our parser.

How to install (See more in https://github.com/google/protobuf/tree/master/src)

1. clone protobuf
$ git clone https://github.com/google/protobuf/.git

2. install required packages 
$ sudo apt-get install autoconf automake libtool curl make g++ unzip

3. install protobuf
$ cd protobuf
$ ./autogen.sh
$ ./configure --prefix=/usr
$ make
$ make check
$ sudo make install
$ sudo ldconfig # refresh shared library cache.

How to specify a network

Some requirements:

-Name of data layer must be "input"
-prototxts of solver & network should be located in caffe_parser dir
