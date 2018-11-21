/* ========================================================================== */
/*                                                                            */
/*   CreateDir.h                                                            */
/*   from: https://stackoverflow.com/a/29828907/3895476                       */
/*                                                                            */
/*   A simple way to create folders in both, windows and linux (mac?)         */
/*                                                                            */
/* ========================================================================== */

#ifndef CREATEDIR_H    //Safeguard
#define CREATEDIR_H

#include <iostream>
#include <string>
#include <sys/stat.h> // stat
#include <errno.h>    // errno, ENOENT, EEXIST
#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif

bool isDirExist(const std::string& path)
{
#if defined(_WIN32)
    struct _stat info;
    if (_stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & _S_IFDIR) != 0;
#else 
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
#endif
}

bool makePath(const std::string& path)
{
#if defined(_WIN32)
    int ret = _mkdir(path.c_str());
#else
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
#endif
    if (ret == 0)
        return true;

    switch (errno)
    {
    case ENOENT:
        // parent didn't exist, try to create it
        {
            int pos = path.find_last_of('/');
            if (pos == int(std::string::npos))
#if defined(_WIN32)
                pos = path.find_last_of('\\');
            if (pos == std::string::npos)
#endif
                return false;
            if (!makePath( path.substr(0, pos) ))
                return false;
        }
        // now, try to create again
#if defined(_WIN32)
        return 0 == _mkdir(path.c_str());
#else 
        return 0 == mkdir(path.c_str(), mode);
#endif

    case EEXIST:
        // done!
        return isDirExist(path);

    default:
        return false;
    }
}
#endif
/* for testing
int main(int argc, char* ARGV[])
{
    for (int i=1; i<argc; i++)
    {
        std::cout << "creating " << ARGV[i] << " ... " << (makePath(ARGV[i]) ? "OK" : "failed") << std::endl;
    }
    return 0;
}
*/
