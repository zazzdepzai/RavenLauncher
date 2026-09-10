#pragma once
#include <string>
#include <functional>

namespace mclauncher {

// Progress callback: (bytesReceived, bytesTotal)
using ProgressCallback = std::function<void(size_t, size_t)>;

// Result of a single file fetch.
struct DownloadResult {
    bool success = false;
    std::string localPath;
    std::string error;
};

class DownloadManager {
public:
    // Downloads `url` to `destPath`, verifies against `expectedSha256`
    // (skip check by passing an empty string). Reports progress via callback.
    // Backed by libcurl on the iOS side (see ios/NetworkBridge.mm).
    static DownloadResult DownloadFile(const std::string& url,
                                        const std::string& destPath,
                                        const std::string& expectedSha256,
                                        const ProgressCallback& onProgress = nullptr);

    // Fetches a small text/JSON resource (manifests) synchronously.
    static bool FetchText(const std::string& url, std::string& outText);

    static std::string Sha256File(const std::string& path);
};

} // namespace mclauncher
