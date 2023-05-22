 // FOR CREATING DIR
#pragma once

#ifdef _WIN32
#include<windows.h>
#endif // _WIN32

#ifdef __linux__
#include<unistd.h>
#include<sys/stat.h>
#include<errno.h>
#endif

int noName = 0;

std::string GetFolder(std::string in)
{
    if (in.find_first_of('/') == in.npos)
    {
        return std::to_string(noName++);
    }
    return in.substr(0, in.find_last_of('/')); // GET FILES FOLDER
}

std::string GetFileName(std::string in)
{
    in = in.substr(in.find_last_of('/')); // GET FILE NAME WITHOUT EXTENSION
    return in.substr(0, in.find_last_of('.'));
}

bool CreateDir(std::string basePath, std::string DirName)
{

#ifdef WIN32
		if (!CreateDirectoryA((basePath + DirName + '/').c_str(), NULL))
		{
			int error = GetLastError();
			if (error == 0xB7) // PATH EXISTS
			{
                return true;
			}
			else
			{
				printf("\x1B[31mError: %d\033[0m\n", error);
			}
			printf("\nhttps://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes");
			std::cin.get();
			return false;
		}
#endif // _WIN32

#ifdef __linux__
            struct stat st;
            std::string combined = basePath + DirName;
            if(stat(combined.c_str(),&st) == 0)
            {
                return 1;
            }

            //printf("%s%s", basePath.c_str(), DirName.c_str());
            if (-1 == mkdir((basePath + DirName + '/').c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
            {
                int error = ENOENT;
                if (error == 2)
                {
                    printf("\n\x1B[31mError %d: Path not found. Please Specify a Full path\033[0m\n", error);
                }
                else
                {
                    printf("\x1B[31mError: %d\033[0m\n", error);
                }
                std::cin.get();
                return -1;
            }
#endif


return 1;
}
