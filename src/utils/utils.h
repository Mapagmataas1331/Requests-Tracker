#pragma once
#include <string>

std::string extractMethod(const std::string &request);
std::string extractPath(const std::string &request);
std::string extractProtocol(const std::string &request);
std::string extractHeaderValue(const std::string &request, const std::string &header);
std::string getCurrentTimestamp();
std::string loadStaticHTML(const std::string &path);