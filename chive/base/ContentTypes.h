#ifndef CHIVE_NET_CONTENT_TYPES_H
#define CHIVE_NET_CONTENT_TYPES_H

namespace chive
{

/**
 * 主流的body content type
 * 
 * application/x-www-form-urlencoded： 不属于http content-type规范，通常用于浏览器表单提交，
 * 格式:name1=value1&name2=value2, POST会放入http body，GET则显示在在URL
 * urlencoded格式如 URL中出现 %E4%BD%A0，与unicode(\uxxxx) 区分
 * 
 * 
 */
enum class ContentType
{
    ApplicationJson,
    ApplicationXml,
    ApplicationBase64,
    ApplicationXW3FormUrlEncoded,   /**/
    ApplicationOctetStream, /*二进制流或字节数组*/
    MultipartFormdata,  /*表单*/
    TextPlain,
    TextCss,
    TextHtml,
    ApplicationJavascript,
    OtherType
};

enum class MimeType
{
    TextXml,
    TextHtml,
};

} // namespace chive

#endif