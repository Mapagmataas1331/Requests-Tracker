#pragma once

void runServer(int port, const char *requestsFile);
void runHttpsServer(int port, const char *requestsFile, const char *certFile, const char *keyFile);