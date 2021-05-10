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
