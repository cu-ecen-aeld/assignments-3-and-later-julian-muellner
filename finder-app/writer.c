

/* OLD SCRIPT:
#!/usr/bin/env bash

if [[ $# -ne 2 ]]; then
    echo "Exactly two arguments expected"
    exit 1
fi

writefile=$1
writestr=$2

mkdir -p $(dirname $writefile) && touch ${writefile} && echo ${writestr} > ${writefile}

if [[ $? -eq 1 ]]; then
    echo "File could not be created"
    exit 1
fi
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int main(int argc, char** argv) {
    openlog(NULL, 0, LOG_USER);

    if(argc != 3) {
        syslog(LOG_ERR, "Invalid number of arguments, expected 2 but received %d", argc - 1);
        exit(EXIT_FAILURE);
    }

    const char* writefile = argv[1];
    const char* writestr = argv[2];

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

    // can assume path existence
    FILE* f = fopen(writefile, "w");
    
    if(f == NULL) {
        syslog(LOG_ERR, "Failed to open file %s: %s", writefile, strerror(errno));
        exit(EXIT_FAILURE);    
    }

    size_t bytes = strlen(writestr) + 1;
    size_t result = fwrite(writestr, sizeof(char), bytes, f);
    
    if(result != bytes) {
        syslog(LOG_ERR, "Failed to write file %s", writefile);
        exit(EXIT_FAILURE);    
    }

    exit(EXIT_SUCCESS);
}
