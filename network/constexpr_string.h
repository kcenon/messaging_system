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

#ifdef _WIN32
constexpr auto HEADER = L"header";
constexpr auto DATA = L"data";
constexpr auto SOURCE_ID = L"source_id";
constexpr auto SOURCE_SUB_ID = L"source_sub_id";
constexpr auto TARGET_ID = L"target_id";
constexpr auto TARGET_SUB_ID = L"target_sub_id";
constexpr auto MESSAGE_TYPE = L"message_type";
constexpr auto INDICATION_ID = L"indication_id";
constexpr auto RESPONSE = L"response";
constexpr auto REQUEST_CONNECTION = L"request_connection";
constexpr auto CONFIRM_CONNECTION = L"confirm_connection";
constexpr auto RESULT = L"result";
constexpr auto RESULT_MESSAGE = L"result_message";
constexpr auto TRANSFER_CONDITON = L"transfer_condition";
constexpr auto REQUEST_FILE = L"request_file";
constexpr auto SNIPPING_TARGETS = L"snipping_targets";
constexpr auto ENCRYPT_MODE = L"encrypt_mode";
constexpr auto FILES = L"files";
constexpr auto SOURCE = L"source";
constexpr auto TARGET = L"target";
constexpr auto GATEWAY_SOURCE_ID = L"gateway_source_id";
constexpr auto GATEWAY_SOURCE_SUB_ID = L"gateway_source_sub_id";
constexpr auto CONNECTION_KEY = L"connection_key";
#else
constexpr auto HEADER = "header";
constexpr auto DATA = "data";
constexpr auto SOURCE_ID = "source_id";
constexpr auto SOURCE_SUB_ID = "source_sub_id";
constexpr auto TARGET_ID = "target_id";
constexpr auto TARGET_SUB_ID = "target_sub_id";
constexpr auto MESSAGE_TYPE = "message_type";
constexpr auto INDICATION_ID = "indication_id";
constexpr auto RESPONSE = "response";
constexpr auto REQUEST_CONNECTION = "request_connection";
constexpr auto CONFIRM_CONNECTION = "confirm_connection";
constexpr auto RESULT = "result";
constexpr auto RESULT_MESSAGE = "result_message";
constexpr auto TRANSFER_CONDITON = "transfer_condition";
constexpr auto REQUEST_FILE = "request_file";
constexpr auto SNIPPING_TARGETS = "snipping_targets";
constexpr auto ENCRYPT_MODE = "encrypt_mode";
constexpr auto FILES = "files";
constexpr auto SOURCE = "source";
constexpr auto TARGET = "target";
constexpr auto GATEWAY_SOURCE_ID = "gateway_source_id";
constexpr auto GATEWAY_SOURCE_SUB_ID = "gateway_source_sub_id";
constexpr auto CONNECTION_KEY = "connection_key";
#endif