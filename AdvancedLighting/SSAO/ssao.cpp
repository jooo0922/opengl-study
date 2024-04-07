#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리
#include <algorithm> // std::min(), std::max() 를 사용하기 위해 포함한 라이브러리

// 이미 model.h 에 include 된 이미지 파일 로드 라이브러리의 중복 include 제거
//#define STB_IMAGE_IMPLEMENTATION
//#include "MyHeaders/stb_image.h"

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"
#include "MyHeaders/camera.h"
#include "MyHeaders/model.h"

#include <iostream>
#include <cstdlib> // srand() 및 rand() 함수 사용을 위해 포함
#include <random> // c++ 11 부터 들어온 랜덤 라이브러리 (난수 생성기, 난수 분포 조작 클래스 (std::uniform_real_distribution<> 등...) 사용을 위해 포함)

/* 콜백함수 전방선언 */

// GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언
void processInput(GLFWwindow* window);

// 씬에 큐브를 렌더링하는 함수 선언
void renderCube();

// shadow map 을 샘플링하여 깊이 버퍼를 시각화할 QuadMesh 를 렌더링하는 함수 선언
void renderQuad();


// 윈도우 창 생성 옵션
// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이

// 카메라 클래스 생성 (카메라 위치값만 매개변수로 전달함.)
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

// 가장 최근 마우스 입력 좌표값을 스크린 좌표의 중점으로 초기화
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// 첫 번째 마우스 입력 여부를 나타내는 상태값 > 첫 번째 마우스 입력에 대한 예외처리 목적
bool firstMouse = true;

// 카메라 이동속도 보정에 사용되는 deltaTime 변수 선언 및 초기화
float deltaTime = 0.0f; // 마지막에 그려진 프레임 ~ 현재 프레임 사이의 시간 간격
float lastFrame = 0.0f; // 마지막에 그려진 프레임의 ElapsedTime(경과시간)

// sample kernel 이동 벡터의 길이를 조정할 때 사용할 선형 보간 함수 구현
float ourLerp(float a, float b, float f)
{
	return a + f * (b - a);
}


int main()
{
	// GLFW 초기화
	glfwInit();


	/* GLFW 윈도우(창) 설정 구성 */

	// GLFW 에게 OpenGL 3.3 버전 사용 명시
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// GLFW 에게 core-profile 버전 사용 명시 (Core-profile vs Immediate mode 비교글 참고)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// macos 를 지칭하는 매크로 __APPLE__ 전처리기 존재 여부를 통해 
	// 현재 운영체제가 macos 일 경우, 미래 버전의 OpenGL 을 사용해서 GLFW 창을 생성하도록 함. (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE)
	// 이를 통해, macos 에서 실행되는 OpenGL 버전 호환성 문제를 해결함.
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW 윈도우 생성 (윈도우 높이, 너비, 윈도우창 제목, 풀스크린 모드/창모드, 컨텍스트 리소스를 공유할 또 다른 윈도우)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		// GLFW 윈도우 생성 실패(null 체크) 시 예외 처리
		std::cout << "Failed to create GLFW window" << std::endl;

		// 현재 남아있는 glfw 윈도우 제거, 리소스 메모리 해제, 라이브러리 초기화 이전 상태로 되돌림. 
		// glfwInit() 과 반대 역할!
		glfwTerminate();

		return -1;
	}

	// 새로운 GLFWwindow 윈도우 객체를 만들면, 해당 윈도우의 OpenGL 컨텍스트를 현재 실행중인 스레드의 현재 컨텍스트로 지정
	glfwMakeContextCurrent(window);


	/* GLFWwindow 에 콜백함수 등록 */

	// GLFWwindow 창 크기 변경(resize) 감지 시, 발생시킬 리사이징 콜백함수 등록 (콜백함수는 항상 게임루프 시작 전 등록!)
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// GLFWwindow 에 마우스 커서 입력 감지 시, 발생시킬 콜백함수 등록
	glfwSetCursorPosCallback(window, mouse_callback);

	// GLFWwindow 에 마우스 스크롤 입력 감지 시, 발생시킬 콜백함수 등록
	glfwSetScrollCallback(window, scroll_callback);


	// 마우스 커서 입력에 대한 설정을 지정
	// GLFW_CURSOR_DISABLED 로 커서 입력을 설정할 경우, 
	// 마우스 이동 시, 커서를 보이지 않게 함과 동시에, 마우스 커서 위치가 window 창 범위를 벗어나지 않도록(capture) 함
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLAD 로 런타임에 각 운영체제에 걸맞는 OpenGL 함수 포인터 초기화 및 실패 시 예외 처리.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Depth Test(깊이 테스팅) 상태를 활성화함
	glEnable(GL_DEPTH_TEST);

	// MRT 프레임버퍼에 attach 된 G-buffer 생성 시 적용할 쉐이더 객체 생성
	Shader shaderGeometryPass("MyShaders/ssao_geometry.vs", "MyShaders/ssao_geometry.fs");

	// G-buffer 로부터 샘플링된 데이터로 조명 계산을 적용할 쉐이더 객체 생성
	Shader shaderLightingPass("MyShaders/ssao.vs", "MyShaders/ssao_lighting.fs");

	// G-buffer, sample kernel, random rotation buffer 로부터 샘플링된 데이터로 SSAO 효과를 적용할 쉐이더 객체 생성
	Shader shaderSSAO("MyShaders/ssao.vs", "MyShaders/ssao.fs");

	// SSAO 가 적용된 텍스쳐 버퍼에 blur 효과를 적용할 쉐이더 객체 생성
	Shader shaderSSAOBlur("MyShaders/ssao.vs", "MyShaders/ssao_blur.fs");


	/* Assimp 를 사용하여 모델 업로드 */

	// model.h 에서 텍스쳐 업로드 전, OpenGL 좌표계에 맞게 이미지를 Y축 반전(= 수직 반전)하여 로드되도록 설정
	stbi_set_flip_vertically_on_load(true);

	// Assimp 의 기능들을 추상화한 Model 클래스를 생성하여 모델 업로드
	Model backpack("resources/models/backpack/backpack.obj");


	/* G-buffer 로 사용할 프레임버퍼(Floating point framebuffer) 생성 및 설정 */
	/* 또한, 이 프레임버퍼는 Multiple Render Target(MRT) 로 설정. */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// FBO 객체에 attach 할 텍스쳐 객체들의 참조 id 를 반환받을 변수 초기화
	unsigned int gPosition, gNormal, gAlbedo;

	// FBO 객체에 attach 할 G-buffer(position) 텍스쳐 객체 생성 및 바인딩
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// FBO 객체에 attach 할 G-buffer(normal) 텍스쳐 객체 생성 및 바인딩
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// FBO 객체에 attach 할 G-buffer(albedo, specular) 텍스쳐 객체 생성 및 바인딩
	/*
		주목할 점은, albedo 와 specular 버퍼는
		하나의 텍스쳐 버퍼를 같이 사용함.

		albedo 는 .rgb, specular 는 .a 에 저장하려는 것!
	*/
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

	// MRT 프레임버퍼 사용을 위해, OpenGL 에게 하나의 프레임버퍼가 3개의 color attachment 에 각각 렌더링할 수 있도록 명시함
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// FBO 객체에 attach 할 RBO(RenderBufferObject) 객체 생성 및 바인딩
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);

	// RBO 객체 메모리 공간 할당
	// 단일 Renderbuffer 에 depth 값만 저장하는 데이터 포맷 지정 -> GL_DEPTH_COMPONENT 
	// 또한, 텍스쳐 객체와 마찬가지로 스크린 해상도와 Renderbuffer 해상도를 일치시킴 -> 그래야 SCR_WIDTH * SCR_HEIGHT 개수 만큼의 데이터 저장 공간 확보 가능!
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);

	// FBO 객체에 생성한 RBO 객체 attach (자세한 매개변수 설명은 LearnOpenGL 본문 참고!)
	// off-screen framebuffer 에 렌더링 시, RBO 객체에는 depth 값만 저장할 것이므로, GL_DEPTH_STENCIL_ATTACHMENT 를 적용함!
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	// 현재 GL_FRAMEBUFFER 상태에 바인딩된 FBO 객체 설정 완료 여부 검사 (설정 완료 조건은 LearnOpenGL 본문 참고)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// FBO 객체가 제대로 설정되지 않았을 시 에러 메시지 출력
		std::cout << "Framebuffer is not complete!" << std::endl;
	}

	// 생성한 FBO 객체 설정 완료 후, 다시 default framebuffer 바인딩하여 원상복구 (참고로, default framebuffer 의 참조 id 가 0임!)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/* SSAO 효과를 적용할 프레임버퍼(Floating point framebuffer) 생성 및 설정 */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int ssaoFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	// FBO 객체에 attach 할 텍스쳐 버퍼 객체의 참조 id 를 반환받을 변수 초기화
	unsigned int ssaoColorBuffer;

	// FBO 객체에 attach 할 SSAO 적용 결과를 렌더링할 텍스쳐 버퍼 객체 생성 및 바인딩 (GL_RED 관련 하단 필기 참고)
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

	// 현재 GL_FRAMEBUFFER 상태에 바인딩된 FBO 객체 설정 완료 여부 검사
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// FBO 객체가 제대로 설정되지 않았을 시 에러 메시지 출력
		std::cout << "SSAO Framebuffer is not complete!" << std::endl;
	}


	/* blur 효과를 적용할 프레임버퍼(Floating point framebuffer) 생성 및 설정 */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int ssaoBlurFBO;
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

	// FBO 객체에 attach 할 텍스쳐 버퍼 객체의 참조 id 를 반환받을 변수 초기화
	unsigned int ssaoColorBufferBlur;

	// FBO 객체에 attach 할 SSAO 적용 결과를 렌더링할 텍스쳐 버퍼 객체 생성 및 바인딩 (GL_RED 관련 하단 필기 참고)
	glGenTextures(1, &ssaoColorBufferBlur);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

	// 현재 GL_FRAMEBUFFER 상태에 바인딩된 FBO 객체 설정 완료 여부 검사
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// FBO 객체가 제대로 설정되지 않았을 시 에러 메시지 출력
		std::cout << "SSAO Blur Framebuffer is not complete!" << std::endl;
	}


	/* 반구 영역 내의 랜덤한 sample kernel 계산 (SSAO sample kernel 관련 하단 필기 참고) */

	// [0.0, 1.0] 사이의 실수형 난수 생성 시 따를 균등 분포 생성
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);

	// c++ 표준 라이브러라에서 제공하는 기본 난수 생성 엔진 사용
	std::default_random_engine generator;

	// 생성된 랜덤한 sample kernel 들을 보관할 동적 배열 선언
	std::vector<glm::vec3> ssaoKernel;

	// 반복문을 순회하며 64개의 랜덤한 sample kernel 계산
	for (unsigned int i = 0; i < 64; i++)
	{
		// 각 프래그먼트에 적용할 탄젠트 공간 기준 양의 z 축 방향을 향하도록 임의의 sample kernel 이동 벡터 계산
		// 왜 양의 z 축 방향을 향해야 할까? 각 프래그먼트의 노멀 벡터를 중심으로 한 '반구 영역 내의' 이동 벡터를 계산하고 싶기 때문에!
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0, // 이동 벡터의 x 값은 [-1.0, 1.0] 사이의 범위로 계산
			randomFloats(generator) * 2.0 - 1.0, // 이동 벡터의 y 값은 [-1.0, 1.0] 사이의 범위로 계산
			randomFloats(generator) // 이동 벡터의 z 값은 [0.0, 1.0] 사이의 범위로 계산 -> 이동 벡터가 탄젠트 공간 기준 양의 z 축 방향을 향해야 하니까!
		);

		// sample kernel 이동 벡터 정규화 -> 길이를 모두 1로 맞춤
		sample = glm::normalize(sample);

		// 길이가 1로 맞춰진 상태에서 [0.0, 1.0] 사이의 랜덤한 스칼라 값을 곱함 -> 이동 벡터의 길이를 [0.0, 1.0] 사이의 랜덤한 길이로 변경
		sample *= randomFloats(generator);

		// 64개의 각 sample kernel 이동 벡터의 길이를 선형 보간 함수로 조정할 때 사용할 alpha 값 계산 
		// (sample kernel 길이 조정 관련 하단 필기 참고)
		float scale = (float)i / 64.0;

		// alpha 값을 quadratic 하게 계산함으로써, scale 값을 선형 보간 -> 가속 보간으로 변형
		scale = ourLerp(0.1f, 1.0f, scale * scale);

		// 이동 벡터의 길이를 가속 보간된 scale 값으로 조정 -> 64개의 이동 벡터들이 전체적으로 0.1 에 더 가까운 scale 길이값을 갖도록 조정
		sample *= scale;

		// 길이가 조정된 sample kernel 이동 벡터를 std::vector 동적 배열에 추가
		ssaoKernel.push_back(sample);
	}


	/* 반구 영역을 전체적으로 회전시킬 Random rotation vector 계산 (Random rotation 관련 하단 필기 참고) */

	// random rotation vector 들을 보관할 동적 배열 선언
	std::vector<glm::vec3> ssaoNoise;

	// 4*4 크기의 텍스쳐 버퍼에 각 random rotation vector 들을 저장하기 위해 총 16개의 rotation vector 계산
	for (unsigned int i = 0; i < 16; i++)
	{
		/*
			반구 영역 내의 각 sample kernel 이동 벡터들이
			positive z 축을 향하도록 계산되어 있기 때문에,

			반구 영역 내의 전체적인 이동 벡터들을 회전시킬
			rotation vector 는 xy 평면에 대한 회전, 즉, 
			z축 회전을 정의해야 하므로, z component 는 0 으로 고정
		*/
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f
		);

		// 계산된 random rotation vector 를 동적 배열에 추가
		ssaoNoise.push_back(noise);
	}


	/* 생성된 Random rotation vector 를 저장할 4*4 크기의 텍스쳐 버퍼 생성 (텍스쳐 버퍼에 저장하는 이유 하단 필기 참고) */

	// 텍스쳐 버퍼 객체의 참조 id 를 반환받을 변수 초기화
	unsigned int noiseTexture;

	// 텍스쳐 버퍼 객체 생성 및 바인딩
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	// 텍스쳐 버퍼 크기를 4*4 로 지정
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);

	// 4*4 텍스쳐 버퍼를 전체 screen 영역에서 반복해서 샘플링하도록, wrap mode 를 GL_REPEAT 으로 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	/* 광원 정보 초기화 */

	// 씬에 배치할 광원 위치값 초기화
	glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);

	// 씬에 배치할 광원 색상값 초기화
	glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);


	/*
		각 pass 단계마다 적용할 쉐이더에 선언된
		각 G-buffer 및 텍스쳐 버퍼들의 uniform sampler 변수들에
		texture unit 위치값 전송
	*/
	shaderLightingPass.use();
	shaderLightingPass.setInt("gPosition", 0);
	shaderLightingPass.setInt("gNormal", 1);
	shaderLightingPass.setInt("gAlbedo", 2);
	shaderLightingPass.setInt("ssao", 3);
	shaderSSAO.use();
	shaderSSAO.setInt("gPosition", 0);
	shaderSSAO.setInt("gNormal", 1);
	shaderSSAO.setInt("texNoise", 2);
	shaderSSAOBlur.use();
	shaderSSAOBlur.setInt("ssaoInput", 0);


	// while 문으로 렌더링 루프 구현
	while (!glfwWindowShouldClose(window))
	{
		/* 카메라 이동속도 보정을 위한 deltaTime 계산 */

		// 현재 프레임 경과시간
		float currentFrame = static_cast<float>(glfwGetTime());

		// 현재 프레임 경과시간 - 마지막 프레임 경과시간 = 두 프레임 사이의 시간 간격
		deltaTime = currentFrame - lastFrame;

		// 마지막 프레임 경과시간을 현재 프레임 경과시간으로 업데이트!
		lastFrame = currentFrame;


		// 윈도우 창 및 키 입력 감지 밎 이벤트 처리
		processInput(window);

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// 색상 버퍼 및 깊이 버퍼 초기화 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		/* 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함. */


		/* Geometry Pass (씬의 geometry data 를 G-buffer 에 렌더링하기) */

		// MRT framebuffer 바인딩
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

		// 현재 바인딩된 framebuffer 의 색상 및 깊이 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 카메라의 zoom 값으로부터 투영 행렬 계산
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 쉐이더에 전송할 모델 행렬을 단위 행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// 변환행렬을 전송할 쉐이더 프로그램 바인딩
		shaderGeometryPass.use();

		// 계산된 투영행렬을 쉐이더 프로그램에 전송
		shaderGeometryPass.setMat4("projection", projection);

		// 계산된 뷰 행렬을 쉐이더 프로그램에 전송
		shaderGeometryPass.setMat4("view", view);

		// room cube 에 적용할 모델 행렬 계산
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 7.0f, 0.0f));
		model = glm::scale(model, glm::vec3(7.5f, 7.5f, 7.5f));

		// 계산된 모델 행렬을 쉐이더 프로그램에 전송
		shaderGeometryPass.setMat4("model", model);

		// room cube 는 BACK_FACE 를 렌더링해줘야 하므로, 바깥으로 향하는 노멀벡터를 뒤집도록 플래그 설정
		shaderGeometryPass.setInt("invertedNormals", 1);

		// room cube 그리기 명령 호출
		renderCube();

		// 다른 오브젝트들은 정상적으로 FRONT_FACE 를 렌더링하기 위해, 노멀벡터 뒤집는 플래그를 원복
		shaderGeometryPass.setInt("invertedNormals", 0);

		// backpack 에 적용할 모델 행렬 계산
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		model = glm::scale(model, glm::vec3(1.0f));

		// 계산된 모델 행렬을 쉐이더 프로그램에 전송
		shaderGeometryPass.setMat4("model", model);

		// backpack 그리기 명령 호출
		backpack.Draw(shaderGeometryPass);

		// default framebuffer 로 바인딩 복구
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		/* 
			SSAO Pass 
			
			G-buffer, sample kernel, random rotation buffer 로부터 샘플링된 데이터로 
			SSAO 효과를 적용하여 계산한 occlusion factor 들을 ssaoFBO 프레임버퍼에 attach 된 
			ssaoColorBuffer 텍스쳐 버퍼 객체에 렌더링 
		*/

		// SSAO 효과를 적용하여 렌더링할 framebuffer 바인딩
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		// 현재 바인딩된 framebuffer 의 색상 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT);

		// SSAO 효과를 적용하여 occlusion factor 계산을 수행하는 쉐이더 프로그램 바인딩
		shaderSSAO.use();

		// 미리 계산해 둔 64개의 sample kernel 이동 벡터를 쉐이더 프로그램에 전송
		for (unsigned int i = 0; i < 64; i++)
		{
			shaderSSAO.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
		}

		// view space 기준 sample points 위치값을 NDC 좌표계로 변환하는 과정에서 사용할 투영 행렬을 쉐이더 프로그램에 전송
		shaderSSAO.setMat4("projection", projection);

		// 미리 생성해 둔 2개의 G-buffer 들과 random rotation vector 텍스쳐 버퍼를 각 texture unit 에 바인딩
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);

		// pixel 단위 SSAO occlusion factor 계산 결과를 렌더링할 QuadMesh 그리기
		renderQuad();

		// default framebuffer 로 바인딩 복구
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		/*
			SSAO Blur Pass

			반복적인 random rotation vector 텍스쳐 버퍼 tiling 으로 인해 발생하는
			noise pattern 을 blur 처리하여 fix 하는 단계
		*/

		// SSAO Blur 효과를 적용하여 렌더링할 framebuffer 바인딩
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

		// 현재 바인딩된 framebuffer 의 색상 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT);

		// SSAO Blur 효과를 적용할 쉐이더 프로그램 바인딩
		shaderSSAOBlur.use();

		// SSAO occlusion factor 계산 결과가 렌더링된 텍스쳐 버퍼를 texture unit 에 바인딩
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);

		// pixel 단위 Blur 효과를 렌더링할 QuadMesh 그리기
		renderQuad();

		// default framebuffer 로 바인딩 복구
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		/* Lighting Pass (G-buffer 및 SSAO occlusion factor 가 적용된 텍스쳐 버퍼에서 pixel 단위로 데이터를 샘플링하여 조명 연산하여 QuadMesh 에 렌더링) */

		// 현재 바인딩된 framebuffer 의 색상 및 깊이 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 조명 연산을 수행하는 쉐이더 프로그램 바인딩
		shaderLightingPass.use();

		// view space 기준 광원 위치값 계산하여 쉐이더 프로그램에 전송
		glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
		shaderLightingPass.setVec3("light.Position", lightPosView);

		// 광원 색상값을 쉐이더 프로그램에 전송
		shaderLightingPass.setVec3("light.Color", lightColor);

		// attenuation(감쇄) 계산에 필요한 데이터들 추가 전송
		const float linear = 0.09f;
		const float quadratic = 0.032f;
		shaderLightingPass.setFloat("light.Linear", linear);
		shaderLightingPass.setFloat("light.Quadratic", quadratic);

		// 미리 생성해 둔 3개의 G-buffer 들을 각 texture unit 에 바인딩
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedo);

		// Blur 처리까지 적용된 SSAO occlusion factor 텍스쳐 버퍼를 texture unit 에 바인딩
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

		// pixel 단위 조명 연산 결과를 렌더링할 QuadMesh 그리기
		renderQuad();


		// Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwSwapBuffers(window);

		// 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
		glfwPollEvents();
	}


	// while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제
	glfwTerminate();

	return 0;
}


/* 전방선언된 콜백함수 정의 */

/* 씬에 큐브를 렌더링하는 함수 구현 */

// Cube VBO, VAO 객체(object) 참조 id 를 저장할 변수 전역 선언 (why? 다른 함수들에서도 참조)
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderCube()
{
	/*
		VAO 참조 ID 가 아직 할당되지 않았을 경우,
		큐브의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고)
	*/
	if (cubeVAO == 0)
	{
		// 큐브의 정점 데이터 정적 배열 초기화
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};

		// VAO(Vertex Array Object) 객체 생성
		glGenVertexArrays(1, &cubeVAO);

		// VBO(Vertex Buffer Object) 객체 생성
		glGenBuffers(1, &cubeVBO);

		// VAO 객체 먼저 컨텍스트에 바인딩(연결)함. 
		// -> 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.
		glBindVertexArray(cubeVAO);

		// VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

		// 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. 
		// -> 그 중에서 0번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(0);

		// 정점 위치 데이터(0번 location 입력변수 in vec3 aPos 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

		// 1번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(1);

		// 정점 노멀 데이터(1번 location 입력변수 in vec3 aNormal 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

		// 2번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(2);

		// 정점 UV 데이터(2번 location 입력변수 in vec2 aTexCoords 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

		// VBO 객체 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// 마찬가지로, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
		glBindVertexArray(0);
	}

	/* 큐브 그리기 */

	// 큐브에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
	glBindVertexArray(cubeVAO);

	// 큐브 그리기 명령
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// 그리기 명령 종료 후, VAO 객체 바인딩 해제
	glBindVertexArray(0);
}


/* 버퍼를 시각화할 QuadMesh 를 렌더링하는 함수 구현 */

// QuadMesh VBO, VAO 객체(object) 참조 id 를 저장할 변수 전역 선언 (why? 다른 함수들에서도 참조)
unsigned int quadVAO = 0;
unsigned int quadVBO = 0;

void renderQuad()
{
	/*
		VAO 참조 ID 가 아직 할당되지 않았을 경우,
		QuadMesh 의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고)
	*/
	if (quadVAO == 0)
	{
		// QuadMesh 의 정점 데이터 정적 배열 초기화
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		// VAO(Vertex Array Object) 객체 생성
		glGenVertexArrays(1, &quadVAO);

		// VBO(Vertex Buffer Object) 객체 생성
		glGenBuffers(1, &quadVBO);

		// VAO 객체 먼저 컨텍스트에 바인딩(연결)함. 
		// -> 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.
		glBindVertexArray(quadVAO);

		// VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

		// 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		// 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. 
		// -> 그 중에서 0번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(0);

		// 정점 위치 데이터(0번 location 입력변수 in vec3 aPos 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		// 1번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(1);

		// 정점 UV 데이터(1번 location 입력변수 in vec2 aTexCoords 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

		// VBO 객체 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// 마찬가지로, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
		glBindVertexArray(0);
	}

	/* QuadMesh 그리기 */

	// QuadMesh 에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
	glBindVertexArray(quadVAO);

	// QuadMesh 그리기 명령
	// (Quad 를 그리려면 2개의 삼각형(== 6개의 정점)이 정의되어야 하지만, 
	// 위에서 4개의 정점 데이터만 정의했으므로, 정점을 공유하여 삼각형을 조립하는 GL_TRIANGLE_STRIP 모드로 렌더링한다.)
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// 그리기 명령 종료 후, VAO 객체 바인딩 해제
	glBindVertexArray(0);
}


// GLFWwindow 윈도우 창 리사이징 감지 시, 호출할 콜백 함수 정의
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height); // GLFWwindow 상에 렌더링될 뷰포트 영역을 정의. (뷰포트 영역의 좌상단 좌표, 뷰포트 영역의 너비와 높이)
}

// GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수 정의
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	// 콜백함수의 매개변수로 전달받는 마우스 좌표값의 타입을 double > float 으로 형변환
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		// 맨 처음 전달받은 마우스 좌표값은 초기에 설정된 lastX, Y 와 offset 차이가 심할 것임.
		// 이 offset 으로 yaw, pitch 변화를 계산하면 회전이 급격하게 튀다보니,
		// 맨 처음 전달받은 마우스 좌표값으로는 offset 을 계산하지 않고, lastX, Y 값을 업데이트 하는 데에만 사용함.
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// 마지막 프레임의 마우스 좌표값에서 현재 프레임의 마우스 좌표값까지 이동한 offset 계산
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // y축 좌표는 스크린좌표계와 3D 좌표계(오른손 좌표계)와 방향이 반대이므로, -(ypos - lastY) 와 같이 뒤집어준 것!

	// 마지막 프레임의 마우스 좌표값 갱신
	lastX = xpos;
	lastY = ypos;

	// 마우스 이동량(offset)에 따른 카메라 오일러 각 재계산 및 카메라 로컬 축 벡터 업데이트
	camera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수 정의
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// 마우스 수직방향 스크롤 이동량(yoffset)에 따른 카메라 zoom 값 재계산
	camera.ProcessMouseScroll(yoffset);
}

// GLFWwindow 윈도우 입력 및 키 입력 감지 후 이벤트 처리 함수 (렌더링 루프에서 반복 감지)
void processInput(GLFWwindow* window)
{
	// 현재 GLFWwindow 에 대하여(활성화 시,) 특정 키(esc 키)가 입력되었는지 여부를 감지
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true); // GLFWwindow 의 WindowShouldClose 플래그(상태값)을 true 로 설정 -> main() 함수의 while 조건문에서 렌더링 루프 탈출 > 렌더링 종료!
	}

	// 카메라 이동속도 보정 (기본 속도 2.5 가 어느 컴퓨터에서든 유지될 수 있도록 deltaTime 값으로 속도 보정)
	float cameraSpeed = static_cast<float>(2.5 * deltaTime);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime); // 키 입력에 따른 카메라 이동 처리 (GLFW 키 입력 메서드에 독립적인 enum 사용)
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime); // 키 입력에 따른 카메라 이동 처리 (GLFW 키 입력 메서드에 독립적인 enum 사용)
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime); // 키 입력에 따른 카메라 이동 처리 (GLFW 키 입력 메서드에 독립적인 enum 사용)
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime); // 키 입력에 따른 카메라 이동 처리 (GLFW 키 입력 메서드에 독립적인 enum 사용)
	}
}


/*
	VAO 는 왜 만드는걸까?

	VBO 객체를 생성 및 바인딩 후,
	해당 버퍼에 정점 데이터를 쓰고,
	버퍼에 쓰여진 데이터를 버텍스 쉐이더의 몇번 location 의 변수에서 사용할 지,
	해당 데이터를 몇 묶음으로 해석할 지 등의 해석 방식을 정의하고,
	해당 버퍼에 쓰여진 데이터를 사용하는 location 의 변수를 활성화하는 등의 작업은 이해가 가지?

	모두 GPU 메모리 상에 저장된 정점 버퍼의 데이터를
	버텍스 쉐이더가 어떻게 가져다 쓸 지 정의하기 위한 과정이지.

	그런데, 만약 서로 다른 오브젝트가 100개 존재하고,
	각 오브젝트에 5개의 vertex attribute 를 사용한다면?
	이런 식으로 VBO 를 구성하고 데이터 해석 방식을 설정하는 작업을
	그리기 명령이 발생할 때마다 500번씩 매번 해야된다는 소리...

	그런데, VAO 객체를 사용하면, 거기에다가
	VAO 안에 VBO 객체와 데이터 해석 방식, 해당 location 변수 활성화 여부 등의
	설정 상태를 모두 저장해두고 그리기 명령을 호출할 때마다
	필요한 VAO 객체를 교체하거나 꺼내쓸 수 있다.

	즉, 저런 번거로운 VBO 객체 생성 및 설정 작업을 반복하지 않아도 된다는 뜻!
*/

/*
	Floating point framebuffer


	HDR 을 구현하려면, [0, 1] 범위를 넘어서는 색상값들이
	프레임버퍼에 attach 된 텍스쳐 객체에 저장될 때,
	[0, 1] 사이로 clamping 되지 않고,

	원래의 색상값이 그대로 저장될 수 있어야 함.

	그러나, 일반적인 프레임버퍼에서 color 를 저장할 때,
	내부 포맷으로 사용하는 GL_RGB 같은 포맷은
	fixed point(고정 소수점) 포맷이기 때문에,

	OpenGL 에서 프레임버퍼에 색상값을 저장하기 전에
	자동으로 [0, 1] 사이의 값으로 clamping 해버리는 문제가 있음.


	이를 해결하기 위해,
	GL_RGB16F, GL_RGBA16F, GL_RGB32F, GL_RGBA32F 같은
	floating point(부동 소수점) 포맷으로
	프레임버퍼의 내부 색상 포맷을 변경하면,

	[0, 1] 범위를 벗어나는 값들에 대해서도
	부동 소수점 형태로 저장할 수 있도록 해줌!


	이때, 일반적인 프레임버퍼의 기본 색상 포맷인
	GL_RGB 같은 경우 하나의 컴포넌트 당 8 bits 메모리를 사용하는데,
	GL_RGB32F, GL_RGBA32F 같은 포맷은 하나의 컴포넌트 당 32 bits 의 메모리를 사용하기 때문에,
	우리는 이 정도로 많은 메모리를 필요로 하지는 않음.

	따라서, GL_RGB16F, GL_RGBA16F 같이
	한 컴포넌트 당 16 bits 정도의 메모리를 예약해서 사용하는
	적당한 크기의 색상 포맷으로 사용하는 게 좋겠지!
*/

/*
	Multiple Render Target(MRT) 프레임버퍼


	Bloom 을 구현하려면,
	현재 씬을 렌더링하는 Render Pass 로부터
	2개의 이미지를 추출해와야 함.

	첫 번째 이미지는 일반적인 조명 계산에 의한 씬의 이미지이고,
	두 번째 이미지는 밝기값(gray scale)이 일정 threshold 를 넘는 색상값만 추출한 씬의 이미지임.
	(자세한 건 LearnOpenGL 본문 일러스트 참고)


	현재 씬으로부터 2개의 이미지를 렌더링하려면,
	2개의 프레임버퍼를 각각 생성 및 바인딩해서
	2번의 Render Pass 로부터 얻어오는 방법이 가장 먼저 떠오르겠지만,

	1개의 프레임버퍼만 생성 및 바인딩하고,
	해당 프레임버퍼에 2개의 color attachment(즉, 텍스쳐 객체)를 생성 및 바인딩해서,
	1번의 Render Pass 로부터 2개의 이미지를 렌더링할 수 있는 방법이 있음!

	이러한 기법을 'Multiple Render Target(MRT)' 라고 함.


	이때, 주의해야 할 점은,

	OpenGL 은 기본적으로 프레임버퍼에
	여러 개의 color attachment 가 바인딩되어 있을 시,
	맨 첫 번째 color attachment 에만 렌더링 결과를 저장하고,
	나머지 color attachment 들은 무시해버림.

	이러한 현상을 방지하기 위해, glDrawBuffers() 함수를 사용하여
	'이 프레임버퍼에서는 여러 개의 color attachment 를 사용할 것'임을
	명시할 수 있음!
*/

/*
	Deferred rendering 과 G-buffer


	G-buffer 의 개념을 이해하기 위해서는,
	Deferred rendering vs Forward rendering 의 차이를 이해해야 함.

	일반적인 Object 단위로 모든 프래그먼트들을 순회하면서
	조명 연산을 수행하는 방식이 Forward rendering 이라면,

	Deferred rendering 은 실제 화면에 렌더링될 pixel 에 대해서만
	조명 연산을 수행하는 방식이라고 보면 됨.

	즉, Deferred rendering 에서는 조명 연산이 획기적으로 줄어드는
	성능 상의 이점이 존재함.


	그렇다면, 화면에 최종적으로 렌더링될 pixel 의 조명값을
	어떻게 계산할 수 있을까?

	이때 필요한 개념이 바로 G-buffer 임!


	G-buffer 는 화면에 최종적으로 렌더링되는 프래그먼트에
	대한 geometry 정보들(예를 들어, position, normal, albedo, specular 등)을
	MRT 프레임버퍼에 attach 된 텍스쳐 버퍼에 기록해 둔 것을 의미함!

	이 geometry 정보들이 담긴 텍스쳐 버퍼들을
	Lighting Pass 에서 샘플링하여 조명 연산을 수행하면,
	스크린의 각 pixel 에 최종적으로 찍힐 프래그먼트들의 조명 연산만 수행할 수 있음!
*/

/*
	SSAO 버퍼에 GL_RED 포맷을 사용한 이유

	
	SSAO 효과가 적용될 버퍼에는
	환경광 차폐(ambient occlusion)가 발생한 영역의 그림자만 표시하면 되므로,
	grayscale value 만 버퍼에 저장해주면 됨.

	따라서, 텍스쳐 버퍼의 내부 포맷을
	1개의 컴포넌트(= GL_RED)만 사용하도록 설정함.
*/

/*
	SSAO sample kernel


	LearnOpenGL 의 본문을 보면, 
	SSAO 효과를 구현하기 위해 가장 중요한 개념으로
	'반구 영역의 sample kernel' 을 설명하고 있음.

	'반구 영역의 sample kernel' 이란 간단하게 설명하면,
	각 프래그먼트의 노멀벡터를 중심으로 한 반구(hemisphere) 영역 내에
	위치하는 랜덤하게 분포하는 sample points 들을 의미함.

	이게 왜 필요하냐면, SSAO 는 결국 'ambient occlusion', 즉,
	'환경광 차폐' 정도를 표현하는 효과인데,

	해당 프래그먼트가 다른 geometry 들에 의해 더 많이 가려질수록,
	그 프래그먼트에 들어오는 ambient light 가 더 많이 차폐된다고 가정하고,
	그 프래그먼트의 occlusion factor (가려짐 지수)를 더 높게 계산하여,
	ambient light 의 가중치로 사용한다고 보면 됨.

	이때, '다른 geometry 에 의해 가려짐'을 판별하는 방법이
	해당 프래그먼트의 노멀벡터를 기준으로 한 반구 영역 내의 임의의 sample points 들이 
	
	view space 기준으로 다른 geometry 들의 프래그먼트보다 깊이값이 더 멀리 있다면,
	또 그러한 sample points 들이 많으면 많을수록, 해당 프래그먼트를 차폐시키는
	주변 장애물들이 더 많다고 판별하는 것임.

	이러한 이유로 SSAO 를 구현하려면,
	각 프래그먼트를 기준으로 한 반구 영역 내의
	수십 개의 랜덤한 sample points 들이 필요한 것임!


	그러나, 각 프래그먼트들 주변의 sample points 들을
	프래그먼트마다 일일이 구하는 것을 비현실적임.

	그 대신, 각 프래그먼트의 view space 위치값에서 '더해줄'
	'offset(이동 벡터)' 를 미리 계산해두면, 실제 sample points 들의
	위치값을 일일이 계산하지 않아도 되겠지!


	이때, 어떤 프래그먼트에서도 더해줄 수 있도록,
	각 프래그먼트의 위치값을 원점으로 가정하는 tangent space(탄젠트 공간)을
	기준으로 'offset(이동 벡터)'를 계산해놓는 게 좋겠지!
*/

/*
	가속 보간 함수로 sample kernel 길이 조정


	sample kernel 이동 벡터를 계산한 뒤,

	for 문을 돌 때마다 0.0 -> 1.0 으로 점진적으로 증가하는
	scale 값을 계산한 다음, 이를 quadratic 하게 거듭제곱 처리하고,

	선형 보간 함수의 alpha 값으로 넘겨주면,
	선형 보간의 min 값에 가까운 결과값들을 더 많이 반환하는데,
	이를 '가속 보간(accelerating interpolation)' 이라고 부름.


	위 코드에서는 이렇게 가속 보간으로 얻은 scale 값을
	스칼라곱하여 각 sample kernel 이동 벡터의 길이를 조정하고 있음.

	이렇게 함으로써, 
	대부분의 이동 벡터의 길이가 전체적으로 짧아지게 되고,
	
	각 프래그먼트의 노멀벡터 중심의 반구 영역 내에서도
	해당 프래그먼트의 위치에 더 조밀하게 모이는 sample points 들을
	계산할 수 있게 됨.


	즉, 반구 영역 내의 sample points 들을
	프래그먼트에 더 가깝게 오밀조밀하게 모이도록 계산함으로써,
	해당 프래그먼트를 차폐하는 주변 geometry 를 더 정확하게
	감지할 수 있도록 한 것임!
*/

/*
	Random rotation vector 란?


	SSAO 에서 사용하는 반구 영역 내의 sample kernel 들은
	그 수가 많으면 많을수록 occlusion factor 를 더 정확하게 계산할 수 있음.

	그렇지만, sample point 들을 늘리면 늘릴수록
	그만큼의 성능 저하가 발생하겠지?


	그래서 sample point 들의 수를 늘리는 대신,
	각 프래그먼트마다 sample kernel 의 방향을 랜덤하게 회전시켜서
	각 프래그먼트마다 마치 '다른 방향의 sample kernel 이동 벡터를 사용하는 것처럼'
	모의할 수 있음.

	즉, 각 프래그먼트마다
	sample kernel 이동 벡터가 포함된 반구 영역을
	전체적으로 회전시킬 Random rotation vector 가 필요한 것임.


	이 Random rotation vector 를 사용해서 
	구체적으로 어떻게 반구 영역을 회전시키냐면, 

	SSAO 효과를 적용할 쉐이더(shaderSSAO) 내에서
	tangent space 로 계산된 sample kernel 이동 벡터를
	TBN 행렬을 곱해서 view space 로 변환하는데,

	이때 사용하는 TBN 행렬의 tangent 기저 축을 계산할 때,
	위에서 계산했던 Random rotation vector 를 기반으로,
	G-buffer 에서 샘플링한 노멀벡터에 대해 re-orthogonalize(재직교화)시켜서 계산함.

	
	이렇게 하면, TBN 행렬에 의해 정의되는
	tangent, normal, bitangent 의 세 기저 축 자체가
	Random rotation vector 를 기반으로 정의되는 것이나 다름없기 때문에,

	tangent space 의 세 기저축을 회전시키는 셈이 되는 것이지!


	이렇게 회전된 tangent space 기저 축을 열 벡터로 꽂아서 정의된 TBN 행렬을 사용하여 
	tangent space -> view space 로 좌표계를 변환하는 단계에서
	반구 영역 내의 전체 sample kernel 이동 벡터들이 랜덤한 회전이 적용되는 것임!
*/

/*
	Random rotation vector 를 텍스쳐 버퍼로 저장하는 이유


	각 프래그먼트마다 Random rotation vector 를 일일이 계산하는 게
	물론 가장 이상적이겠지만, 그만큼 메모리를 많이 잡아먹겠지.

	이러한 문제점을 해결하기 위해,
	4*4 정도의 아주 작은 크기의 텍스쳐 버퍼를 만들고,
	거기에 16개(= 4*4)의 Random rotation vector 를 저장한 다음,

	전체 screen 영역(QuadMesh)에서 4*4 크기의 텍스쳐 버퍼를
	GL_REPEAT 모드로 샘플링함으로써, 

	한 번 생성해 둔 16개의 Random rotation vector 를
	전체 QuadMesh 의 프래그먼트들을 돌면서 반복적으로 재사용하려는 것임! 
*/