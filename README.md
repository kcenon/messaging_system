# messaging_system

Normally, it is pretty hard to implement a TCP server and client with a proper thread system in the C++ language. To support newbie's desire to make their own TCP communication system, I planned its project.

So, it will contain several systems like below,
1. File log system
2. Concurrency control by the thread pool system
3. Serializable data packet container
4. Multi-session TCP server
5. TCP Client

And, it will provide functions like below,
1. Callback functions for each sequence such as connection, receiving data and receiving file
2. Send packet to specific target client
3. Send packet to all connected clients
4. Send files between main server and middle server
5. Packet compress and encrypt

Before building this project, you have to download and build vcpkg(https://github.com/Microsoft/vcpkg).
Secondly, should install libraries like below,

1. ASIO(https://github.com/chriskohlhoff/asio/) or Boost(https://github.com/boostorg) library
2. fmt(https://github.com/fmtlib/fmt) library
3. cryptopp(https://www.cryptopp.com/) library
4. lz4(https://github.com/lz4/lz4) library
