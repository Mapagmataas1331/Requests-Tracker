#include "server.h"
#include "../logger/logger.h"
#include "../utils/utils.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ws2_32.lib")

void handleClient(SOCKET clientSocket, const char *requestsFile);
void handleHttpsClient(SSL *ssl, const char *requestsFile);

void runServer(int port, const char *requestsFile)
{
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(static_cast<u_short>(port));
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
  listen(listenSocket, SOMAXCONN);

  std::cout << "HTTP server started on port " << port << std::endl;

  while (true)
  {
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    std::thread(handleClient, clientSocket, requestsFile).detach();
  }

  closesocket(listenSocket);
  WSACleanup();
}

void runHttpsServer(int port, const char *requestsFile, const char *certFile, const char *keyFile)
{
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();

  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  SSL_CTX_use_certificate_file(ctx, certFile, SSL_FILETYPE_PEM);
  SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM);

  SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(static_cast<u_short>(port));
  serverAddr.sin_addr.s_addr = INADDR_ANY;

  bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr));
  listen(listenSocket, SOMAXCONN);

  std::cout << "HTTPS server started on port " << port << std::endl;

  while (true)
  {
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, static_cast<int>(clientSocket));
    SSL_accept(ssl);
    std::thread(handleHttpsClient, ssl, requestsFile).detach();
  }

  closesocket(listenSocket);
  SSL_CTX_free(ctx);
  EVP_cleanup();
}

void handleClient(SOCKET clientSocket, const char *requestsFile)
{
  sockaddr_in clientAddr{};
  int addrSize = sizeof(clientAddr);
  getpeername(clientSocket, (sockaddr *)&clientAddr, &addrSize);

  char ipStr[INET_ADDRSTRLEN] = {};
  inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
  // std::cout << "Request from " << ipStr << ":" << ntohs(clientAddr.sin_port) << std::endl;

  char buffer[4096]{};
  int bytes = recv(clientSocket, buffer, static_cast<int>(sizeof(buffer)), 0);
  if (bytes <= 0)
  {
    closesocket(clientSocket);
    return;
  }

  std::string request(buffer, bytes);
  saveRequest(request, clientSocket, requestsFile);

  std::string html = loadStaticHTML("static/index.html");
  std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + html;
  send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
  closesocket(clientSocket);
}

void handleHttpsClient(SSL *ssl, const char *requestsFile)
{
  SOCKET clientSocket = static_cast<SOCKET>(SSL_get_fd(ssl));
  sockaddr_in clientAddr{};
  int addrSize = sizeof(clientAddr);
  getpeername(clientSocket, (sockaddr *)&clientAddr, &addrSize);

  char ipStr[INET_ADDRSTRLEN] = {};
  inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
  // std::cout << "Request from " << ipStr << ":" << ntohs(clientAddr.sin_port) << std::endl;

  char buffer[4096]{};
  int bytes = SSL_read(ssl, buffer, static_cast<int>(sizeof(buffer)));
  if (bytes <= 0)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return;
  }

  std::string request(buffer, bytes);
  saveRequest(request, clientSocket, requestsFile);

  std::string html = loadStaticHTML("static/index.html");
  std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + html;
  SSL_write(ssl, response.c_str(), static_cast<int>(response.size()));

  SSL_shutdown(ssl);
  SSL_free(ssl);
}
