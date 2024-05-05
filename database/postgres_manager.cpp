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

#include "postgres_manager.h"

#include "libpq-fe.h"

#include "converting.h"

namespace database
{
  using namespace converting;

  postgres_manager::postgres_manager(void) : _connection(nullptr) {}

  postgres_manager::~postgres_manager(void) {}

  database_types postgres_manager::database_type(void) { return database_types::postgres; }

  bool postgres_manager::connect(const wstring &connect_string)
  {
    _connection = PQconnectdb(converter::to_string(connect_string).c_str());
    if (PQstatus((PGconn *)_connection) != CONNECTION_OK)
    {
      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return false;
    }

    return true;
  }

  bool postgres_manager::create_query(const wstring &query_string)
  {
    PGresult *result = (PGresult *)query_result(query_string);
    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      PQclear(result);
      result = nullptr;

      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return false;
    }

    PQclear(result);
    result = nullptr;

    return true;
  }

  unsigned int postgres_manager::insert_query(const wstring &query_string)
  {
    PGresult *result = (PGresult *)query_result(query_string);
    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      PQclear(result);
      result = nullptr;

      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return 0;
    }

    unsigned int result_count = atoi(PQcmdTuples(result));

    PQclear(result);
    result = nullptr;

    return result_count;
  }

  unsigned int postgres_manager::update_query(const wstring &query_string)
  {
    PGresult *result = (PGresult *)query_result(query_string);
    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      PQclear(result);
      result = nullptr;

      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return 0;
    }

    unsigned int result_count = atoi(PQcmdTuples(result));

    PQclear(result);
    result = nullptr;

    return result_count;
  }

  unsigned int postgres_manager::delete_query(const wstring &query_string)
  {
    PGresult *result = (PGresult *)query_result(query_string);
    if (PQresultStatus(result) != PGRES_TUPLES_OK)
    {
      PQclear(result);
      result = nullptr;

      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return 0;
    }

    unsigned int result_count = atoi(PQcmdTuples(result));

    PQclear(result);
    result = nullptr;

    return result_count;
  }

  shared_ptr<container::value_container> postgres_manager::select_query(const wstring &query_string)
  {
    shared_ptr<container::value_container> container
        = make_shared<container::value_container>(L"query", vector<shared_ptr<container::value> >{});

    return container;
  }

  bool postgres_manager::disconnect(void)
  {
    if (_connection == nullptr)
    {
      return false;
    }

    PQfinish((PGconn *)_connection);
    _connection = nullptr;

    return true;
  }

  void *postgres_manager::query_result(const wstring &query_string)
  {
    if (_connection == nullptr)
    {
      return nullptr;
    }

    if (PQstatus((PGconn *)_connection) != CONNECTION_OK)
    {
      PQfinish((PGconn *)_connection);
      _connection = nullptr;

      return nullptr;
    }

    return PQexec((PGconn *)_connection, converter::to_string(query_string).c_str());
  }
}; // namespace database