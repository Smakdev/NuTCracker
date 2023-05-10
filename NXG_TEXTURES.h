#pragma once

typedef uint32_t nu32;

enum NuTAttribute : nu32
{
    diffuse = 0x000,
    normalMap = 0x100,
    Lightmap = 0x400,
    Skydome = 0x700,
    Cubemap = 0x600,
    BRDF = 0x0C00,
    unk1 = 0x10000,
    unk2 = 0x20000
};

std::string toStr(NuTAttribute attribute)
{
    switch(attribute)
    {
        case NuTAttribute::diffuse:
            return "Diffuse";
            break;
        case NuTAttribute::normalMap:
            return "NormalMap";
            break;
        case NuTAttribute::Lightmap:
            return "Lightmap";
            break;
        case NuTAttribute::Skydome:
            return "Skydome";
            break;
        case NuTAttribute::Cubemap:
            return "Cubemap";
            break;
        case NuTAttribute::BRDF:
            return "BRDF";
            break;
        case NuTAttribute::unk1:
            return "Unknown";
            break;
        case NuTAttribute::unk2:
            return "Unknown";
            break;
        default:
            return "Undocumented";
            break;
    }
}

struct NuTPath
{
    std::string Checksum;
    std::string Path;
    NuTAttribute attribute;

    NuTPath(char ichecksum[], char iPath[], int PathSize, NuTAttribute iattribute)
    {
        Checksum = ichecksum;
        for (int i = 0; i < PathSize; i++)
		{
			if (iPath[i] == '\\')
			{
				iPath[i] = '/';
			}
		}
        Path = iPath;
        attribute = iattribute;
    }
};

std::string getFileName(std::string cur)
{
	cur = cur.substr(cur.find_last_of('/') + 1);
	return cur.substr(0, cur.find_last_of('.'));
}

std::string getFileFolder(std::string cur)
{
    int count = 0;

    if (cur.find_first_of('/') == cur.npos)
    {
        return cur;
    }

    for (int i = cur.size(); i --> 0;)
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

struct nuVec
{
    char identifier[4];
    nu32 size;
};

nu32 getNu32(std::istream& stream)
{
    char memblock[4] = {0};
    stream.read(memblock, 4);
    return (((uint8_t)memblock[0]) << 24) | (((uint8_t)memblock[1]) << 16) | (((uint8_t)memblock[2]) << 8) | (((uint8_t)memblock[3]) << 0);
};

class RESH
{
private:
	nu32 headersize;
    char FourCC[0xFF] = { 0 };
    nu32 Version = 0;
    uint32_t someData2 = 0;
    nuVec Vector =  { };
    char SomeData3[0x14] = { 0 };
    char strSize = 0;
    char Parent[256] = { 0 };
    uint16_t someData4 = 0;

public:

    void readRESH(std::istream& stream)
	{
		headersize = getNu32(stream);
		stream.read(FourCC, 0x0C);
		Version = getNu32(stream);
		stream.read((char*)&someData2, 4);
		stream.read((char*)&Vector, sizeof(Vector));
		stream.read((char*)SomeData3, 0x14);
		stream.read(&strSize, 1);
		stream.read((char*)Parent, (unsigned char)strSize);
		stream.read((char*)&someData4, 2);
	};
    RESH() = default;
	RESH(std::istream& stream)
	{
		readRESH(stream);
	}

	char* getParent()
	{
		return Parent;
	}
};

class TSXT
{
private:
	char CCID = 0;
	char FourCC[0xFF] = { 0 };
	nu32 nuEntries = 0;
	char TSXTID[5] = { 0 };
	nu32 nuTSXTC = 0;
	nu32 textSize = 0;
    char convTEXT[256] = { 0 };

public:
	void ReadTSXT(std::istream& stream)
	{
		stream.read(&CCID, 1);
		stream.read(FourCC, 0x8);
		nuEntries = getNu32(stream);
		stream.read(TSXTID, 4);
		nuTSXTC = getNu32(stream);
		if (nuTSXTC)
		{
			textSize = getNu32(stream);
			stream.read(convTEXT, (unsigned char)textSize);
		}
	}

    TSXT() = default;
	TSXT(std::istream& stream)
	{
		ReadTSXT(stream);
	}

	char* getText()
	{
        if (nuEntries)
        {
            return convTEXT;
        }
        return nullptr;
	}
};


class Files
{
private:

    char vectorIdentifier[4] = { 0 };
    nu32 vectorSize = 0;

    char Checksum[16]; // 128 bit checksum
    nu32 PathSize;
    char Path[256] = { 0 };
    NuTAttribute Attribute;
    std::vector<NuTPath> FilePaths = std::vector<NuTPath>();


public:

    void readFiles(std::istream& stream)
    {
        stream.read(vectorIdentifier, 4);
        vectorSize = getNu32(stream);
        for (int i = 0; i < vectorSize; i++)
        {
            stream.read(Checksum, 16);
            PathSize = getNu32(stream);
            stream.read(Path, PathSize);
            Attribute = (NuTAttribute)getNu32(stream);
            FilePaths.push_back(NuTPath(Checksum, Path, PathSize, Attribute));
        }

    };
    Files() = default;
    Files(std::istream& stream)
    {
        readFiles(stream);
    };

    std::vector<NuTPath> getPaths()
    {
        return FilePaths;
    }
};

struct NXG_TEXTURES
{
    RESH resourceHeader;
    TSXT text;
    Files files;
    NXG_TEXTURES(std::istream& stream)
    {
        // READ RESOURCE HEADER
        resourceHeader = RESH(stream);
        stream.ignore(2);
        text = TSXT(stream);
        files = Files(stream);
    }
};
