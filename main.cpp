#include<string>
#include<cinttypes>
#include<fstream>
#include<iostream>
#include<vector>
#include"Dir.h"
#include"DDSHelper.h"
#include"NXG_TEXTURES.h"


char gFCC;

bool ReadDDS(std::ifstream& stream)
{
	DDS_HEADER header{};
	stream.read((char*)&header, sizeof(DDS_HEADER));
	if (header.pf.FourCC == 0x30315844)
	{
		DDS_HEADER_DXT10 header10{};
		stream.read((char*)&header10, sizeof(DDS_HEADER_DXT10));
	}
	//printf("MipMapCount: %d\n", header.MipMapCount);
	if (header.pf.FourCC == 0x31545844)
	{
		//printf("DXT1");
		gFCC = '1';
		for (int i = 0; i < header.MipMapCount + 1; i++)
		{
			if (header.Width == 0 && header.Height != 0)
				header.Width = 1;
			if (header.Height == 0 && header.Width != 0)
				header.Height = 1;
			//printf("MipMap %d x %d\n", header.Height, header.Width);
			int TWidth = ((header.Width + 3) / 4) * 4;
			int THeight = ((header.Height + 3) / 4) * 4;
			float size = (TWidth * THeight);
			int chunkC = size / 16.0f + 0.9999;
			//printf("Size %d\n", chunkC * 8);
			stream.seekg(chunkC * 8, stream.cur);

			header.Width /= 2;
			header.Height /= 2;
		}
	}
	else if (
		header.pf.FourCC == 0x35545844 ||
		header.pf.FourCC == 0x34545844 ||
		header.pf.FourCC == 0x33545844 ||
		header.pf.FourCC == 0x32545844)
	{
		//printf("DXT%c", );
		gFCC = header.pf.FourCC >> 24 & 0xFF;
		for (int i = 0; i < header.MipMapCount + 1; i++)
		{
			if (header.Width == 0 && header.Height != 0)
				header.Width = 1;
			if (header.Height == 0 && header.Width != 0)
				header.Height = 1;
			//printf("MipMap: %d x %d\n", header.Height, header.Width);
			int RoundWidth = ((header.Width + 3) / 4) * 4;
			int RoundHeight = ((header.Height + 3) / 4) * 4;
			int chunkC = (RoundWidth / 4) * (RoundHeight / 4);
			//printf("Block: %d\n", chunkC);
			//printf("Size: %d\n", chunkC * 16);
			stream.seekg(chunkC * 16, stream.cur);

			header.Width /= 2;
			header.Height /= 2;
		}
	}
	else
	{
		printf("Unknown type %x\n", header.pf.FourCC);
		std::cin.get();
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		printf("Not enough arguments.\n");
		printf("Usage is: NuTCracker File\n");
		printf("Usage is: NuTCracker File Destination\n");
		std::cin.get();
		return 1;
	}
	std::string srcFile = argv[1];
	std::string resDir = " ";

	printf("\nNuTCracker\n");
	printf("NXG_TEXTURES extraction tool, developed by SmakDev\n");

	if (argc == 3)
	{
		resDir = argv[2];

		for (int i = 0; i < resDir.size(); i++)
		{
			if (resDir[i] == '\\')
			{
				resDir[i] = '/';
			}
		}
		resDir += '/';
	}

	else if (argc == 2)
	{
		for (int i = 0; i < srcFile.size(); i++)
		{
			if (srcFile[i] == '\\')
			{
				srcFile[i] = '/';
			}
		}

		std::string Name = " ";
		std::string Path = "";
		resDir = GetFolder(srcFile) + GetFileName(srcFile) + '/';
		CreateDir(GetFolder(srcFile), GetFileName(srcFile));
	}

	// READ NXG_TEXTURES HEADERS
	std::ifstream NXGStream(srcFile.c_str(), std::ios::binary | std::ios::ate);
	if (!NXGStream.is_open())
	{
		printf("Could not open %s", srcFile.c_str());
		std::cin.get();
		return 1;
	}
	NXGStream.seekg(0, NXGStream.beg);


	NXG_TEXTURES file = NXG_TEXTURES(NXGStream);

	NXGStream.close();


	std::vector<NuTPath> Paths = file.files.getPaths();

	int dupe = 1;
	// HANDLE LIGHTMAPS
	for (int i = 0; i < Paths.size(); i++)
	{
		if (Paths[i].Path == "Lightmap")
		{
			Paths[i].Path = "LM/Lightmap/Lightmap" + std::to_string(dupe++) + ".dds";
		}
		//printf("%s/%s\n", getFileFolder(Paths[i].Path).c_str(), getFileName(Paths[i].Path).c_str());
	}

	// CREATE SUBFOLDERS

	for (int i = 0; i < Paths.size(); i++)
	{
		std::string dirName = resDir + getFileFolder(Paths[i].Path);
		CreateDir(resDir, getFileFolder(Paths[i].Path));
	}

	// READ DDS FILES
	printf("\nDestination: %s\n", resDir.c_str());
	printf("Source: %s\n\n", srcFile.c_str());
	printf("  Offset           Size          Name        FourCC\n");
	printf("---------------------------------------------------\n");

	std::ifstream DDSStream(srcFile.c_str(), std::ios::binary | std::ios::ate);
	if (!DDSStream.is_open())
	{
		printf("Could not open %s", srcFile.c_str());
		std::cin.get();
		return 1;
	}

	uint64_t FileSize = DDSStream.tellg();
	DDSStream.seekg(0, DDSStream.beg);

	int dupeCount = 0;
	int position = 0 ;
	uint32_t ResCount = 0;
	uint32_t NavByte = 0;
	std::string filename;
	for (uint64_t i = 0; i < FileSize; i++)
	{
		DDSStream.read((char*)&NavByte, 1);
		if ((NavByte & 0xFF) == 'D')
		{
			DDSStream.read(((char*)&NavByte) + 1, 3);
			if (NavByte != DDSMagic)
			{
				DDSStream.seekg(i + 1, DDSStream.beg);
				continue;
			}
			position = (unsigned long)DDSStream.tellg() - 4;
			bool good = ReadDDS(DDSStream);
			if (good)
			{
				ResCount++;
				filename = resDir + getFileFolder(Paths[ResCount - 1].Path) + '/' + getFileName(Paths[ResCount - 1].Path) + ".dds";

				uint64_t size = ((uint64_t)DDSStream.tellg()) - i;
				DDSStream.seekg(i, DDSStream.beg);
				std::ofstream OStream(filename.c_str(), std::ios::binary);
				if (!OStream.is_open())
				{
					printf("\nCould not write to %s", filename.c_str());
					std::cin.get();
					return -1;
				}
				char* buffer = new char[size];
				DDSStream.read(buffer, size);
				OStream.write(buffer, size);
				OStream.close();
				delete[] buffer;
				printf("%d  %016x ", ResCount, position);
				printf("%-*lu", 11, (unsigned long)size);
				printf("%s%s.dds  ", getFileFolder(Paths[ResCount - 1].Path).c_str(),GetFileName(Paths[ResCount - 1].Path).c_str());
				printf("DXT%c\n", gFCC);
				i = DDSStream.tellg();
				i--;
			}
			else
			{
				DDSStream.seekg(i + 1, DDSStream.beg);
			}
		}
	}
	DDSStream.close();
	printf("\n---------------------------------------------------\n");
	printf("Parent File: %s\n", file.resourceHeader.getParent());
	printf("Conversion Date: %s\n", file.text.getText());
	if (ResCount)
	{
		printf("Found %d DDS files\n", ResCount);
	}
	else
	{
		printf("No DDS Files found in file\n");
#ifdef _WIN32
		RemoveDirectoryA(resDir.c_str());
#endif // _WIN32
#ifdef __linux__
		rmdir(resDir.c_str());
#endif
	}
	std::cin.get();
	return 0;
}
