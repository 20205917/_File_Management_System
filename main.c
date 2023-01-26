#include <stdio.h>
#include <assert.h>
#include "ramfs.h"


int main() {
    char * c =clean_path("///a//////////b///////c////");
    printf("%s\n",c);

    c =clean_path("///a//////////b///////c.txt/");
    assert(c==NULL);

   c =clean_path("a//////////b///////c////");
    printf("%s\n",c);
}
