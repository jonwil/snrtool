#pragma once
#include <stdio.h>
struct STDSTREAM
{
    FILE* file;
    int filepos;
    int filesize;
    int currentwritepos;
    char* pointer;
    char* pointer2;
    int iswritable;
    int needsflush;
    int filemode;
    char fname[1024];
    char fname2[1024];
    char buffer[8192];
};
STDSTREAM* gopen(const char* filename);
STDSTREAM* gwopen(char* filename);
int gclose(STDSTREAM* g);
int gread(STDSTREAM* g, void* buf, int size);
int gwrite(STDSTREAM* g, void* buf, int size);
int gseek(STDSTREAM* g, long long pos);
void getpath(const char* path, wchar_t* outpath, int pos);
int removefile(const char* fname);
int renamefile(const char* fname1, const char* fname2);
bool fileexists(const char* filename);
void buildfilename(char* s1, int size, const char* s2, const char* s3, const char* s4);