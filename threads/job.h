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

#pragma once

#include "job_priorities.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace threads
{
  class job_pool;
  class job : public std::enable_shared_from_this<job>
  {
  public:
    job(const priorities &priority);
    job(const priorities &priority, const std::vector<uint8_t> &data);
    job(const priorities &priority, const std::function<void(void)> &working_callback);
    job(const priorities &priority,
        const std::vector<uint8_t> &data,
        const std::function<void(const std::vector<uint8_t> &)> &working_callback);
    job(const priorities &priority,
        const std::vector<uint8_t> &data,
        const std::function<void(std::weak_ptr<job_pool> job_pool, const std::vector<uint8_t> &)> &working_callback);
    ~job(void);

  public:
    std::shared_ptr<job> get_ptr(void);

  public:
    const priorities priority(void);

  public:
    void set_job_pool(std::shared_ptr<job_pool> job_pool);

  public:
    bool work(const priorities &worker_priority);

  protected:
    void save(const std::wstring &folder_name);
    void load(void);
    void destroy(void);

  protected:
    virtual void working(const priorities &worker_priority);

  protected:
    std::vector<uint8_t> _data;
    std::weak_ptr<job_pool> _job_pool;

  private:
    priorities _priority;
    std::wstring _temporary_stored_path;

    std::function<void(void)> _working_callback;
    std::function<void(const std::vector<uint8_t> &)> _working_callback2;
    std::function<void(std::weak_ptr<job_pool>, const std::vector<uint8_t> &)> _working_callback3;
  };
} // namespace threads