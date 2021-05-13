## What is?
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

## How to build
Before building this project, you have to download and build vcpkg(https://github.com/Microsoft/vcpkg).
Secondly, should install libraries like below followed vcpkg install rule,

1. ASIO(https://github.com/chriskohlhoff/asio/) or Boost(https://github.com/boostorg) library
2. fmt(https://github.com/fmtlib/fmt) library
3. cryptopp(https://www.cryptopp.com/) library
4. lz4(https://github.com/lz4/lz4) library

* If you want to use Boost library instead of Asio library, you have to remove 'ASIO_STANDALONE' on preprocessor definition on network library.

After all installations, you can build this project with Visual Studio 2019.

## License

Note: This license has also been called the "New BSD License" or "Modified BSD License". See also the 2-clause BSD License.

Copyright 2021 🍀☀🌕🌥 🌊

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## Contact
Please report issues or questions here: https://github.com/kcenon/messaging_system/issues
