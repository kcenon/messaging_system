/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2021, 🍀☀🌕🌥 🌊
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "job.h"

#include "converting.h"
#include "file_handler.h"
#include "folder_handler.h"
#include "job_pool.h"
#include "logging.h"

#include "fmt/format.h"
#include "fmt/xchar.h"

#include <chrono>
#include <future>

#include "crossguid/guid.hpp"

namespace threads {
using namespace logging;
using namespace converting;
using namespace file_handler;
using namespace folder_handler;

job::job(const priorities &priority)
    : _priority(priority), _temporary_stored_path(L""),
      _working_callback(nullptr), _working_callback2(nullptr),
      _working_callback3(nullptr) {}

job::job(const priorities &priority, const vector<uint8_t> &data)
    : _priority(priority), _data(data), _temporary_stored_path(L""),
      _working_callback(nullptr), _working_callback2(nullptr),
      _working_callback3(nullptr) {}

job::job(const priorities &priority,
         const function<void(void)> &working_callback)
    : _priority(priority), _temporary_stored_path(L""),
      _working_callback(working_callback), _working_callback2(nullptr),
      _working_callback3(nullptr) {}

job::job(const priorities &priority, const vector<uint8_t> &data,
         const function<void(const vector<uint8_t> &)> &working_callback)
    : _priority(priority), _data(data), _temporary_stored_path(L""),
      _working_callback(nullptr), _working_callback2(working_callback),
      _working_callback3(nullptr) {}

job::job(const priorities &priority, const vector<uint8_t> &data,
         const function<void(weak_ptr<job_pool> job_pool,
                             const vector<uint8_t> &)> &working_callback)
    : _priority(priority), _data(data), _temporary_stored_path(L""),
      _working_callback(nullptr), _working_callback2(nullptr),
      _working_callback3(working_callback) {}

job::~job(void) {}

shared_ptr<job> job::get_ptr(void) { return shared_from_this(); }

const priorities job::priority(void) { return _priority; }

void job::set_job_pool(shared_ptr<job_pool> job_pool) { _job_pool = job_pool; }

bool job::work(const priorities &worker_priority) {
  auto start = logger::handle().chrono_start();

  load();

  if (_working_callback != nullptr) {
    try {
      _working_callback();
    } catch (...) {
      logger::handle().write(
          logging_level::sequence,
          fmt::format(L"cannot complete working function on job: job "
                      L"priority[{}], worker priority[{}]",
                      (int)_priority, (int)worker_priority),
          start);

      return false;
    }

    destroy();

    logger::handle().write(
        logging_level::sequence,
        fmt::format(L"completed working callback function without value on "
                    L"job: job priority[{}], worker priority[{}]",
                    (int)_priority, (int)worker_priority),
        start);

    return true;
  }

  if (_working_callback2 != nullptr) {
    try {
      _working_callback2(_data);
    } catch (...) {
      logger::handle().write(
          logging_level::sequence,
          fmt::format(L"cannot complete working function on job: job "
                      L"priority[{}], worker priority[{}]",
                      (int)_priority, (int)worker_priority),
          start);

      return false;
    }

    destroy();

    logger::handle().write(
        logging_level::sequence,
        fmt::format(L"completed working callback function with value on job: "
                    L"job priority[{}], worker priority[{}]",
                    (int)_priority, (int)worker_priority),
        start);

    return true;
  }

  if (_working_callback3 != nullptr) {
    try {
      _working_callback3(_job_pool, _data);
    } catch (...) {
      logger::handle().write(
          logging_level::sequence,
          fmt::format(L"cannot complete working function on job: job "
                      L"priority[{}], worker priority[{}]",
                      (int)_priority, (int)worker_priority),
          start);

      return false;
    }

    destroy();

    logger::handle().write(
        logging_level::sequence,
        fmt::format(L"completed working callback function with value on job: "
                    L"job priority[{}], worker priority[{}]",
                    (int)_priority, (int)worker_priority),
        start);

    return true;
  }

  try {
    working(worker_priority);
  } catch (...) {
    logger::handle().write(
        logging_level::sequence,
        fmt::format(L"cannot complete working function on job: job "
                    L"priority[{}], worker priority[{}]",
                    (int)_priority, (int)worker_priority),
        start);

    return false;
  }

  destroy();

  logger::handle().write(logging_level::sequence,
                         fmt::format(L"completed working function on job: job "
                                     L"priority[{}], worker priority[{}]",
                                     (int)_priority, (int)worker_priority),
                         start);

  return true;
}

void job::save(const wstring &folder_name) {
  if (_data.empty()) {
    return;
  }

  wstring priority = L"";
  switch (_priority) {
  case priorities::low:
    priority = L"low";
    break;
  case priorities::normal:
    priority = L"normal";
    break;
  case priorities::high:
    priority = L"high";
    break;
  case priorities::top:
    priority = L"top";
    break;
  default:
    return;
  }

  _temporary_stored_path = fmt::format(
      L"{}{}/{}/{}.job", folder::get_temporary_folder(), folder_name, priority,
      converter::to_wstring(xg::newGuid().str()));

  file::save(_temporary_stored_path, _data);
  _data.clear();
}

void job::load(void) {
  if (_temporary_stored_path.empty()) {
    return;
  }

  _data = file::load(_temporary_stored_path);
}

void job::destroy(void) {
  if (_temporary_stored_path.empty()) {
    return;
  }

  file::remove(_temporary_stored_path);
  _temporary_stored_path.clear();
}

void job::working(const priorities &worker_priority) {
  logger::handle().write(
      logging_level::error,
      L"cannot complete job::working because it does not implemented");
}
} // namespace threads
