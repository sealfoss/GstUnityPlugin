#include "UnityTextureController.h"

using namespace std;

#if SUPPORT_OPENGL_UNIFIED

static GLuint CreateShader(GLenum type, const char* sourceText)
{
	GLuint ret = glCreateShader(type);
	glShaderSource(ret, 1, &sourceText, NULL);
	glCompileShader(ret);
	return ret;
}

enum VertexInputs
{
	kVertexInputPosition = 0,
	kVertexInputColor = 1
};

// Simple vertex shader source
#define VERTEX_SHADER_SRC(ver, attr, varying)						\
	ver																\
	attr " highp vec3 pos;\n"										\
	attr " lowp vec4 color;\n"										\
	"\n"															\
	varying " lowp vec4 ocolor;\n"									\
	"\n"															\
	"uniform highp mat4 worldMatrix;\n"								\
	"uniform highp mat4 projMatrix;\n"								\
	"\n"															\
	"void main()\n"													\
	"{\n"															\
	"	gl_Position = (projMatrix * worldMatrix) * vec4(pos,1);\n"	\
	"	ocolor = color;\n"											\
	"}\n"															\

static const char* kGlesVProgTextGLES2 = VERTEX_SHADER_SRC("\n", "attribute", "varying");
static const char* kGlesVProgTextGLES3 = VERTEX_SHADER_SRC("#version 300 es\n", "in", "out");
#if SUPPORT_OPENGL_CORE
static const char* kGlesVProgTextGLCore = VERTEX_SHADER_SRC("#version 150\n", "in", "out");
#endif

#undef VERTEX_SHADER_SRC


// Simple fragment shader source
#define FRAGMENT_SHADER_SRC(ver, varying, outDecl, outVar)	\
	ver												\
	outDecl											\
	varying " lowp vec4 ocolor;\n"					\
	"\n"											\
	"void main()\n"									\
	"{\n"											\
	"	" outVar " = ocolor;\n"						\
	"}\n"											\

static const char* kGlesFShaderTextGLES2 = FRAGMENT_SHADER_SRC("\n", "varying", "\n", "gl_FragColor");
static const char* kGlesFShaderTextGLES3 = FRAGMENT_SHADER_SRC("#version 300 es\n", "in", "out lowp vec4 fragColor;\n", "fragColor");
#if SUPPORT_OPENGL_CORE
static const char* kGlesFShaderTextGLCore = FRAGMENT_SHADER_SRC("#version 150\n", "in", "out lowp vec4 fragColor;\n", "fragColor");
#endif

#undef FRAGMENT_SHADER_SRC

GlTextureController::GlTextureController(UnityGfxRenderer apiType)
{
	_apiType = apiType;
}

void GlTextureController::ProcessDeviceEvent(UnityGfxDeviceEventType type, 
	IUnityInterfaces* interfaces)
{
	if (type == kUnityGfxDeviceEventInitialize)
	{
		CreateResources();
	}
	else if (type == kUnityGfxDeviceEventShutdown)
	{
		//@TODO: release resources
	}
}

int GlTextureController::BeginModifyTexture(int id, void* texHandle, int texWidth,
	int texHeight, int texDepth)
{
	return (int)_formats[texDepth];
}

void GlTextureController::EndModifyTexture(int id, void* texHandle, unsigned char* img,
	int texWidth, int texHeight, int texDepth)
{
	GLuint gltex = (GLuint)(size_t)(texHandle);
	GLenum glFormat = (GLenum)(size_t)(texDepth);
	glBindTexture(GL_TEXTURE_2D, gltex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, glFormat, GL_UNSIGNED_BYTE, img);
}

void GlTextureController::CreateResources()
{
#	if UNITY_WIN && SUPPORT_OPENGL_CORE
	if (_apiType == kUnityGfxRendererOpenGLCore)
		gl3wInit();
#	endif
	// Make sure that there are no GL error flags set before creating resources
	while (glGetError() != GL_NO_ERROR) {}

	// Create shaders
	if (_apiType == kUnityGfxRendererOpenGLES30)
	{
		m_VertexShader = CreateShader(GL_VERTEX_SHADER, kGlesVProgTextGLES2);
		m_FragmentShader = CreateShader(GL_FRAGMENT_SHADER, kGlesFShaderTextGLES2);
	}
	else if (_apiType == kUnityGfxRendererOpenGLES30)
	{
		m_VertexShader = CreateShader(GL_VERTEX_SHADER, kGlesVProgTextGLES3);
		m_FragmentShader = CreateShader(GL_FRAGMENT_SHADER, kGlesFShaderTextGLES3);
	}
#	if SUPPORT_OPENGL_CORE
	else if (_apiType == kUnityGfxRendererOpenGLCore)
	{
		m_VertexShader = CreateShader(GL_VERTEX_SHADER, kGlesVProgTextGLCore);
		m_FragmentShader = CreateShader(GL_FRAGMENT_SHADER, kGlesFShaderTextGLCore);
	}
#	endif // if SUPPORT_OPENGL_CORE


	// Link shaders into a program and find uniform locations
	m_Program = glCreateProgram();
	glBindAttribLocation(m_Program, kVertexInputPosition, "pos");
	glBindAttribLocation(m_Program, kVertexInputColor, "color");
	glAttachShader(m_Program, m_VertexShader);
	glAttachShader(m_Program, m_FragmentShader);
#	if SUPPORT_OPENGL_CORE
	if (_apiType == kUnityGfxRendererOpenGLCore)
		glBindFragDataLocation(m_Program, 0, "fragColor");
#	endif // if SUPPORT_OPENGL_CORE
	glLinkProgram(m_Program);

	GLint status = 0;
	glGetProgramiv(m_Program, GL_LINK_STATUS, &status);
	assert(status == GL_TRUE);

	m_UniformWorldMatrix = glGetUniformLocation(m_Program, "worldMatrix");
	m_UniformProjMatrix = glGetUniformLocation(m_Program, "projMatrix");

	// Create vertex buffer
	glGenBuffers(1, &m_VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 1024, NULL, GL_STREAM_DRAW);

	assert(glGetError() == GL_NO_ERROR);
}

void* GlTextureController::CreateTexture(int w, int h, int d)
{
	return NULL;
}

bool GlTextureController::DeleteTexture(void* tex)
{
	return false;
}

#endif