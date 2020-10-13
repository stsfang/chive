/**
 * File.h
 * 文件抽象对象，包括文件操作的方法
 */

#ifndef CHIVE_BASE_FILE_H
#define CHIVE_BASE_FILE_H

#include "chive/base/copyable.h"
#include "chive/base/ContentTypes.h"
#include <sys/types.h>  // for off_t
#include <fstream>

namespace chive
{
namespace FileUtil
{

class File : copyable 
{
public:
    explicit File(std::string filename, ContentType type = ContentType::TextPlain);
    ~File();
    // 输出到指定路径
    // void output(std::string outputPath);
    // 将char[begin, end]输出到FILE_PATH/filename
    void output(const char* begin, const char* end);

    //void flush();

    //void append(const char* text, size_t len);

    off_t writtenBytes() const 
    {
        return writtenBytes_;
    }

private:
    // size_t write(const char* text, size_t len);

    std::string filename_;
    ContentType contentType_;
    MimeType mimeType_;
    // suffixType_; 扩展名
    // 
    std::fstream fout_;
    // char buffer_[64 * 1024];    // 64KB
    off_t writtenBytes_;        // 
};


} // namespace FileUtil



} // namespace chive


#endif
