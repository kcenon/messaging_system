%module threads

%{
#include "job_priorities.h"
#include "job.h"
#include "job_pool.h"
#include "thread_worker.h"
#include "thread_pool.h"
%}

%include "job_priorities.h"
%include "job.h"
%include "job_pool.h"
%include "thread_worker.h"
%include "thread_pool.h"