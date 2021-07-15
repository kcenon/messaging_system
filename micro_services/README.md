## Microservice

There are tons of styles of microservice architecture. So, this sample of microservice may provide parts of style about how to design.

### 1. File streaming system via Microservice

This sample provides a message network grid for microservice, and file transferring service between main_server and middle_server.  Basically, all messages can send to other clients connected to each middle_server even on another computer by this network grid, for example, only one target or all same name targets, etc. However, the special thing about it, a middle_server acts for the file transfer stream between main_server, instead of client connected to middle_server. So, each client only needs that do a request to middle_server and receive a result from middle_server without any concerns about transferring.

1. [main_server](https://github.com/kcenon/messaging_system/tree/main/micro_services/main_server) : the main_server acts for message transferring root. Each middle_server has connected to main_server and all messages have passed through the main_server to each middle_server standardly.
2. [middle_server](https://github.com/kcenon/messaging_system/tree/main/micro_services/middle_server): the middle_server acts as a gateway between the main_server and each client. For example, send and receive a message packet, and make a file transferring progress packet to send a client. 