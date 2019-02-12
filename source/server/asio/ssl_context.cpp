/*!
    \file ssl_context.cpp
    \brief SSL context implementation
    \author Ivan Shynkarenka
    \date 12.02.2019
    \copyright MIT License
*/

#include "server/asio/ssl_context.h"

#if defined(_WIN32) || defined(_WIN64)
#include <wincrypt.h>
#endif

namespace CppServer {
namespace Asio {

void SSLContext::set_root_certs()
{
#if defined(_WIN32) || defined(_WIN64)
    HCERTSTORE hStore = CertOpenSystemStore(0, "ROOT");
    if (hStore == nullptr)
        return;

    X509_STORE* store = X509_STORE_new();
    PCCERT_CONTEXT pContext = nullptr;
    while ((pContext = CertEnumCertificatesInStore(hStore, pContext)) != nullptr)
    {
        // Convert from DER to internal format
        X509* x509 = d2i_X509(nullptr, (const unsigned char**)&pContext->pbCertEncoded, pContext->cbCertEncoded);
        if (x509 != nullptr)
        {
            X509_STORE_add_cert(store, x509);
            X509_free(x509);
        }
    }

    CertFreeCertificateContext(pContext);
    CertCloseStore(hStore, 0);

    // Attach X509_STORE to the current SSL context
    SSL_CTX_set_cert_store(native_handle(), store);
#endif
}

} // namespace Asio
} // namespace CppServer
