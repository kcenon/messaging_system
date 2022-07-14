%module network

%{
#include "../container/value_types.h"
#include "../container/value.h"
#include "../container/container.h"
#include "session_types.h"
#include "connection_conditions.h"
#include "data_handling.h"
#include "messaging_client.h"
#include "messaging_server.h"
%}

%include "session_types.h"
%include "connection_conditions.h"
%include "messaging_client.h"
%include "messaging_server.h"