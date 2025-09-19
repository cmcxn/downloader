#pragma once

#include <string>

#include <curl/curl.h>
#include <sstream>

inline static size_t data_cb(void *contents, size_t size, size_t nmemb, std::string* userp)
{
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);

    return total_size;
}



inline int http_post(const std::string& url, const std::string& post_data, std::string& response)
{
    
    CURL* curl;
    CURLcode res = CURLE_FAILED_INIT;

    struct curl_slist* slist_header = NULL;
    slist_header = curl_slist_append(slist_header, "Content-Type: application/json");

    curl_global_init(CURL_GLOBAL_ALL);


    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist_header);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_slist_free_all(slist_header);
    
    curl_global_cleanup();

    return res;
}

inline int http_get(const std::string& url, std::string& response)
{

    CURL* curl;
    CURLcode res = CURLE_FAILED_INIT;
    

    curl_global_init(CURL_GLOBAL_ALL);


    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

 
    curl_global_cleanup();

    return res;
}

inline int http_head(const std::string& url, std::string& response_headers) {
    CURL* curl;
    CURLcode res = CURLE_FAILED_INIT;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 只请求头部
        curl_easy_setopt(curl, CURLOPT_HEADER, 1L); // 获取 Header
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_headers);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "http_head failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return res;
}
