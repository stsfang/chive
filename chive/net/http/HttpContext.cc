#include "chive/net/http/HttpContext.h"
#include "chive/base/clog/chiveLog.h"
#include "chive/net/Buffer.h"
#include <algorithm>


using namespace chive;
using namespace chive::net;


HttpContext::HttpContext()
    : state_ (HttpParseState::kExpectRequestLine)
{
}

bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while (hasMore)
    {
        if (state_ == HttpParseState::kExpectRequestLine)
        {
            const char* crlf = buf->findCRLF(); // crlf的地址
            if (crlf)
            {
                ok = processRequestLine(buf->peek(), crlf);  // 提取请求行
                if (ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf+2);             // 移动buffer的readerIndex到请求行后
                    state_ = HttpParseState::kExpectHeader; // 转移到提取首部的状态
                }
                else 
                {
                    hasMore = false;    // 请求行提取失败，终止处理
                }
            }
            else {
                hasMore = false;    // 请求行提取失败，终止处理
            }
        }
        ///
        /// header field : value \r\n
        ///
        else if (state_ == HttpParseState::kExpectHeader)
        {
            const char* crlf = buf->findCRLF();         // 查找首部行的crlf结束符
            if(crlf)
            {
                const char* colon = std::find(buf->peek(), crlf, ':');  // 在 [begin(), crlf]之间找':'
                if (colon != crlf)
                {
                    request_.addHeader(buf->peek(), colon, crlf);   // 找到一个 ':' 则说明一个首部字段
                }
                else 
                {
                    ///FIXME:
                    /// no find colon, just empty line, end of header
                    state_ = HttpParseState::kExpectBody;
                    // hasMore = false;
                }
                buf->retrieveUntil(crlf+2);
            }
            else 
            {
                hasMore = false;
            }
        }
        else if (state_ == HttpParseState::kExpectBody)
        {
            std::string b = buf->retrieveAllAsString();
            if (!b.empty())
            {
                request_.setBody(b);
            }
            else 
            {
                CHIVE_LOG_WARN("no http body");
            }
            state_ = HttpParseState::kGotAll;
            hasMore = false;
        }
    }// end while
    return ok;
}

void HttpContext::reset()
{
    state_ = HttpParseState::kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
}

///
/// Http Method | space | URL | space | proto version | \r\n
///  
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');     // method
    if (space != end && request_.setMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');     // ur
        if (space != end)
        {
            const char* questionMark = std::find(start, space, '?');
            if (questionMark != space)
            {
                request_.setPath(start, questionMark);      // path
                request_.setQuery(questionMark, space);     // query string
            }
            else
            {
                request_.setPath(start, space);             // only path
            }

            start = space + 1;

            ///FIXME:
            ///增加对http2.0的支持
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed)
            {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::HttpVersion::kHttp20);
                } else {
                    succeed = false;
                }

            }
            
        }
        return succeed;
    }
}