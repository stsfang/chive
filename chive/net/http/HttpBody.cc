#include "chive/net/http/HttpBody.h"
#include "chive/base/clog/chiveLog.h"
#include <fstream>

using namespace chive;
using namespace chive::net;

HttpBody::HttpBody(const std::string& body, const std::string& boundary)
    : body_(body),
      boundary_(boundary)
{
}

void HttpBody::parse()
{
    CHIVE_LOG_DEBUG("body all %s", body_.c_str());
    std::string spliter = "--"+boundary_+"\r\n";
    auto pos = body_.find_first_of(spliter);
    pos += spliter.size();

    CHIVE_LOG_DEBUG("begin parse -- %s",body_.substr(pos).c_str());
    // content-disposition

    //filename
    auto filenamePos = body_.find("filename=", pos);
    filenamePos += std::string("filename=").size();
    // 去掉双引号
    std::string fileSegment = body_.substr(filenamePos);   // 注意有 \r\n
    CHIVE_LOG_DEBUG("body substr %d : %s", filenamePos, fileSegment.c_str());
    std::string exchanger = "\r\n";
    auto lineChangerPos = fileSegment.find_first_of(exchanger);
    std::string finalFilename = fileSegment.substr(0, lineChangerPos);
    std::string finalFilename2(&*(finalFilename.begin()+1), &*(finalFilename.end()-1));
    CHIVE_LOG_DEBUG("body substr2 %d : %s, boundary %s", 
            lineChangerPos, finalFilename2.c_str(), boundary_.c_str());

    auto fileBegin = fileSegment.find("\r\n\r\n");
    auto fileEnd = fileSegment.find_last_of("\r\n\r\n") - spliter.size() - 2;
    std::string fileContent = fileSegment.substr(fileBegin+4, fileEnd - fileBegin-4);
    CHIVE_LOG_DEBUG("filebegin %d fileend %d file content %s", fileBegin, fileEnd, fileContent.c_str());

    // 将filecontent写入/data/chive/upload/finalFilename2
    std::fstream fout;
    fout.open("./"+finalFilename2, std::ios::out|std::ios::binary);
    if (!fout.is_open())
    {
        CHIVE_LOG_DEBUG("open file err");
    }
    else 
    {
        fout << fileContent;
        fout.flush();
        fout.close();
    }
}
