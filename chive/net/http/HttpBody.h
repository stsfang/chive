#ifndef CHIVE_NET_HTTPBODY_H
#define CHIVE_NET_HTTPBODY_H

#include "chive/base/noncopyable.h"
#include "chive/base/ContentTypes.h"
#include "chive/base/FileUtil.h"

///FIXME: how to parse multipart/form-data
/// Method POST, PUT, DELETE support body filed (which body content-type should be supported in priority)
/// Method GET doesn't support body field

/*

content-type: multipart/form-data; boundary=----WebKitFormBoundaryzAq7xCCRBiuAI8E8

Body content ------WebKitFormBoundary9wcq6lyUzDdPCBZc
Content-Disposition: form-data; name="file"; filename="file_test.txt"
Content-Type: text/plain

this a test file to chive server.
------WebKitFormBoundary9wcq6lyUzDdPCBZc
Content-Disposition: form-data; name="name"

stsfang
------WebKitFormBoundary9wcq6lyUzDdPCBZc
Content-Disposition: form-data; name="name2"; filename="blob"
Content-Type: application/json

hello
------WebKitFormBoundary9wcq6lyUzDdPCBZc--

*/

#include <string>
#include <vector>
#include <utility>  //std::pair
namespace chive
{
namespace net
{

class HttpBody : noncopyable 
{
public:

    HttpBody(const std::string& body, const std::string& boundary);
    
    /**
     * 解析Http body 的内容, 必须在调用其他成员方法前调用，且应该仅调用一次
     */
    void parse();
    //ContentType type();
    //isMultipart();
    

private:
    enum class ContentDisposition 
    {
        FormData
    };
    // 表示表单数据的Item
    struct Segment 
    {
        ContentDisposition disp_;
        std::string name_;      // name属性
        std::string filename_;  // filename, 非文件的value，默认设置为blob？
        ContentType type_;      // 单个key-value的contentType，一般不用，暂把key-value当做字符串
        std::string key_;       
        std::string value_;
    };
    using DataPair = std::pair<std::string, std::string>;
    using FormData  = std::vector<Segment>;
   

    ContentType contentType_;           /// body content type
    uint64_t    contentLength_;         /// body content length
    //== form data
    //std::vector<FileUtil::File> fileList_;        /// file list from multipart/formdata
    FormData formData_;                 /// form data 
    //== form data

    const std::string& body_;
    const std::string boundary_;       /// body 参数分隔边界

};
} // namespace net

} // namespace chive


#endif