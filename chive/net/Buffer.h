#ifndef CHIVE_NET_BUFFER_H
#define CHIVE_NET_BUFFER_H

#include "chive/base/copyable.h"
#include <vector>
#include <string>
#include <cassert>

namespace chive
{
namespace net
{

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
///
/// 使用可移动的readerIndex，充分利用prependable bytes的动态空间
/// 可以兼顾内存使用量和使用效率
/// @endcode

class Buffer : copyable
{
public:
    /**
     * 预留8字节的空间
     * 已读的bytes也会算入prepend space的一部分
     * prepend space 是一个动态的区域
     */
    static const size_t kCheapPrepend = 8;         
    static const size_t kInitialSize = 1024;
    explicit Buffer(size_t initialSize = kInitialSize);
    ssize_t readFd(int fd, int* savedErrno);

    // 当前剩余容量还可写的字节数
    size_t writableBytes() const 
    { return buffer_.size() - writerIndex_; }

    // 可读的字节数
    size_t readableBytes() const 
    { return writerIndex_ - readerIndex_; }

    // 当前的prependable space大小
    size_t prependableBytes() const 
    { return readerIndex_; }

    const char* peek() const
    { return begin() + readerIndex_; }

    // for findCRLF()
    const char* beginWrite() const
    { return begin() + writerIndex_; }

    char* beginWrite() 
    { return begin() + writerIndex_; }

    // 保证足够的可写空间 >= len
    void ensureWritableBytes(size_t len);

    // 将data append 到writerIndex_
    void append(const char* data, size_t len);

    void append(const std::string& msg);
    
    // 增加已写的计数
    void encWritten(size_t len);

    // 可写空间的大小
    int writable();

    // 读取全部可读的字节作为string返回
    std::string retrieveAllAsString();

    // 读取可读的len字节作为string返回
    std::string retrieveAsString(size_t len);

    // 移动readIndex_ 和 writeIndex_ 游标
    void retrieve(size_t len);
    void retrieveAll();

    const char* findCRLF() const;
    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    std::string toString() const 
    {
        return std::string(peek(), static_cast<int>(readableBytes()));
    }

private:
    char* begin() 
    { return &*buffer_.begin(); }

    const char* begin() const 
    { return &*buffer_.begin(); }

    void makeSpace(size_t len);

    std::vector<char> buffer_;      // 可扩容的buf
    size_t readerIndex_;            // 读取的游标
    size_t writerIndex_;            // 写入的游标

    static const char kCRLF[];
};
} // namespace net

} // namespace chive

#endif