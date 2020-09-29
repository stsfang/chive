#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fnctl.h>

#define BUFFER_SIZE 4096

enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER
};

enum LINE_STATUS {
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN,
};

enum HTTP_CODE {
    NO_REQUEST,
    GET_REQUEST,
    BAD_REQUEST,
    FORBIDDEN_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};

static const char* szret[] = {"i get a correct result\n", "something wrong\n"};

LINE_STATUS parse_line(char *buffer, int& cehcked_index, int& read_index)
{
    char temp;
    for(; checked_index < read_index; ++checked_index)
    {
        temp = buffer[checked_index];
        if(temp == '\r')
        {
            if((checked_index + 1) == read_index)
            {
                return LINE_OPEN;
            }
            else if(buffer[checked_index+1] == '\n')
            {
                //replace '\r' and '\n' with '\0'
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(temp == '\n')
        {
            if((checked_index > 1) && buffer[checked_index - 1] == '\r')
            {
                buffer[checked_index-1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    
    }
    // find no '\r', no line found
    return LINE_OPEN;
}

HTTP_CODE parse_requestline(char *temp, CHECK_STATE& checkstate)
{
    // begin to parse the request line

    //1. parse http method
    char* url = strpbrk(temp, " \t");
    if(!url)
    {
        printf("bad request.\n");
        return BAD_REQUEST;
    }
    // replace ' ' with '\0' 
    *url = '\0';
    url++;

    char* method = temp;
    if(strcasecmp(method, "GET") == 0)
    {
        printf("request method is GET.\n");
    }
    else
    {
        return BAD_REQUEST;
    }

    //remove ' ' at the begin
    url += strspn(url, " \t");
    
    //2) parse http version
    char* version = strpbrk(url, " \t");
    if(!version)
    {
        return BAD_REQUEST;
    }

    *version = '\0';
    version++;
    //remove ' ' at the begin
    version += strspn(version, " \t");
    if(strcasecmp(version, "HTTP/1.1") != 0)
    {
        return BAD_REQUEST;
    }

    // if url begins with "http://", we need to remove this part
    // and make url begin with '/'
    if(strncasecmp(url,"http://", 7) == 0)
    {
        url += 7;                   // skip "http://"
        utl = strchr(url, '/');     // reset to begin with '/'
    }
    //check the format of url
    if(!url || url[0] != '/')
    {
        return BAD_REQUEST;
    }

    printf("HTTP method is: %s\n", method);
    printf("The request URL is: %s\n", url);
    printf("HTTP version is: %s\n", version);
    
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTP_CODE parse_headers(char* temp)
{
    // meet with a blank line, request header is over
    if(temp[0] == '\0')
    {
        return GET_REQUEST;
    }
    else if(strncasecmp(temp, "Host:", 5) == 0)
    {
        temp += 5;
        temp += strspn(temp, " \t");
        printf("the request host is: %s\n", temp);
    }
    else
    {
        printf("cannot handle this header\n");
    }
    return NO_REQUEST;
}

HTTP_CODE parse_content(char* buffer, int& checked_index, CHECKED_STATE& checkstate, 
        int& read_index, int& start_line)
{
    LINE_STATUS linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;

    while( (linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK)
    {
        char* temp = buffer + start_line;
        start_line
    }
}
