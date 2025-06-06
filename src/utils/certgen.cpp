#define _CRT_SECURE_NO_WARNINGS

#include "certgen.h"

#include <filesystem>
#include <iostream>

#include <openssl/applink.c>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

void ensureCertificateExists(const std::string &certPath, const std::string &keyPath)
{
  if (std::filesystem::exists(certPath) && std::filesystem::exists(keyPath))
  {
    std::cout << "[CERTGEN] Files already exist, skipping generation\n";
    return;
  }

  try
  {
    auto keyDir = std::filesystem::path(keyPath).parent_path();
    if (!keyDir.empty())
    {
      std::filesystem::create_directories(keyDir);
    }

    auto certDir = std::filesystem::path(certPath).parent_path();
    if (!certDir.empty())
    {
      std::filesystem::create_directories(certDir);
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << "[CERTGEN] Failed to create directories: " << e.what() << "\n";
    return;
  }

  EVP_PKEY *pkey = nullptr;
  X509 *x509 = nullptr;

  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
  if (!pctx)
  {
    std::cerr << "[CERTGEN] EVP_PKEY_CTX_new_id failed\n";
    ERR_print_errors_fp(stderr);
    return;
  }

  if (EVP_PKEY_keygen_init(pctx) <= 0)
  {
    std::cerr << "[CERTGEN] EVP_PKEY_keygen_init failed\n";
    ERR_print_errors_fp(stderr);
    EVP_PKEY_CTX_free(pctx);
    return;
  }

  if (EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048) <= 0)
  {
    std::cerr << "[CERTGEN] EVP_PKEY_CTX_set_rsa_keygen_bits failed\n";
    ERR_print_errors_fp(stderr);
    EVP_PKEY_CTX_free(pctx);
    return;
  }

  if (EVP_PKEY_keygen(pctx, &pkey) <= 0)
  {
    std::cerr << "[CERTGEN] EVP_PKEY_keygen failed\n";
    ERR_print_errors_fp(stderr);
    EVP_PKEY_CTX_free(pctx);
    return;
  }
  EVP_PKEY_CTX_free(pctx);

  x509 = X509_new();
  if (!x509)
  {
    std::cerr << "[CERTGEN] X509_new failed\n";
    ERR_print_errors_fp(stderr);
    EVP_PKEY_free(pkey);
    return;
  }

  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 60L * 60 * 24 * 365); // 1 year
  X509_set_pubkey(x509, pkey);

  X509_NAME *name = X509_get_subject_name(x509);
  X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"RU", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"ma.cyou", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"localhost", -1, -1, 0);
  X509_set_issuer_name(x509, name);

  if (!X509_sign(x509, pkey, EVP_sha256()))
  {
    std::cerr << "[CERTGEN] X509_sign failed\n";
    ERR_print_errors_fp(stderr);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return;
  }

  FILE *pkey_file = nullptr;
  if (fopen_s(&pkey_file, keyPath.c_str(), "wb") != 0 || !pkey_file)
  {
    std::cerr << "[CERTGEN] Failed to open key file for writing: " << keyPath << "\n";
    ERR_print_errors_fp(stderr);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return;
  }
  if (!PEM_write_PrivateKey(pkey_file, pkey, nullptr, nullptr, 0, nullptr, nullptr))
  {
    std::cerr << "[CERTGEN] PEM_write_PrivateKey failed\n";
    ERR_print_errors_fp(stderr);
    fclose(pkey_file);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return;
  }
  fclose(pkey_file);

  FILE *cert_file = nullptr;
  if (fopen_s(&cert_file, certPath.c_str(), "wb") != 0 || !cert_file)
  {
    std::cerr << "[CERTGEN] Failed to open cert file for writing: " << certPath << "\n";
    ERR_print_errors_fp(stderr);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return;
  }
  if (!PEM_write_X509(cert_file, x509))
  {
    std::cerr << "[CERTGEN] PEM_write_X509 failed\n";
    ERR_print_errors_fp(stderr);
    fclose(cert_file);
    X509_free(x509);
    EVP_PKEY_free(pkey);
    return;
  }
  fclose(cert_file);

  X509_free(x509);
  EVP_PKEY_free(pkey);

  std::cout << "[CERTGEN] Certificate and key generated successfully.\n";
}
