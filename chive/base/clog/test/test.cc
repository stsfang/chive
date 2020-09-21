#include "../chiveLog.h"

int main() {
    startLogPrint(NULL);

    while(1) {
        CHIVE_LOG_ERROR("%s", "helloworld");
        CHIVE_LOG_INFO("%s", "helloworld");
        CHIVE_LOG_VERB("%s", "helloworld");
        CHIVE_LOG_WARN("%s", "helloworld");
        CHIVE_LOG_DEBUG("%s", "helloworld");
    }
}