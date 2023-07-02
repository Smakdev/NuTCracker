#pragma once

std::string getFileName(std::string cur)
{
    cur = cur.substr(cur.find_last_of('/') + 1);
    return cur.substr(0, cur.find_last_of('.'));
}

std::string getFileFolder(std::string cur)
{
    size_t count = 0;

    if (cur.find_first_of('/') == cur.npos)
    {
        return cur;
    }

    for (size_t i = cur.size(); i --> 0;)
    {
        if (cur[i] == '/')
        {
            count++;
        }
        if (count == 2)
        {
            count = i;
            break;
        }
    }
    cur = cur.substr(count + 1, cur.find_last_of('/'));
    return cur.substr(0, cur.find_last_of('/'));
}

int readNu32(std::istream& stream)
{
    char memblock[4] = { 0 };
    stream.read(memblock, 4);
    return (((uint8_t)memblock[0]) << 24) | (((uint8_t)memblock[1]) << 16) | (((uint8_t)memblock[2]) << 8) | (((uint8_t)memblock[3]) << 0);
};

std::string readNXGFile(std::istream& stream)
{
    unsigned int stringSize = 0;
    char path[256] = { 0 };
    stream.ignore(16);
    stringSize = readNu32(stream);
    stream.read(path, stringSize);
    stream.ignore(4);
    return std::string(path);
}

std::string readDX11FileG1(std::istream& stream)
{
    char stringSize = 0;
    char path[256] = { 0 };
    stream.ignore(20);
    stream.read(&stringSize, 1);
    stream.read(path, (unsigned char) stringSize);
    stream.ignore(1);
    return std::string(path);
}


bool checkthesum(char* checksum)
{
    int count = 0;
    for (int i = 0; i < 16; i++)
    {
        if (checksum[i] == 0)
        {
            count++;
        }
    }
    if (count == 16)
        return false;
    
    return true;
};

std::string readDX11FileG2(std::istream& stream)
{
    char checksum[16] = { 0 };
    uint8_t stringSize = 0;
    char path[256] = { 0 };

    stream.read(checksum, 16);
    if (checkthesum(checksum))
    {
        stream.ignore(4);
        stream.read((char*)& stringSize, 1);
        stream.read(path, stringSize);
        stream.ignore(1);
    }
    else
    {
        stream.ignore(1);
        stream.read((char*)& stringSize, 1);
        stream.ignore(stringSize);
        stream.ignore(4);
    }
    stream.ignore(8);
    return std::string(path);
}

struct TSXT
{
    char FourCC[5] = { 0 };
    int Version = 0;
    char ConversionDate[256] = { 0 };

    char Vector[5] = { 0 };
    int fileCount = 0;

    TSXT(std::istream& stream)
    {
        stream.read(FourCC, 4);
        Version = readNu32(stream);
        if (Version)
        {
            int strSize = readNu32(stream);
            stream.read(ConversionDate, strSize);
        }
        stream.read(Vector, 4);
        fileCount = readNu32(stream);
    };
    TSXT() = default;
};

struct NXG_TEXTURES
{
private:
    int RESHSize = 0;
    int TSXTSize = 0;
    char fourCC[8] = { 0 };
    int TSXTCount = 0;
public:
    TSXT Textures;
    std::vector<std::string> Paths = std::vector<std::string>();

    NXG_TEXTURES(std::istream& stream)
    {
        RESHSize = readNu32(stream);
        stream.ignore(RESHSize);
        TSXTSize = readNu32(stream);
        stream.read(fourCC, 8);
        TSXTCount = readNu32(stream);
        Textures = TSXT(stream);
        printf("ConversionDate: %s\n", Textures.ConversionDate);
        printf("FileCount: %d\n", Textures.fileCount);

        if (Textures.Version == 0x01 || Textures.Version == 0x00)
        {
            for (int i = 0; i < Textures.fileCount; i++)
            {
                Paths.push_back(readNXGFile(stream));
            };
        }

        if (Textures.Version == 0x0C)
        {
            for (int i = 0; i < Textures.fileCount; i++)
            {
                Paths.push_back(readDX11FileG1(stream));
            }
        }

        if (Textures.Version == 0x0E)
        {
            for (int i = 0; i < Textures.fileCount; i++)
            {
                std::string path = readDX11FileG2(stream);
                if (!path.empty())
                {
                    Paths.push_back(path);
                }
            }
        }
        // filter paths
        for (std::string& path : Paths)
        {
            if (path == "")
            {
                Textures.fileCount--;
                Paths.pop_back();
            }
            std::replace(path.begin(), path.end(), '\\', '/');
        }
    }
};

/*
 #pragma endian big

struct NXGFile
{
    u128 Checksum;
    u32 strSize;
    char filePath[strSize];
    u32 Type;
}[[name(filePath)]];

struct Gen1_DX11File
{
    u128 Checksum;
    u32 someData;
    u8 strSize;
    char filePath[strSize];
    u8 Attribute;
}[[name(filePath)]];

struct Gen2_DX11File
{
    u128 Checksum;
    if (Checksum)
    {
        u32 someData;
        u8 strSize;
        char filePath[strSize];
        u8 Attribute;
        u32 someData2;
        u32 someData3;
    }
    else
    {
        u16 strSize;
        char filePath[strSize];
        u32 someData;
        u32 someData2;
        u32 someData3;
    }
}[[name(filePath)]];

struct TSXT
{
    char FourCC[4];
    u32 Version;
    if (Version)
    {
        u32 strSize;
        char ConversionDate[strSize];
    }

    char VTOR[4];
    u32 fileCount;

    if (Version == 0x01)
    {
        NXGFile fileData[fileCount];
    }
    if (Version == 0x0C)
    {
        Gen1_DX11File fileData[fileCount];
    }
    if (Version == 0x0E)
    {
        Gen2_DX11File fileData[fileCount];
    }
}[[name(ConversionDate)]];


struct NXG_TEXTURES
{
    u32 RESHSize[[hidden]];
    padding[RESHSize];

    // TXST
    u32 StructSize;
    char FourCC[8];
    u32 Entries;

    TSXT Textures[Entries];
};

be NXG_TEXTURES file @ 0x00;
 */