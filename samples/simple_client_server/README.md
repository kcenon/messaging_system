# Simple Client-Server Sample

This sample demonstrates basic client-server communication using the messaging system.

## Features
- TCP/IP socket communication
- Basic message exchange between client and server
- Connection management

## Build
```bash
cd ../..
./build.sh --samples
```

## Run
```bash
# Start server
./build/bin/simple_client_server server

# In another terminal, start client
./build/bin/simple_client_server client
```