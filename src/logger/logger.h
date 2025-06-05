#pragma once
#include <string>
#include <winsock2.h>

void saveRequest(const std::string &request, SOCKET socket, const char *requestsFile);