#include "chive/net/http/HttpContext.h"
#include "chive/net/Buffer.h"

using namespace chive::net;

HttpContext::HttpContext()
    : state_ (HttpParseState::kExpectRequestLine)
{
}

bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{

}

void HttpContext::reset()
{

}

bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    
}