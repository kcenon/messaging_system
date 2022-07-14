%module database

%{
#include "database_types.h"
#include "database_manager.h"
#include "postgres_manager.h"
%}

%include "database_types.h"
%include "database_manager.h"
%include "postgres_manager.h"