#include "chive/net/http/HttpServer.h"
#include "chive/net/http/HttpRequest.h"
#include "chive/net/http/HttpResponse.h"
#include "chive/net/EventLoop.h"
#include "chive/base/clog/chiveLog.h"

#include <iostream>
#include <map>

using namespace chive;
using namespace chive::net;

// extern 
bool benchmark = false;

void onRequest(const HttpRequest& req, HttpResponse* resp)
{
    CHIVE_LOG_DEBUG("method %s path %s query %s", req.methodToString(), req.path().c_str(), req.query().c_str());
    CHIVE_LOG_DEBUG("Headers %s  %s", req.methodToString(), req.path().c_str());
    if (!benchmark)
    {
        const std::map<std::string, std::string>& headers = req.getHeaders();
        for (const auto& header : headers)
        {
            CHIVE_LOG_DEBUG("%s: %s", header.first.c_str(), header.second.c_str());
        }

        CHIVE_LOG_DEBUG("Body content %s", req.getBody().c_str());
    }

    if (req.path() == "/")
    {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200OK);
        resp->setStatusMessage("OK");
        resp->addHeader("Server", "Chive");
        std::string now = "2020.10.01 16:18:00";
        resp->setBody("<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " + now +
        "</body></html>");
    }
    else if (req.path() == "/hello")
    {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k200OK);
        resp->setStatusMessage("OK");
        resp->setContentType("text/plain");
        resp->addHeader("Server", "Muduo");
        resp->setBody("hello, world!\n");
    }
    else
    {
        resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCloseConnection(true);
    }
}

int main()
{
    int numThreads = 0;
    startLogPrint(nullptr);
    EventLoop loop;
    HttpServer server(&loop, InetAddress("127.0.0.1", 8000), "dummy");
    server.setHttpCallback(onRequest);
    server.setThreadNum(numThreads);
    server.start();
    loop.loop();
}