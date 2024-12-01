#pragma once


#include <stddef.h>
#include <assert.h>
#include <unordered_map>
#include "GstUnityPlugin.h"

#if SUPPORT_D3D11
#include <assert.h>
#include <d3d11.h>
#endif

#if SUPPORT_OPENGL_UNIFIED
#if UNITY_IOS || UNITY_TVOS
#	include <OpenGLES/ES2/gl.h>
#elif UNITY_ANDROID || UNITY_WEBGL
#	include <GLES3/gl3.h>
#elif UNITY_OSX
#	include <OpenGL/gl3.h>
#elif UNITY_WIN
// On Windows, use gl3w to initialize and load OpenGL Core functions. In principle any other
// library (like GLEW, GLFW etc.) can be used; here we use gl3w since it's simple and
// straightforward.
#	include "gl3w/gl3w.h"
#elif UNITY_LINUX
#	define GL_GLEXT_PROTOTYPES
#	include <GL/gl.h>
#elif UNITY_EMBEDDED_LINUX
#	include <GLES2/gl2.h>
#if SUPPORT_OPENGL_CORE
#	define GL_GLEXT_PROTOTYPES
#	include <GL/gl.h>
#endif
#else
#	error Unknown platform
#endif
#endif

#if SUPPORT_VULKAN
#include <stddef.h>
#include <map>
#include <vector>
#include <math.h>
#define VK_NO_PROTOTYPES
#include "Unity/IUnityGraphicsVulkan.h"
#endif


struct IUnityInterfaces;
typedef void* (UNITY_INTERFACE_API* TextureCreationEvent)(int width, int height, int depth);


class UnityTextureController
{
public:

    static UnityTextureController* 
    CreateUnityTextureController(UnityGfxRenderer apiType);

	static void UNITY_INTERFACE_API
		OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

	~UnityTextureController();

	static UnityTextureController* Instance();

	static bool WriteTextureDataToFile(void* texData, int width, int height, 
		int depth, std::string path);

    virtual bool GetUsesReverseZ() = 0;

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, 
        IUnityInterfaces* interfaces) = 0;

	virtual int BeginModifyTexture(int id, void* texHandle, int texWidth, 
        int texHeight, int texDepth) = 0;

	virtual void EndModifyTexture(int id, void* texHandle, unsigned char* img, 
		int texWidth, int texHeight, int pitch) = 0;

	virtual void* CreateTexture(int w, int h, int d) = 0;

	virtual bool DeleteTexture(void* tex) = 0;

	virtual std::string GetTypeName() = 0;

	virtual void DeleteBuffers();

    virtual void SetTextureData(int id, void* texHandle, int texW, 
        int texH, int texFormat, void* imageData);

protected:

	std::unordered_map<int, void*> _texData;
	std::unordered_map<int, void*> _texBuffers;
	int _texDataSize = 0;
	UnityGfxRenderer _apiType;
};


#if SUPPORT_OPENGL_UNIFIED
class GlTextureController : public UnityTextureController
{
public:
	GlTextureController(UnityGfxRenderer apiType);

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, 
        IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ() { return false; }

	virtual int BeginModifyTexture(int id, void* texHandle, int texWidth, 
        int texHeight, int texDepth);

	virtual void EndModifyTexture(int id, void* texHandle, unsigned char* img, 
		int texWidth, int texHeight, int texDepth);

	virtual void* CreateTexture(int w, int h, int d);

	virtual bool DeleteTexture(void* tex);

	virtual std::string GetTypeName() { return "OpenGL"; }

private:

	void CreateResources();
	void ReleaseResources();

	GLuint _textureFormat;
	GLenum _formats[5] = {(GLuint)0, GL_RED, GL_RG, GL_RGB, GL_RGBA};
	GLuint m_VertexShader;
	GLuint m_FragmentShader;
	GLuint m_Program;
	GLuint m_VertexArray;
	GLuint m_VertexBuffer;
	int m_UniformWorldMatrix;
	int m_UniformProjMatrix;
};
#endif

#if SUPPORT_D3D11
class Dx11TextureController : public UnityTextureController
{
public:
	Dx11TextureController(UnityGfxRenderer apiType);

	//~Dx11TextureController() { }

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type, 
        IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ();

	virtual int BeginModifyTexture(int id, void* texHandle, int texWidth, 
        int texHeight, int texDepth);

	virtual void EndModifyTexture(int id, void* texHandle, unsigned char* img, 
		int texWidth, int texHeight, int texDepth);

	virtual void* CreateTexture(int w, int h, int d);

	virtual bool DeleteTexture(void* tex);

	virtual std::string GetTypeName() { return "DX11"; }

private:

	void CreateResources();
	void ReleaseResources();


    ID3D11Device* _device;
	ID3D11Buffer* m_VB; // vertex buffer
	ID3D11Buffer* m_CB; // constant buffer
	ID3D11VertexShader* m_VertexShader;
	ID3D11PixelShader* m_PixelShader;
	ID3D11InputLayout* m_InputLayout;
	ID3D11RasterizerState* m_RasterState;
	ID3D11BlendState* m_BlendState;
	ID3D11DepthStencilState* m_DepthState;
};
#endif


#if SUPPORT_VULKAN

extern "C" void VkTextureController_OnPluginLoad(IUnityInterfaces*);

struct VulkanBuffer
{
	VkBuffer buffer;
	VkDeviceMemory deviceMemory;
	void* mapped;
	VkDeviceSize sizeInBytes;
	VkDeviceSize deviceMemorySize;
	VkMemoryPropertyFlags deviceMemoryFlags;
};

class VkTextureController : public UnityTextureController
{
public:
	VkTextureController(UnityGfxRenderer apiType);
	virtual ~VkTextureController();

	virtual void ProcessDeviceEvent(UnityGfxDeviceEventType type,
		IUnityInterfaces* interfaces);

	virtual bool GetUsesReverseZ() { return true; }

	virtual void DrawSimpleTriangles(const float worldMatrix[16], int triangleCount,
		const void* verticesFloat3Byte4);

	virtual int BeginModifyTexture(int id, void* texHandle, int texWidth, 
        int texHeight, int texDepth);

	virtual void EndModifyTexture(int id, void* texHandle, unsigned char* img, 
		int texWidth, int texHeight, int texDepth);

	virtual void* BeginModifyVertexBuffer(void* bufferHandle, size_t* outBufferSize);

	virtual void EndModifyVertexBuffer(void* bufferHandle);

	virtual void* CreateTexture(int w, int h, int d) { return NULL; }

	virtual bool DeleteTexture(void* tex) { return false; }

	virtual std::string GetTypeName() { return "Vulkan"; }

private:
	typedef std::vector<VulkanBuffer> VulkanBuffers;
	typedef std::map<unsigned long long, VulkanBuffers> DeleteQueue;

private:
	bool CreateVulkanBuffer(size_t bytes, VulkanBuffer* buffer, VkBufferUsageFlags usage);
	void ImmediateDestroyVulkanBuffer(const VulkanBuffer& buffer);
	void SafeDestroy(unsigned long long frameNumber, const VulkanBuffer& buffer);
	void GarbageCollect(bool force = false);

private:
	int _streamWidth = 0;
	int _streamHeight = 0;
	int _streamDepth = 0;
	int _streamPitch = 0;
	bool _dimsSet = false;
	IUnityGraphicsVulkan* m_UnityVulkan;
	UnityVulkanInstance m_Instance;
	VulkanBuffer m_TextureStagingBuffer;
	VulkanBuffer m_VertexStagingBuffer;
	std::map<unsigned long long, VulkanBuffers> m_DeleteQueue;
	VkPipelineLayout m_TrianglePipelineLayout;
	VkPipeline m_TrianglePipeline;
	VkRenderPass m_TrianglePipelineRenderPass;
};

#endif

