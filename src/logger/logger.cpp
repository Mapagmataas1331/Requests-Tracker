#include <iostream>
#include "logger.h"
#include "../utils/utils.h"
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <mutex>
#include <filesystem>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

using json = nlohmann::json;
std::mutex fileMutex;

int getOrAddIndex(json &arr, const std::string &val)
{
  for (size_t i = 0; i < arr.size(); ++i)
  {
    if (arr[i].get<std::string>() == val)
      return static_cast<int>(i);
  }
  arr.push_back(val);
  return static_cast<int>(arr.size() - 1);
}

void saveRequest(const std::string &request, SOCKET socket, const char *requestsFile)
{
  try
  {
    auto requestsDir = std::filesystem::path(requestsFile).parent_path();
    if (!requestsDir.empty())
    {
      std::filesystem::create_directories(requestsDir);
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "[LOGGER] Failed to create directories: " << e.what() << "\n";
    return;
  }

  sockaddr_in addr;
  int addrSize = sizeof(addr);
  getpeername(socket, (sockaddr *)&addr, &addrSize);

  char ipStr[INET6_ADDRSTRLEN] = {0};

#ifdef _WIN32
  InetNtopA(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr));
#else
  inet_ntop(AF_INET, &addr.sin_addr, ipStr, sizeof(ipStr));
#endif

  std::string method = extractMethod(request);
  std::string path = extractPath(request);
  std::string userAgent = extractHeaderValue(request, "User-Agent");
  std::string protocol = extractProtocol(request);

  std::lock_guard<std::mutex> lock(fileMutex);

  json data;

  try
  {
    std::ifstream inFile(requestsFile);
    if (inFile)
    {
      std::stringstream buffer;
      buffer << inFile.rdbuf();
      std::string content = buffer.str();

      if (!content.empty())
      {
        data = json::parse(content);
      }
      else
      {
        std::cerr << "[LOGGER] Warning: " << requestsFile << " is empty. Initializing...\n";
        data = json::object();
      }
    }
    else
    {
      std::cerr << "[LOGGER] File " << requestsFile << " does not exist. Initializing...\n";
      data = json::object();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "[LOGGER] Error reading/parsing " << requestsFile << ": " << e.what() << "\n";
    data = json::object();
  }

  if (!data.contains("senders"))
    data["senders"] = json::array();
  if (!data.contains("routes"))
    data["routes"] = json::array();
  if (!data.contains("methods"))
    data["methods"] = json::array();
  if (!data.contains("protocols"))
    data["protocols"] = json::array();
  if (!data.contains("user_agents"))
    data["user_agents"] = json::array();
  if (!data.contains("requests"))
    data["requests"] = json::array();

  int senderIndex = getOrAddIndex(data["senders"], ipStr);
  int routeIndex = getOrAddIndex(data["routes"], path);
  int methodIndex = getOrAddIndex(data["methods"], method);
  int protocolIndex = getOrAddIndex(data["protocols"], protocol);
  int userAgentIndex = getOrAddIndex(data["user_agents"], userAgent);

  data["requests"].push_back({getCurrentTimestamp(),
                              routeIndex,
                              senderIndex,
                              methodIndex,
                              protocolIndex,
                              userAgentIndex});

  try
  {
    std::string tempFile = std::string(requestsFile) + ".tmp";
    std::ofstream outFile(tempFile, std::ios::trunc);
    if (outFile.is_open())
    {
      outFile << data.dump(2);
      outFile.close();
      std::filesystem::rename(tempFile, requestsFile);
    }
    else
    {
      std::cerr << "[LOGGER] Error: Unable to write to temporary file " << tempFile << "\n";
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "[LOGGER] Failed to save JSON to file: " << e.what() << "\n";
  }
}
