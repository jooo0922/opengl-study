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
	Shader shaderGeometryPass("MyShaders/g_buffer.vs", "MyShaders/g_buffer.fs");

	// G-buffer 로부터 샘플링된 데이터로 조명 계산을 적용할 쉐이더 객체 생성
	Shader shaderLightPass("MyShaders/deferred_shading.vs", "MyShaders/deferred_shading.fs");

	// 광원 큐브에 적용할 쉐이더 객체 생성
	Shader shaderLightBox("MyShaders/deferred_light_box.vs", "MyShaders/deferred_light_box.fs");


	/* Assimp 를 사용하여 모델 업로드 */

	// model.h 에서 텍스쳐 업로드 전, OpenGL 좌표계에 맞게 이미지를 Y축 반전(= 수직 반전)하여 로드되도록 설정
	stbi_set_flip_vertically_on_load(true);

	// Assimp 의 기능들을 추상화한 Model 클래스를 생성하여 모델 업로드
	Model backpack("resources/models/backpack/backpack.obj");

	// 씬에 렌더링할 모델들의 위치값을 저장해 둘 std::vector 동적 배열
	std::vector<glm::vec3> objectPositions;
	objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
	objectPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
	objectPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
	objectPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
	objectPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
	objectPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
	objectPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
	objectPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
	objectPositions.push_back(glm::vec3(3.0, -0.5, 3.0));


	/* G-buffer 로 사용할 프레임버퍼(Floating point framebuffer) 생성 및 설정 */
	/* 또한, 이 프레임버퍼는 Multiple Render Target(MRT) 로 설정. */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// FBO 객체에 attach 할 텍스쳐 객체들의 참조 id 를 반환받을 변수 초기화
	unsigned int gPosition, gNormal, gAlbedoSpec;

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
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

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


	/*
		lighting pass(조명 계산 단계)에 적용할 쉐이더에 선언된 
		각 G-buffer 들의 uniform sampler 변수들에
		texture unit 위치값 전송

		참고로, Albedo 와 Specular 는 1개의 텍스쳐 버퍼에 한꺼번에 담아서 전달함!
	*/
	shaderLightPass.use();
	shaderLightPass.setInt("gPosition", 0);
	shaderLightPass.setInt("gNormal", 1);
	shaderLightPass.setInt("gAlbedoSpec", 2);


	/* 광원 정보 초기화 */

	// 씬에 배치할 광원의 전체 갯수 초기화
	const unsigned int NR_LIGHTS = 32;

	// 씬에 배치할 4개 광원 위치값을 저장할 std::vector 동적 배열 선언
	std::vector<glm::vec3> lightPositions;

	// 씬에 배치할 4개 광원 색상값을 저장할 std::vector 동적 배열 선언
	std::vector<glm::vec3> lightColors;

	// 난수 생성 시 사용할 고정 seed 값 설정
	// https://github.com/jooo0922/cpp-study/blob/76127bd7d126247e34db789883091eb39acb0f73/TBCppStudy/Chapter5_09/Chapter5_09.cpp 참고
	std::srand(13);

	// 광원 전체 갯수만큼 반복문을 순회하여 랜덤한 광원 색상 및 위치값 계산
	for (unsigned int i = 0; i < NR_LIGHTS; i++)
	{
		// rand() 함수를 사용하여 일정 범위 내의 랜덤한 위치값 계산
		/*
			참고로, rand() 는 [0, RAND_MAX] 사이의 난수를 반환함.

			RAND_MAX 는 rand() 가 반환할 수 있는 최대 정수값을
			매크로 전처리기로 선언해 둔 것이며, 컴파일 및 빌드 환경에 따라 다르지만,
			일반적으로 최소 32767 이상으로 컴파일 됨.
		*/
		float xPos = static_cast<float>(((std::rand() % 100) / 100.0) * 6.0 - 3.0); // [-3, 3] 사이의 x값 계산
		float yPos = static_cast<float>(((std::rand() % 100) / 100.0) * 6.0 - 4.0); // [-4, 2] 사이의 y값 계산
		float zPos = static_cast<float>(((std::rand() % 100) / 100.0) * 6.0 - 3.0); // [-3, 3] 사이의 z값 계산

		// 랜덤한 위치값을 동적 배열에 추가
		lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

		// rand() 함수를 사용하여 일정 범위 내의 랜덤한 색상값 계산
		float rColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5); // [0.5, 1.0] 사이의 r값 계산
		float gColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5); // [0.5, 1.0] 사이의 g값 계산
		float bColor = static_cast<float>(((std::rand() % 100) / 200.0f) + 0.5); // [0.5, 1.0] 사이의 b값 계산
	
		// 랜덤한 색상값을 동적 배열에 추가
		lightColors.push_back(glm::vec3(rColor, gColor, bColor));
	}
	

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

		// std::vector 동적 배열 크기만큼 반복문을 순회하며 backpack 모델 렌더링
		for (unsigned int i = 0; i < objectPositions.size(); i++)
		{
			// 각 backpack 모델에 적용할 모델행렬 계산
			model = glm::mat4(1.0f);
			model = glm::translate(model, objectPositions[i]);
			model = glm::scale(model, glm::vec3(0.5f));

			// 계산된 모델행렬을 쉐이더 프로그램에 전송
			shaderGeometryPass.setMat4("model", model);

			// backpack 모델 그리기 명령 호출
			backpack.Draw(shaderGeometryPass);
		}

		// default framebuffer 로 바인딩 복구
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		/* Lighting Pass (G-buffer 에서 pixel 단위로 데이터를 샘플링하여 조명 연산하여 QuadMesh 에 렌더링) */

		// 현재 바인딩된 framebuffer 의 색상 및 깊이 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 조명 연산을 수행하는 쉐이더 프로그램 바인딩
		shaderLightPass.use();

		// 미리 생성해 둔 3개의 G-buffer 들을 각 texture unit 에 바인딩
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

		// 반복문을 광원 갯수만큼 순회하며 array uniform 에 조명 데이터 전송
		for (unsigned int i = 0; i < lightPositions.size(); i++)
		{
			shaderLightPass.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
			shaderLightPass.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);

			// attenuation(감쇄) 계산에 필요한 데이터들 추가 전송
			const float linear = 0.7f;
			const float quadratic = 1.8f;
			shaderLightPass.setFloat("lights[" + std::to_string(i) + "].Linear", linear);
			shaderLightPass.setFloat("lights[" + std::to_string(i) + "].Quadratic", quadratic);
		}

		// 카메라 위치값을 쉐이더 프로그램에 전송
		shaderLightPass.setVec3("viewPos", camera.Position);

		// pixel 단위 조명 연산 결과를 렌더링할 QuadMesh 그리기
		renderQuad();


		// default framebuffer 로 바인딩 복구
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


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


/* 이전 버퍼를 시각화할 QuadMesh 를 렌더링하는 함수 구현 */

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