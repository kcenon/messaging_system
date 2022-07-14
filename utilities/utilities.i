%module utilities

%{
#include "logging_level.h"
#include "logging.h"
#include "file_handler.h"
#include "folder_handler.h"
#include "encrypting.h"
#include "datetime_handler.h"
#include "converting.h"
#include "compressing.h"
#include "binary_combiner.h"
#include "argument_parser.h"
%}

%include "logging_level.h"
%include "logging.h"
%include "file_handler.h"
%include "folder_handler.h"
%include "encrypting.h"
%include "datetime_handler.h"
%include "converting.h"
%include "compressing.h"
%include "binary_combiner.h"
%include "argument_parser.h"