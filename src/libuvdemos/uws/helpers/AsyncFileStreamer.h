#include <filesystem>
#include "../common.h"

struct AsyncFileStreamer {

    std::map<std::string_view, AsyncFileReader *> asyncFileReaders;
    std::string root;

    AsyncFileStreamer(std::string root) : root(root) {
        // for all files in this path, init the map of AsyncFileReaders
        updateRootCache();
    }

    void updateRootCache() {
        // todo: if the root folder changes, we want to reload the cache
        for(auto &p : std::filesystem::recursive_directory_iterator(root)) {
			std::string _p = p.path().string();
			string_replace_all(_p, "/", "\\");
            std::string url = _p.substr(root.length());
			//printf("url=%.*s\n", url.length(), url.data());
            //if (url == "/index.html") {
            //    url = "/";
            //}

            char *key = new char[url.length()];
            memcpy(key, url.data(), url.length());
            asyncFileReaders[std::string_view(key, url.length())] = new AsyncFileReader(_p);
        }
    }

    template <bool SSL>
    void streamFile(uWS::HttpResponse<SSL> *res, std::string_view url) {
		#define FMT_404 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL %.*s was not found on this server.</p></body></html>"
        auto it = asyncFileReaders.find(url);
        if (it == asyncFileReaders.end()) {
			char RES_404[SHRT_MAX] = { 0 };
            std::cout << "Did not find file: " << url << std::endl;
			snprintf(RES_404, sizeof(RES_404), FMT_404, url.length(), url.data());
			res->tryEnd(std::string_view(RES_404, strlen(RES_404)));
        } else {
            streamFile(res, it->second);
        }
    }

    template <bool SSL>
    static void streamFile(uWS::HttpResponse<SSL> *res, AsyncFileReader *asyncFileReader) {
        /* Peek from cache */
        std::string_view chunk = asyncFileReader->peek(res->getWriteOffset());
        if (!chunk.length() || res->tryEnd(chunk, asyncFileReader->getFileSize()).first) {
            /* Request new chunk */
            // todo: we need to abort this callback if peer closed!
            // this also means Loop::defer needs to support aborting (functions should embedd an atomic boolean abort or something)

            // Loop::defer(f) -> integer
            // Loop::abort(integer)

            // hmm? no?

            // us_socket_up_ref eftersom vi delar ägandeskapet

            if (chunk.length() < asyncFileReader->getFileSize()) {
                asyncFileReader->request(res->getWriteOffset(), [res, asyncFileReader](std::string_view chunk) {
                    // check if we were closed in the mean time
                    //if (us_socket_is_closed()) {
                        // free it here
                        //return;
                    //}

                    /* We were aborted for some reason */
                    if (!chunk.length()) {
                        // todo: make sure to check for is_closed internally after all callbacks!
                        res->close();
                    } else {
                        AsyncFileStreamer::streamFile(res, asyncFileReader);
                    }
                });
            }
        } else {
            /* We failed writing everything, so let's continue when we can */
            res->onWritable([res, asyncFileReader](int offset) {

                // här kan skiten avbrytas!

                AsyncFileStreamer::streamFile(res, asyncFileReader);
                // todo: I don't really know what this is supposed to mean?
                return false;
            })->onAborted([]() {
                std::cout << "ABORTED!" << std::endl;
            });
        }
    }
};
