#include "chive/net/Buffer.h"
#include "chive/base/clog/chiveLog.h"
#include <cassert>
#include <string>
#include <sys/uio.h>
#include <algorithm>

using namespace chive;
using namespace chive::net;

const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialSize)
    : buffer_ (kCheapPrepend + initialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
}


ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    /// stack上创建临时缓冲区 extrabuf 使得总的缓冲区足够大
    /// 足以一次性把数据读到buf上, chive 采用level trigger
    /// 然后再判断需不需要对buffer_ 扩容
    /// 好处是减少buffer_的预分配内存
    char extrabuf[65536];
    /// ref: https://linux.die.net/man/2/readv
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    ssize_t n = readv(fd, vec, 2);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable ) {
        /// 可用空间足够，移动 writerIndex_ 游标
        writerIndex_ += n;      
    } else {
        /// 可用空间不足，append操作会扩容
        /// FIXME: (1)和(2) 应该是原子的，否则，如果(1)执行完毕而(2)扩容失败
        /// 导致结果不正确
        writerIndex_ = buffer_.size();  // (1)
        append(extrabuf, n - writable); // (2)
        
        /// 如下if成立，则可能还有数据没读出来 (数据量 > 64KiB)
        if (n == static_cast<ssize_t>(writable + sizeof(extrabuf))) {
            n += readFd(fd, savedErrno);
        }
    }
    return n;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    encWritten(len);    //增加已写入的计数
}

// for httpResponse
void Buffer::append(const std::string& msg)
{
    append(msg.data(), msg.size());
}
// 保证有足够的可写空间
void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len) 
    {
        makeSpace(len); //可用空间不足，扩容
    }
    assert(writableBytes() >= len);
}


void Buffer::encWritten(size_t len)
{
    assert(len <= writableBytes());
    writerIndex_ += len;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len) 
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len <= readableBytes())
    {
        readerIndex_ += len;
    }
    else 
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

void Buffer::makeSpace(size_t len)
{
    /*前置空闲空间 + 后部可写空间的大小不足以写入len字节的数据*/
    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
        buffer_.resize(writerIndex_ + len);
    }
    /*本地移动可读的数据到begin() + kCheapPrepend开端*/
    else 
    {
        assert(kCheapPrepend < readerIndex_);
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == readableBytes());
    }
}

const char* Buffer::findCRLF() const
{
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
}