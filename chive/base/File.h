/**
 * File.h
 * 文件抽象对象，包括文件操作的方法
 */

#ifndef CHIVE_BASE_FILE_H
#define CHIVE_BASE_FILE_H

#include "chive/base/copyable.h"
#include "chive/base/ContentTypes.h"

namespace chive
{

class File : copyable 
{
public:
    File(const std::string& content, ContentType type);
    // 输出到指定路径
    void output(std::string outputPath);

private:
    std::string filename_;
    ContentType contentType_;
    MimeType mimeType_;
    // suffixType_; 扩展名
    // 
};

} // namespace chive


#endif
