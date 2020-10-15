#ifndef CHIVE_NET_CODEC_HTTP_POST_DECODER_H
#define CHIVE_NET_CODEC_HTTP_POST_DECODER_H

#include "chive/net/http/HttpRequest.h"

#include <string>
#include <vector>


    /**
     * states follow NOTSTARTED PREAMBLE ( (HEADERDELIMITER DISPOSITION (FIELD |
     * FILEUPLOAD))* (HEADERDELIMITER DISPOSITION MIXEDPREAMBLE (MIXEDDELIMITER
     * MIXEDDISPOSITION MIXEDFILEUPLOAD)+ MIXEDCLOSEDELIMITER)* CLOSEDELIMITER)+
     * EPILOGUE
     *
     * First getStatus is: NOSTARTED
     *
     * Content-type: multipart/form-data, boundary=AaB03x => PREAMBLE in Header 
     * --AaB03x => HEADERDELIMITER 
     * content-disposition: form-data; name="field1" => DISPOSITION
     * Joe Blow => FIELD 
     * --AaB03x => HEADERDELIMITER 
     * content-disposition:form-data; name="pics" => DISPOSITION 
     * Content-type: multipart/mixed,boundary=BbC04y
     * --BbC04y => MIXEDDELIMITER 
     * Content-disposition: attachment; filename="file1.txt" => MIXEDDISPOSITION 
     * Content-Type: text/plain
     * ... contents of file1.txt ... => MIXEDFILEUPLOAD --BbC04y =>
     * MIXEDDELIMITER Content-disposition: file; filename="file2.gif" =>
     * MIXEDDISPOSITION Content-type: image/gif Content-Transfer-Encoding:
     * binary
     *
     * ...contents of file2.gif... => MIXEDFILEUPLOAD --BbC04y-- =>
     * MIXEDCLOSEDELIMITER --AaB03x-- => CLOSEDELIMITER
     *
     * Once CLOSEDELIMITER is found, last getStatus is EPILOGUE
     */

class HttpPostRequestDecoder {
public:
    static bool isMultipart(const HttpRequest& request) {
        std::string mimeType = request.getHeader("content-type");
        if (mimeType != nullptr && mimeType.startsWith("multipart/form-data")) {
            return getMultipartDataBoundary(mimeType) != nullptr;
        }
        return false;
    }

    static std::vector<std::string> 
    getMultipartDataBoundary(const std::string& contentType) {
        // Check if Post using "multipart/form-data; boundary=--89421926422648 [; charset=xxx]"
        std::vector<std::string> headerContentType = splitHeaderContentType(contentType);
        std::string multipartHeader = "multipart/form-data";
        if(headerContentType[0].compare(multipartHeader) == 0) {
            ///FIXME:
            ///find out boundary value
        }
        return headerContentType;
    }

    static std::vector<std::string>
    splitHeaderContentType(const std::string& contentType) {
        //"multipart/form-data; boundary=--89421926422648 [; charset=xxx]"
        // like: "multipart/form-data; boundary=--89421926422648; charset=utf8"
        int aStart, aEnd;
        int bStart, bEnd;
        int cStart, cEnd;

        aStart = findNonWhiteSpace(contentType, 0);
        aEnd = contentType.find_first_of(';');
        /// static const size_type npos = -1
        if(aEnd == -1) {
            return { contentType, "", ""};
        }
        bStart = findNonWhiteSpace(contentType, aEnd + 1);
        if (contentType[aEnd - 1] == ' ') {
            aEnd--;
        }
        bEnd = contentType.find_first_of(';', bStart);
        if (bEnd == -1) {
            bEnd = findEndOfString(contentType);
            return { 
                contentType.substr(aStart, aEnd-aStart), 
                contentType.substr(bStart, bEnd-bStart),
                ""
            };
        }
        cStart = findNonWhiteSpace(contentType, bEnd+1);
        if (contentType[bEnd-1] ==' ') {
            bEnd--;
        }
        cEnd = findEndOfString(contentType);
        return { 
            contentType.substr(aStart, aEnd-aStart), 
            contentType.substr(bStart, bEnd-bStart),
            contentType.substr(cStart, cEnd-cStart)
        };
    }

    static int findNonWhiteSpace(const std::string& s, int start) {
        int result = -1;
        for (result = start; result < s.length(); ++result) {
            if (s[result] != ' ') {
                break;
            }
        }
        return result;
    }

    static int findEndOfString(const std::string& s) {
        int result;
        for (result = s.length(); result > 0; --result) {
            if (s[result - 1] != ' ') {
                break;
            }
        }
        return result;
    }
    
};

#endif