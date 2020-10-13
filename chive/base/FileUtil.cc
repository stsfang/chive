#include "chive/base/FileUtil.h"
#include "chive/base/clog/chiveLog.h"
#include <iostream>

using namespace chive;
using namespace chive::FileUtil;

const char* FILE_PATH = "/data/chive/upload/";

File::File(std::string filename, ContentType type)
  : filename_(filename),
    contentType_(type),
    writtenBytes_(0)
{

}

File::~File()
{

}

void File::output(const char* begin, const char* end)
{
    fout_.open(FILE_PATH + filename_, std::ios::out|std::ios::binary);
    if (!fout_.is_open())
    {
        // CHIVE_LOG_ERROR("Cannot open file %s", FILE_PATH + filename_);
        std::cout << "err" << std::endl;
    }
    else 
    {
        fout_ << std::string(begin, end);
        fout_.close();
    }
}
