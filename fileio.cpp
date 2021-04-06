#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys\stat.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "fileio.h"

void strappend(char* s1, int size, const char* s2, const char* s3)
{
    strcpy_s(s1, size, s2);
    strcat_s(s1, size, s3);
}

void getpath(const char* path, wchar_t* outpath, int pos)
{
    wchar_t fname[1024];
    mbstowcs_s(nullptr, fname, path, 1024);
    if (*path == '\\' && path[1] == '\\')
    {
        wcsncpy_s(outpath, 8, L"\\\\?\\UNC", 7);
        wcsncpy_s(outpath + 7, 1016, &fname[1], 1016);
    }
    else
    {
        wcsncpy_s(outpath, 5, L"\\\\?\\", 4);
        GetFullPathName(fname, pos - 8, outpath + 4, 0);
    }
}

void print(const char* format, ...)
{
}

bool fileexists(const char* filename)
{
    if (!filename)
    {
        return 0;
    }
    wchar_t buf[1024];
    getpath(filename, buf, 1024);
    int ret = _waccess(buf, 0);
    if (ret == -1)
    {
        ret = *_errno();
    }
    return ret != ENOENT;
}

STDSTREAM* gopen(const char* filename)
{
    STDSTREAM* s = nullptr;
    FILE* f;
    int mode;
    wchar_t buf[1024];
    getpath(filename, buf, 1024);
    _wfopen_s(&f, buf, L"rb");
    mode = 1;
    if (f)
    {
        if (fseek(f, 0, SEEK_END))
        {
            print("gopen - SEEK ON OPEN FAILED (%s)\n", filename);
        }
        else
        {
            s = new STDSTREAM;
            s->pointer = s->buffer;
            s->pointer2 = s->buffer;
            s->filesize = ftell(f);
            fseek(f, 0, SEEK_SET);
            s->filemode = mode;
            s->file = f;
            s->filepos = 0;
            s->iswritable = 0;
            s->needsflush = 0;
            s->currentwritepos = 0;
            strcpy_s(s->fname, filename);
        }
    }
    return s;
}

int removefile(const char* fname)
{
    wchar_t buf[1024];
    getpath(fname, buf, 1024);
    return _wremove(buf) == 0;
}

int renamefile(const char* fname1, const char* fname2)
{
    wchar_t buf1[1024];
    wchar_t buf2[1024];
    getpath(fname1, buf1, 1024);
    getpath(fname2, buf2, 1024);
    int ret2 = _wrename(buf1, buf2);
    int ret = ret2 == 0;
    if (!ret2 || errno != EEXIST || _wremove(buf2))
    {
        return ret;
    }
    else
    {
        return _wrename(buf1, buf2) == 0;
    }
}

void buildfilename(char* s1, int size, const char* s2, const char* s3, const char* s4)
{
    strcpy_s(s1, size, s2);
    if (s3)
    {
        strcat_s(s1, size, s3);
    }
    strcat_s(s1, size, ".");
    strcat_s(s1, size, s4);
}

STDSTREAM* gwopen(char* filename)
{
    int filemode = 3;
    char fname[1024];
    fname[0] = 0;
    struct _stat64i32 stat;
    if (_stat64i32(filename, &stat) != -1 && (stat.st_mode & S_IREAD) != 0 && (stat.st_mode & S_IWRITE) == 0)
    {
        print("gwopen - ATTEMPT TO OPEN READONLY FILE FOR WRITING (%s)\n", filename);
        return 0;
    }
    if (_stat64i32(filename, &stat) != -1 && (stat.st_mode & S_IFDIR) != 0)
    {
        print("gwopen - ATTEMPT TO OPEN DIRECTORY FOR WRITING (%s)\n", filename);
        return 0;
    }
    strappend(fname, sizeof(fname), filename, ".tm1");
    FILE* f;
    wchar_t buf[1024];
    getpath(fname, buf, 1024);
    _wfopen_s(&f, buf, L"w+b");
    if (!f)
    {
        _wfopen_s(&f, buf, L"wb");
        print("gwopen - UNABLE TO OPEN FILE FOR WRITE (%s)\n", fname);
        if (!f)
        {
            return nullptr;
        }
    }
    STDSTREAM* s = new STDSTREAM;
    s->filemode = 3;
    s->pointer = s->buffer;
    s->pointer2 = s->buffer;
    s->file = f;
    s->filepos = 0;
    s->filesize = 0;
    s->iswritable = 1;
    s->needsflush = 0;
    s->currentwritepos = 0;
    strcpy_s(s->fname, filename);
    strcpy_s(s->fname2, fname);
    return s;
}

long long WriteFlush(STDSTREAM* g)
{
    long long ret = 0;
    if (g->needsflush && g->iswritable)
    {
        fseek(g->file, g->currentwritepos, SEEK_SET);
        int ptr = g->pointer2 - (char*)g;
        if (ptr - 2084 > 0)
        {
            ret = fwrite(g->buffer, 1, ptr - 2084, g->file);
        }
        long long r = g->pointer2 - (char*)g - 0x824;
        if (r >= 0 && ret < (unsigned int)r)
        {
            print("WriteFlush - OUT OF DISK SPACE\n");
        }
        g->needsflush = 0;
    }
    g->pointer2 = g->buffer;
    g->pointer = g->buffer;
    fflush(g->file);
    return ret;
}

int gclose(STDSTREAM* g)
{
    int ret = 1;
    if (g)
    {
        WriteFlush(g);
        if (g->filemode)
        {
            int ret2 = fclose(g->file);
            ret = ret2 == 0;
            if (!ret2 && (g->filemode == 2 || g->filemode == 3))
            {
                char fname1[1024];
                char fname2[1024];
                strappend(fname1, sizeof(fname1), g->fname, ".tm2");
                removefile(fname1);
                renamefile(g->fname, fname1);
                ret = renamefile(g->fname2, g->fname);
                if (ret)
                {
                    if (g->filemode == 2)
                    {
                        strappend(fname2, sizeof(fname2), g->fname, ".bak");
                        removefile(fname2);
                        renamefile(fname1, fname2);
                    }
                    else
                    {
                        removefile(fname1);
                    }
                }
            }
        }
        delete g;
    }
    return ret;
}

int gread(STDSTREAM* g, void* buf, int size)
{
    int i1 = 0;
    int i2 = 0;
    if (!g)
    {
        return 0;
    }
    int i3 = size;
    if (size < 0)
    {
        i3 = 0;
    }
    char* p1 = g->pointer;
    int i4 = g->pointer2 - p1;
    if (i3 < i4)
    {
        i4 = i3;
    }
    char* p2 = (char*)buf;
    if (i4 > 0)
    {
        memcpy(buf, p1, i4);
        g->pointer += i4;
        g->filepos += i4;
        p2 = (char*)buf + i4;
        i3 -= i4;
        i1 = i4;
    }
    int filepos = g->filepos;
    if (i3 >= g->filesize - filepos)
    {
        i3 = g->filesize - filepos;
    }
    if (i3 >= 0)
    {
        if (g->pointer2 == g->pointer && i3)
        {
            int i5 = (unsigned int)(filepos + i3) >> 13 << 13;
            WriteFlush(g);
            int i6 = g->filepos;
            if (i6 < i5)
            {
                int i7 = fread(p2, 1, i5 - i6, g->file);
                p2 += i7;
                i2 = i7;
                i3 -= i7;
            }
            unsigned int i8 = g->filesize - i5;
            g->currentwritepos = i5;
            int i9 = 8192;
            if (i8 <= 8192)
            {
                i9 = i8;
            }
            fread(g->buffer, 1, i9, g->file);
            memcpy(p2, g->pointer, i3);
            g->pointer += i3;
            g->pointer2 += i9;
            g->filepos += i3 + i2;
            i2 += i3;
        }
        return i1 + i2;
    }
    else
    {
        print("gread - ATTEMPT TO READ PAST END OF FILE\n");
        return i1;
    }
}

int gwrite(STDSTREAM* g, void* buf, int size)
{
    int i1 = 0;
    if (!g)
    {
        return 0;
    }
    if (!g->iswritable)
    {
        print("gwrite - ATTEMPT TO WRITE TO A READ ONLY FILE\n");
        return 0;
    }
    char* p1 = g->pointer;
    int i2 = size;
    int i3 = (char*)g - p1 + 10276;
    if (size < i3)
    {
        i3 = size;
    }
    if (i3 > 0)
    {
        memcpy(p1, buf, i3);
        g->pointer += i3;
        p1 = g->pointer;
        g->filepos += i3;
        int i4 = g->filepos;
        if (g->pointer2 < p1)
        {
            g->pointer2 = p1;
        }
        if (i4 > g->filesize)
        {
            g->filesize = i4;
        }
        buf = (char*)buf + i3;
        g->needsflush = 1;
        i2 = size - i3;
        i1 = i3;
    }
    if (g->pointer2 == p1 && i2)
    {
        int i5 = (unsigned int)(i2 + g->filepos) >> 13 << 13;
        WriteFlush(g);
        int i6 = g->filesize;
        if (i6 > i5)
        {
            int i7 = i6 - i5;
            if (i7 > 8192)
            {
                i7 = 8192;
            }
            fseek(g->file, i5, 0);
            fread(g->buffer, 1, i7, g->file);
            g->pointer2 += i7;
        }
        fseek(g->file, g->filepos, 0);
        int i8 = 0;
        if (i5 - g->filepos > 0)
        {
            i8 = fwrite(buf, 1, i5 - g->filepos, g->file);
        }
        int i9 = g->filepos;
        if (i8 < i5 - i9)
        {
            print("gwrite - OUT OF DISK SPACE\n");
            return i8 + i1;
        }
        i1 += i2;
        g->filepos = i8 + i9;
        int i10 = i2 - i8;
        memcpy(g->buffer, (char*)buf + i8, i10);
        g->filepos += i10;
        int i11 = g->filepos;
        g->pointer += i10;
        char* p2 = g->pointer;
        g->currentwritepos = i5;
        if (i11 > g->filesize)
        {
            g->filesize = i11;
        }
        if (p2 > g->pointer2)
        {
            g->pointer2 = p2;
        }
        g->needsflush = 1;
    }
    return i1;
}

int gseek(STDSTREAM* g, long long pos)
{
    int i1 = 1;
    int result;
    if (!g)
    {
        return 0;
    }
    long long i2 = pos;
    if (i2 < 0)
    {
        i2 = 0;
        i1 = 0;
    }
    else
    {
        if (pos > g->filesize && !g->iswritable)
        {
            i2 = g->filesize;
            i1 = 0;
        }
    }
    long long i4 = i2 - g->filepos;
    unsigned int i5 = (unsigned int)(i4 + g->filepos);
    g->filepos = i5;
    if (i4 < (char*)g - g->pointer + 2084 || i4 > g->pointer2 - g->pointer)
    {
        int i6 = i5 >> 13 << 13;
        WriteFlush(g);
        int i1a;
        bool b;
        if (fseek(g->file, i6, SEEK_SET) || (b = 1, i1a = 1, b))
        {
            i1a = 0;
        }
        int size = g->filesize;
        g->currentwritepos = i6;
        if (size <= i6)
        {
            int i7 = g->filepos - i6;
            memset(g->buffer, 0, i7);
            result = i1a;
            g->pointer2 = &g->buffer[i7];
            g->pointer = &g->buffer[i7];
        }
        else
        {
            unsigned int i8 = size - i6;
            int i9 = 8192;
            if (i8 <= 8192)
            {
                i9 = i8;
            }
            fread(g->buffer, 1, i9, g->file);
            g->pointer2 += i9;
            g->pointer += g->filepos - i6;
            result = i1a;
        }
    }
    else
    {
        result = i1;
        g->pointer = &g->pointer[i4];
    }
    return result;
}
