#include "utils.h"
#include <sstream>
#include <fstream>
#include <ctime>

std::string extractMethod(const std::string &request)
{
  return request.substr(0, request.find(' '));
}

std::string extractPath(const std::string &request)
{
  size_t start = request.find(' ') + 1;
  return request.substr(start, request.find(' ', start) - start);
}

std::string extractProtocol(const std::string &request)
{
  std::istringstream stream(request);
  std::string method, path, protocol;
  stream >> method >> path >> protocol;
  return protocol;
}

std::string extractHeaderValue(const std::string &request, const std::string &header)
{
  std::istringstream stream(request);
  std::string line;
  while (std::getline(stream, line))
  {
    if (line.find(header) != std::string::npos)
    {
      return line.substr(line.find(':') + 2); // Skip colon and space
    }
  }
  return "";
}

std::string getCurrentTimestamp()
{
  time_t now = time(nullptr);
  tm localTime;
  localtime_s(&localTime, &now);
  char buf[64];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &localTime);
  return buf;
}

std::string loadStaticHTML(const std::string &path)
{
  std::ifstream file(path);
  return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}