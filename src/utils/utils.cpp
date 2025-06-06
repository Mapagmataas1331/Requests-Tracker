#include "utils.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <ctime>

std::string extractMethod(const std::string &request)
{
  size_t pos = request.find(' ');
  if (pos == std::string::npos)
    return "";
  return request.substr(0, pos);
}

std::string extractPath(const std::string &request)
{
  size_t start = request.find(' ');
  if (start == std::string::npos)
    return "";
  size_t end = request.find(' ', start + 1);
  if (end == std::string::npos)
    return "";
  return request.substr(start + 1, end - (start + 1));
}

std::string extractProtocol(const std::string &request)
{
  std::istringstream stream(request);
  std::string method, path, protocol;
  stream >> method >> path >> protocol;
  return protocol.empty() ? "" : protocol;
}

std::string extractHeaderValue(const std::string &request, const std::string &header)
{
  std::istringstream stream(request);
  std::string line;
  std::string headerLower = header + ":";
  std::transform(headerLower.begin(), headerLower.end(), headerLower.begin(), [](unsigned char c)
                 { return static_cast<char>(std::tolower(c)); });

  while (std::getline(stream, line))
  {
    std::string lineLower = line;
    std::transform(lineLower.begin(), lineLower.end(), lineLower.begin(), [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });

    if (lineLower.find(headerLower) == 0)
    {
      size_t pos = line.find(':');
      if (pos != std::string::npos)
      {
        size_t start = pos + 1;
        while (start < line.size() && line[start] == ' ')
          start++;
        return line.substr(start);
      }
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