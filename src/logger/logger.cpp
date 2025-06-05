#include <iostream>
#include "logger.h"
#include "../utils/utils.h"
#include <json.hpp>
#include <fstream>
#include <mutex>

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
    std::cerr << "Failed to create directories: " << e.what() << "\n";
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
  std::ifstream inFile(requestsFile);
  if (inFile.is_open())
  {
    try
    {
      inFile >> data;
      if (!data.is_object())
      {
        data = json::object();
      }
    }
    catch (...)
    {
      data = json::object();
    }
    inFile.close();
  }
  else
  {
    data = json::object();
  }

  if (!data.contains("senders") || !data["senders"].is_array())
    data["senders"] = json::array();
  if (!data.contains("routes") || !data["routes"].is_array())
    data["routes"] = json::array();
  if (!data.contains("methods") || !data["methods"].is_array())
    data["methods"] = json::array();
  if (!data.contains("protocols") || !data["protocols"].is_array())
    data["protocols"] = json::array();
  if (!data.contains("user_agents") || !data["user_agents"].is_array())
    data["user_agents"] = json::array();
  if (!data.contains("requests") || !data["requests"].is_array())
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

  std::ofstream outFile(requestsFile, std::ios::trunc);
  if (outFile.is_open())
  {
    outFile << data.dump(4);
    outFile.close();
  }
  else
  {
    std::cerr << "Error: Unable to write to " << requestsFile << std::endl;
  }
}
