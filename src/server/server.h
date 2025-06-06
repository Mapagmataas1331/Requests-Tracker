#pragma once

#include <atomic>

void runServer(int port, const char *requestsFile, std::atomic<bool> &running);
void runHttpsServer(int port, const char *requestsFile, const char *certFile, const char *keyFile, std::atomic<bool> &running);