#include "server/server.h"
#include "utils/certgen.h"
#include <thread>
#include <atomic>
#include <iostream>

const char *CERT_FILE = "cert/cert.pem";
const char *KEY_FILE = "cert/key.pem";
const char *REQUESTS_FILE = "data/requests.json";

std::atomic<bool> running(true);

int main()
{
  ensureCertificateExists(CERT_FILE, KEY_FILE);

  std::thread http(runServer, 80, REQUESTS_FILE, std::ref(running));
  std::thread https(runHttpsServer, 443, REQUESTS_FILE, CERT_FILE, KEY_FILE, std::ref(running));

  std::cout << "[MAIN] Press 'q' and Enter to stop the server gracefully.\n";

  std::thread watcher([]
                      {
    std::string cmd;
    while (running)
    {
      std::getline(std::cin, cmd);
      if (cmd == "q" || cmd == "Q")
      {
        std::cout << "[MAIN] Shutdown signal received.\n";
        running = false;
        break;
      }
    } });

  http.join();
  https.join();
  watcher.join();
  return 0;
}
