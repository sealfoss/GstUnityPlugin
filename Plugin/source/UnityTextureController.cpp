#include <iostream>
#include <fstream>
#include <cstdint>
#include "UnityTextureController.h"

#if SUPPORT_D3D11
#include <windows.h>
#endif

using namespace std;

#pragma pack(push, 1)
struct BMPFileHeader 
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};

// Bitmap info header structure
struct BMPInfoHeader 
{
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
#pragma pack(pop)

static UnityTextureController* s_TextureController = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;


UnityTextureController::~UnityTextureController()
{
	DeleteBuffers();
	s_TextureController = NULL;
}

UnityTextureController* UnityTextureController::Instance()
{
	return s_TextureController;
}

UnityTextureController* 
UnityTextureController::CreateUnityTextureController(UnityGfxRenderer apiType)
{

#if SUPPORT_D3D11
	if (apiType == kUnityGfxRendererD3D11)
	{
		return new Dx11TextureController(apiType);
	}
#endif 

#if SUPPORT_OPENGL_UNIFIED || SUPPORT_OPENGL_CORE
	if (apiType == kUnityGfxRendererOpenGLCore 
        || apiType == kUnityGfxRendererOpenGLES30)
	{
		return new GlTextureController(apiType);
	}
#endif // if SUPPORT_OPENGL_UNIFIED

#if SUPPORT_VULKAN
	if (apiType == kUnityGfxRendererVulkan)
	{
		return new VkTextureController(apiType);
	}
#endif // if SUPPORT_VULKAN

	// Unknown or unsupported graphics API
	return NULL;
}

void UNITY_INTERFACE_API 
UnityTextureController::OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		assert(s_TextureController == NULL);
		s_DeviceType = UnityGraphicsInterface()->GetRenderer();
		s_TextureController = CreateUnityTextureController(s_DeviceType);
	}

	// Let the implementation process the device related events
	if (s_TextureController)
	{
		s_TextureController->ProcessDeviceEvent(eventType, UnityInterfaces());
	}

	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		delete s_TextureController;
		s_TextureController = NULL;
		s_DeviceType = kUnityGfxRendererNull;
	}
}

void UnityTextureController::SetTextureData(int id, void* texHandle, int texW, 
        int texH, int texDepth, void* imageData)
{
	unsigned char* img = (unsigned char*)imageData;
    int pitch = BeginModifyTexture(id, texHandle, texW, texH, texDepth);
	EndModifyTexture(id, texHandle, img, texW, texH, pitch);
}

void UnityTextureController::DeleteBuffers()
{
	for (auto& pair : _texData) 
	{
		delete[] static_cast<char*>(pair.second);
		pair.second = NULL;
	}
}

bool UnityTextureController::WriteTextureDataToFile(void* texData, int width, 
	int height, int depth, string path)
{
	bool success = false;
#if UNITY_WIN
	const char* filePath = path.c_str();
	BMPFileHeader fileHeader;
	BMPInfoHeader infoHeader;
	int imageSize = width * height * depth; // RGBA (4 bytes per pixel)
	string msg;

	// Fill the file header
	fileHeader.bfType = 0x4D42; // 'BM' in hexadecimal
	fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + imageSize;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

	// Fill the info header
	infoHeader.biSize = sizeof(BMPInfoHeader);
	infoHeader.biWidth = width;
	infoHeader.biHeight = -height; // Negative height to indicate top-down bitmap
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 32; // 32 bits for RGBA
	infoHeader.biCompression = BI_RGB;
	infoHeader.biSizeImage = imageSize;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biClrImportant = 0;

	// Write the headers and image data to the file
	ofstream file(filePath, std::ios::out | std::ios::binary);

	if (file)
	{
		file.write(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
		file.write(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));
		file.write(reinterpret_cast<char*>(texData), imageSize);
		file.close();

		if (file)
			success = true;
	}
#endif
	return success;
}