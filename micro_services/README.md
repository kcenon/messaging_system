## Microservice

There are tons of styles of microservice architecture. So, this sample of microservice may provide parts of style about how to design.

### File streaming system via Microservice

This sample provides a message network grid for microservice, and file transferring service between main_server and middle_server.  Basically, all messages can send to other programs even on another computer by this network grid, for example, only one target or all same name targets, etc. A computer installed middle_server can also handle the file transfer stream to main_server. So, each client only needs that do a request to middle_server and receive a result from middle_server without any concerns about it.

1. [main_server](https://github.com/kcenon/messaging_system/tree/main/micro_services/main_server)
2. [middle_server](https://github.com/kcenon/messaging_system/tree/main/micro_services/middle_server)