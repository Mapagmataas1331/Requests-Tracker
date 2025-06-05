#include "server/server.h"
#include "utils/certgen.h"
#include <thread>

const char *CERT_FILE = "cert/cert.pem";
const char *KEY_FILE = "cert/key.pem";
const char *REQUESTS_FILE = "data/requests.json";

int main()
{
  ensureCertificateExists(CERT_FILE, KEY_FILE);

  std::thread http(runServer, 80, REQUESTS_FILE);
  std::thread https(runHttpsServer, 443, REQUESTS_FILE, CERT_FILE, KEY_FILE);

  http.join();
  https.join();
  return 0;
}