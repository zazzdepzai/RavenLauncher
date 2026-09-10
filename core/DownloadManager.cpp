#include "DownloadManager.hpp"
#include <curl/curl.h>
#include <openssl/sha.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace mclauncher {

namespace {

struct ProgressCtx {
    const ProgressCallback* cb;
};

size_t WriteToFile(void* ptr, size_t size, size_t nmemb, void* userp) {
    auto* out = static_cast<std::ofstream*>(userp);
    out->write(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

size_t WriteToString(void* ptr, size_t size, size_t nmemb, void* userp) {
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

int ProgressFn(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t) {
    auto* ctx = static_cast<ProgressCtx*>(clientp);
    if (ctx && ctx->cb && *ctx->cb) {
        (*ctx->cb)(static_cast<size_t>(dlnow), static_cast<size_t>(dltotal));
    }
    return 0;
}

} // namespace

std::string DownloadManager::Sha256File(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "";

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    char buf[8192];
    while (f.read(buf, sizeof(buf)) || f.gcount()) {
        SHA256_Update(&ctx, buf, static_cast<size_t>(f.gcount()));
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);

    std::ostringstream oss;
    for (unsigned char b : hash) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

bool DownloadManager::FetchText(const std::string& url, std::string& outText) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    outText.clear();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outText);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return res == CURLE_OK;
}

DownloadResult DownloadManager::DownloadFile(const std::string& url,
                                              const std::string& destPath,
                                              const std::string& expectedSha256,
                                              const ProgressCallback& onProgress) {
    DownloadResult result;
    std::ofstream out(destPath, std::ios::binary);
    if (!out) {
        result.error = "cannot open dest path for writing: " + destPath;
        return result;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        result.error = "curl init failed";
        return result;
    }

    ProgressCtx ctx{&onProgress};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ProgressFn);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    out.close();

    if (res != CURLE_OK) {
        result.error = std::string("download failed: ") + curl_easy_strerror(res);
        return result;
    }

    if (!expectedSha256.empty()) {
        std::string actual = Sha256File(destPath);
        if (actual != expectedSha256) {
            result.error = "checksum mismatch (expected " + expectedSha256 + ", got " + actual + ")";
            std::remove(destPath.c_str());
            return result;
        }
    }

    result.success = true;
    result.localPath = destPath;
    return result;
}

} // namespace mclauncher
