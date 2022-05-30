#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include "WaveLoader.h"
#include "WaveFileDialog.h"



char* GetFileNameFromPath(char* Path)
{
	if (Path == NULL)
		return NULL;

	char* FileName = Path;
	for (char* Cur = Path; *Cur != '\0'; Cur++)
	{
		if (*Cur == '/' || *Cur == '\\')
			FileName = Cur + 1;
	}

	return FileName;
}

#define TEX_SIZE 128
#define TEXTURE_SIZE 128

void SaveAsDisplayList(WaveModelData* Data, const int8_t* OutPath)
{
	FILE* File = fopen(OutPath, "wb");
	printf("%s\n", OutPath);

	uint32_t* MaterialVertices = (uint32_t*)malloc(Data->Material->MaterialCount * sizeof(uint32_t));
	uint32_t MaterialIndex = 0;
		
	for (uint32_t i = 0; i < Data->Material->MaterialCount; i++)
	{
		char* FileName = GetFileNameFromPath(Data->Material[i].DiffuseTexture);
		strcat(Data->Material[i].MaterialName, "Model");
		char* NewName = (char*)malloc(strlen(FileName) + strlen("Model") + 1);
		strcpy(NewName, "Tex");
		strcat(NewName, FileName);
		strcpy(FileName, NewName);
		
		FileName[strlen(FileName) - 4] = '\0';
	//	strcat(FileName, ".h");
		printf("#include \"%s\"\n", FileName);
		printf("%d\n", Data->Material[i].VertexIndex);
		fprintf(File, "#include \"%s_pcx.h\"\n", FileName);
	}

	for (uint32_t i = 0; i < Data->Material->MaterialCount; i++)
	{
		if (i + 1 != Data->Material->MaterialCount)
			MaterialVertices[i] = Data->Material[i + 1].VertexIndex - Data->Material[i].VertexIndex;
		else
			MaterialVertices[i] = Data->VertexCount - Data->Material[i].VertexIndex;

	//	printf("mat: %d\n", Data->Material[i].VertexIndex - Data->Material[i + 1].VertexIndex);
	}

	for (uint32_t j = 0; j < Data->Material->MaterialCount; j++)
	{
		uint32_t End = 0;
		if (j + 1 != Data->Material->MaterialCount)
			End = Data->Material[j + 1].VertexIndex;
		else
			End = Data->VertexCount;
		
		uint32_t i = Data->Material[j].VertexIndex;

		fprintf(File, "u32 %s[] =\n", Data->Material[j].MaterialName);
		fprintf(File, "{\n");

		fprintf(File, "\t%d,\n", (MaterialVertices[j] - 2) * 5 + 11);
		
		fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16),\n");
		fprintf(File, "\tGL_TRIANGLE,\n");
		fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);
		fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
		i++;

		for (; i < End - 1; i++)
		{
			fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_NOP),\n");
			fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);
			fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);
			fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
			fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
		}

		fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_END),\n");
		fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);
		fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
		fprintf(File, "};\n");
	}

	//remove last two chars from OutPath and save it in a new string
	printf("OutPath: %s\n", OutPath);
	
	size_t lenght = strlen(OutPath);
	char* FunctionName = malloc(lenght + 1);
	strcpy(FunctionName, OutPath);
	FunctionName[lenght - 2] = '\0';
	
	printf("FunctionName: %s\n", FunctionName);

	fprintf(File, "int %sTextures[%d];\n", FunctionName, Data->Material->MaterialCount);
	fprintf(File, "void %sLoadTextures()\n", FunctionName);
	fprintf(File, "{\n");
	for (uint32_t i = 0; i < Data->Material->MaterialCount; i++)
	{
		fprintf(File, "\t%sTextures[%d] = LoadTexture(TEXTURE_SIZE_%d, (u8*)%s_pcx);\n", FunctionName, i, TEXTURE_SIZE, Data->Material[i].DiffuseTexture);
	}
	fprintf(File, "}\n");

	fprintf(File, "void %sDraw()\n", FunctionName);
	fprintf(File, "{\n");
	for (uint32_t i = 0; i < Data->Material->MaterialCount; i++)
	{
		fprintf(File, "\tglBindTexture(GL_TEXTURE_2D, %sTextures[%d]);\n", FunctionName, i);
		fprintf(File, "\tglCallList((u32*)%s);\n", Data->Material[i].MaterialName);
	}
	fprintf(File, "}\n");

	fprintf(File, "void %sDeleteTextures()\n", FunctionName);
	fprintf(File, "{\n");
	fprintf(File, "\tglDeleteTextures(%d, %sTextures);\n", Data->Material->MaterialCount, FunctionName);
	fprintf(File, "}\n");

	printf("enddddddd\n");

	free(FunctionName);

	/*
	uint32_t i;
	for (i = 0; i < Data->VertexCount - 1; i++)
	{
		if (Data->Material[MaterialIndex].VertexIndex == i)
		{
			if (i != 0)
			{
				fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_END),\n");

				fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);

				fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

				fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
				fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);

				fprintf(File, "};\n");
				
			//	MaterialIndex++;
				i++;
			}

			fprintf(File, "u32 %s[] =\n", Data->Material[MaterialIndex].MaterialName);
			fprintf(File, "{\n");

			fprintf(File, "\t%d,\n", (MaterialVertices[MaterialIndex] - 2) * 5 + 11);
			fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16),\n");
			fprintf(File, "\tGL_TRIANGLE,\n");

			fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);

			fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

			fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
			fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
			MaterialIndex++;
		}

		fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_NOP),\n");

		fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);

		fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

		fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
	}
	
	fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_END),\n");

	fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);

	fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

	fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
	fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);

	fprintf(File, "};\n");
	
	//remove last two chars from OutPath and save it in a new string
	char * FunctionName = malloc(strlen(OutPath) - 1);
	strncpy(FunctionName, OutPath, strlen(OutPath) - 1);
	FunctionName[strlen(FunctionName) - 1] = '\0';
	
	
	fprintf(File, "void %sDraw()\n", FunctionName);
	fprintf(File, "{\n");
	for (uint32_t i = 0; i < Data->Material->MaterialCount; i++)
	{
		fprintf(File, "\tglCallList((u32*)%s);\n", Data->Material[i].MaterialName);
	}
	fprintf(File, "}\n");
	printf("enddddddd\n");

	free(FunctionName);
	*/
	/*
	fprintf(File, "u32 %s[] =\n", Data->Material[MaterialIndex].MaterialName);
	fprintf(File, "{\n");
			
	fprintf(File, "\t%d,\n", 255);
	fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16),\n");
	fprintf(File, "\tGL_TRIANGLE,\n");
			
	fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[0].TexCoords.x * TEX_SIZE, Data->Mesh[0].TexCoords.y * TEX_SIZE);
			
	fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[0].Normals.x, Data->Mesh[0].Normals.y, Data->Mesh[0].Normals.z);
			
	fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[0].Vertices.x, Data->Mesh[0].Vertices.y);
	fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[0].Vertices.z);
	
	for (uint32_t i = 0; i < Data->VertexCount; i++)
	{
	//	Data->Mesh[i].Vertices.x;
	//	Data->Mesh[i].Vertices.y;
	//	Data->Mesh[i].Vertices.z;
	//
	//	Data->Mesh[i].Normals.x;
	//	Data->Mesh[i].Normals.y;
	//	Data->Mesh[i].Normals.z;
	//
	//	Data->Mesh[i].TexCoords.x;
	//	Data->Mesh[i].TexCoords.y;

		if (Data->HasMaterial)
		{
			if (Data->Material[MaterialIndex].VertexIndex == i)
			{
				fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_NOP),\n");

				fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * 128, Data->Mesh[i].TexCoords.y * 128);

				fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

				fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
				fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);

				fprintf(File, "};\n");

				printf("%d %d\n", MaterialIndex, i);
				MaterialIndex++;
				i++;

				fprintf(File, "u32 %s[] =\n", Data->Material[MaterialIndex].MaterialName);
				fprintf(File, "{\n");

				fprintf(File, "\t%d,\n", 255);
				fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16),\n");
				fprintf(File, "\tGL_TRIANGLE,\n");

				fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * TEX_SIZE, Data->Mesh[i].TexCoords.y * TEX_SIZE);

				fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

				fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
				fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
			}
		}
		else
		{
			//	MeshInfo.Vertices[i].Color.x = Data->Mesh[i].VertexColor.x;
			//	MeshInfo.Vertices[i].Color.y = Data->Mesh[i].VertexColor.y;
			//	MeshInfo.Vertices[i].Color.z = Data->Mesh[i].VertexColor.z;
		}
		
		fprintf(File, "\tFIFO_COMMAND_PACK(FIFO_TEX_COORD, FIFO_NORMAL, FIFO_VERTEX16, FIFO_NOP),\n");

		fprintf(File, "\tTEXTURE_PACK(floattot16(%f), floattot16(%f)),\n", Data->Mesh[i].TexCoords.x * 128, Data->Mesh[i].TexCoords.y * 128);

		fprintf(File, "\tNORMAL_PACK(floattov10(%f), floattov10(%f), floattov10(%f)),\n", Data->Mesh[i].Normals.x, Data->Mesh[i].Normals.y, Data->Mesh[i].Normals.z);

		fprintf(File, "\tVERTEX_PACK(floattov16(%f), floattov16(%f)),\n", Data->Mesh[i].Vertices.x, Data->Mesh[i].Vertices.y);
		fprintf(File, "\tVERTEX_PACK(floattov16(%f), 0),\n", Data->Mesh[i].Vertices.z);
		
		
	}
	fprintf(File, "};\n");
	*/
	WaveFreeModel(Data);
	free(MaterialVertices);
	fclose(File);
}

//create a function to change the file extension from path
char* ChangeFileExtension(char* Path, char* Extension)
{
	char* File = GetFileNameFromPath(Path);
	File[strlen(File) - 4] = '\0';
	strcat(File, Extension);
	return File;
}

int32_t main()
{
	int8_t Path[4096];
	int8_t Name[4096];
	WaveModelData Data;
	if (WaveOpenFileDialog(Path, NULL))
	{
		Data = WaveLoadModel(Path, WAVE_LOAD_MATERIAL | WAVE_FLIP_UVS | WAVE_GEN_NORMALS | WAVE_GEN_UVS);
		
		SaveAsDisplayList(&Data, ChangeFileExtension(Path, ".h"));
	}

	return 0;
}