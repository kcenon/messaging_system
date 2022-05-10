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
constexpr auto RESPONSE_EXTRACTION = L"response_extraction";
constexpr auto REQUEST_CONNECTION = L"request_connection";
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
constexpr auto CONNECTION_KEY = LCONNECTION_KEY;
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
constexpr auto RESPONSE_EXTRACTION = "response_extraction";
constexpr auto REQUEST_CONNECTION = "request_connection";
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