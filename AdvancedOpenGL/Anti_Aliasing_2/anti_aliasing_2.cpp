#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리
#include <algorithm> // std::min(), std::max() 를 사용하기 위해 포함한 라이브러리

// 이미지 파일 로드 라이브러리 include (관련 설명 하단 참고)
#define STB_IMAGE_IMPLEMENTATION
#include "MyHeaders/stb_image.h"

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"
#include "MyHeaders/camera.h"

#include <iostream>


/* 콜백함수 전방선언 */

// GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언
void processInput(GLFWwindow* window);


// 윈도우 창 생성 옵션
// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이

// 카메라 클래스 생성 (카메라 위치값만 매개변수로 전달함.)
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

	// GLFW 에게 MSAA 알고리즘 사용을 위해, 
	// 각 픽셀 당 4개의 subsample 을 저장할 수 있는 multisample buffer 를 생성하도록, 
	// 버퍼의 픽셀 당 sample 수를 4개로 명시
	glfwWindowHint(GLFW_SAMPLES, 4);


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

	// OpenGL 드라이버에 내장된 MSAA(Multi Sampling Anti Aliasing) 알고리즘 활성화 (MSAA 관련 필기 하단 참고)
	glEnable(GL_MULTISAMPLE);

	// 큐브 렌더링 시 적용할 쉐이더 객체 생성
	Shader shader("MyShaders/anti_aliasing.vs", "MyShaders/anti_aliasing.fs");

	// 스크린 평면 렌더링 시 적용할 쉐이더 객체 생성
	Shader screenShader("MyShaders/off_screen.vs", "MyShaders/off_screen_grayscale.fs");

	// 큐브의 정점 데이터 정적 배열 초기화
	float cubeVertices[] = {
		// positions  
		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	// default framebuffer 에 렌더링할 스크린 평면 (off-screen rendering 을 텍스쳐로 적용하는 평면)의 정점 데이터 배열 초기화
	// 스크린 평면은 좌표계 변환을 수행하지 않아도 되므로, position 좌표값을 NDC 좌표계를 바로 전달해버림.
	// 또한, 스크린 평면은 원근을 고려할 필요가 없으므로, position z값을 별도로 전달할 필요 없이, 버텍스 쉐이더에서 0.0 으로 통일해서 전달할 것임!
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};


	/* 큐브의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	// VBO, VAO 객체(object) 참조 id 를 저장할 변수
	unsigned int cubeVBO, cubeVAO;

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. 
	// -> 그 중에서 0번 location 변수만 사용하도록 활성화한 것!
	glEnableVertexAttribArray(0);

	// 정점 위치 데이터(0번 location 입력변수 in vec3 aPos 에 전달할 데이터) 해석 방식 정의
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// VBO 객체 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// 마찬가지로, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);


	/* 스크린 평면에 대한 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	unsigned int quadVAO, quadVBO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &quadVAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &quadVBO); // VBO(Vertex Buffer Object) 객체 생성

	glBindVertexArray(quadVAO); // 스크린 평면에 대한 렌더링 정보를 저장하기 위해 VAO 객체를 컨텍스트에 바인딩함.

	glBindBuffer(GL_ARRAY_BUFFER, quadVBO); // 정점 데이터를 덮어쓸 VBO 객체 바인딩
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.

	// VBO 객체 설정
	// 정점 위치 데이터 해석 방식 설정
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	// 정점 uv 데이터 해석 방식 설정
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// quadVAO 객체에 저장해둘 cubeVBO 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);


	/* off-screen MSAA 지원 framebuffer 생성 및 설정 */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// FBO 객체에 attach 할 텍스쳐 객체 생성 및 바인딩
	// multisampled buffer 를 지원하는 텍스쳐 객체일 경우, GL_TEXTURE_2D_MULTISAMPLE 상태에 바인딩함.
	unsigned int textureColorBufferMultiSampled;
	glGenTextures(1, &textureColorBufferMultiSampled);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);

	// 텍스쳐 객체 메모리 공간 할당 (할당된 메모리에 이미지 데이터를 덮어쓰지 않음! -> 대신 FBO 에서 렌더링된 데이터를 덮어쓸 거니까!)
	// 텍스쳐 객체의 해상도는 스크린 해상도와 일치시킴 -> 왜냐? 이 텍스쳐는 '스크린 평면'에 적용할 거니까!
	// multisampled buffer 를 지원하는 텍스쳐 객체일 경우, glTexImage2DMultisample() 함수를 이용해서 텍스쳐 메모리 할당
	// 두 번째 매개변수는 multisampled buffer 에서 사용할 subsample 개수를 전달함.
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);

	// GL_TEXTURE_2D_MULTISAMPLE 상태에 바인딩한 텍스쳐 객체 메모리 할당이 끝났으면 바인딩 해제
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// FBO 객체에 생성한 텍스쳐 객체 attach (자세한 매개변수 설명은 LearnOpenGL 본문 참고!)
	// off-screen framebuffer 에 렌더링 시, 텍스쳐 객체에는 최종 color buffer 만 저장하면 되므로, color attachment 만 적용!
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);

	// FBO 객체에 attach 할 RBO(RenderBufferObject) 객체 생성 및 바인딩
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	// RBO 객체 메모리 공간 할당
	// 단일 Renderbuffer 에 stencil 및 depth 값을 동시에 저장하는 데이터 포맷 지정 -> GL_DEPTH24_STENCIL8 
	// multisampled buffer 를 지원하는 Renderbuffer 의 경우, glRenderbufferStorageMultisample() 함수를 이용해서 메모리 할당
	// 또한, 텍스쳐 객체와 마찬가지로 스크린 해상도와 Renderbuffer 해상도를 일치시킴
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);

	// Renderbuffer 메모리 할당이 끝났으면 바인딩 해제
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// FBO 객체에 생성한 RBO 객체 attach (자세한 매개변수 설명은 LearnOpenGL 본문 참고!)
	// off-screen framebuffer 에 렌더링 시, RBO 객체에는 stencil 및 depth buffer 를 저장할 것이므로, GL_DEPTH_STENCIL_ATTACHMENT 를 적용함!
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// 현재 GL_FRAMEBUFFER 상태에 바인딩된 FBO 객체 설정 완료 여부 검사 (설정 완료 조건은 LearnOpenGL 본문 참고)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// FBO 객체가 제대로 설정되지 않았을 시 에러 메시지 출력
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	// 생성한 FBO 객체 설정 완료 후, 다시 default framebuffer 바인딩하여 원상복구
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	/* Blitting 을 위한 off-screen non-multisampled framebuffer 생성 및 설정 (관련 내용 하단 필기 참고) */

	// FBO(FrameBufferObject) 객체 생성 및 바인딩
	unsigned int intermediateFBO;
	glGenFramebuffers(1, &intermediateFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

	// FBO 객체에 attach 할 텍스쳐 객체 생성 및 바인딩
	// 참고로, Blitting 은 색상 버퍼에만 적용할 것이므로, 이를 attach 할 텍스쳐 객체만 생성해주면 됨.
	unsigned int screenTexture;
	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);

	// 텍스쳐 객체 메모리 공간 할당
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	// Texture Filtering(텍셀 필터링(보간)) 모드 설정 -> 스크린 평면은 축소/확대되지 않으므로 별로 중요 x
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// FBO 객체에 생성한 텍스쳐 객체 attach (자세한 매개변수 설명은 LearnOpenGL 본문 참고!)
	// off-screen framebuffer 에 렌더링 시, 텍스쳐 객체에는 최종 color buffer 만 저장하면 되므로, color attachment 만 적용 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

	// 현재 GL_FRAMEBUFFER 상태에 바인딩된 FBO 객체 설정 완료 여부 검사 (설정 완료 조건은 LearnOpenGL 본문 참고)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		// FBO 객체가 제대로 설정되지 않았을 시 에러 메시지 출력
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	// 생성한 FBO 객체 설정 완료 후, 다시 default framebuffer 바인딩하여 원상복구
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// 스크린 평면에 적용할 쉐이더 프로그램의 uniform sampler 변수에 0번 texture unit 위치값 전송
	screenShader.use();
	screenShader.setInt("screenTexture", 0);


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

		// off-screen MSAA 지원 framebuffer 바인딩하여, 이후의 렌더링 결과를 해당 multisampled buffer 에 저장
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// 색상 버퍼 및 깊이 버퍼 초기화 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		/* 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함. */


		/* 변환행렬 계산 및 쉐이더 객체에 전송 */

		// 변환행렬을 전송할 쉐이더 프로그램 바인딩
		shader.use();

		// 카메라의 zoom 값으로부터 투영 행렬 계산
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 계산된 투영행렬을 쉐이더 프로그램에 전송
		shader.setMat4("projection", projection);

		// 계산된 뷰 행렬을 쉐이더 프로그램에 전송
		shader.setMat4("view", view);

		// 단위행렬로 초기화된 모델행렬을 쉐이더 프로그램에 전송 -> 큐브를 변환하지 않음!
		shader.setMat4("model", glm::mat4(1.0f));


		/* 큐브 그리기 */

		// 큐브에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(cubeVAO);

		// 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* off-screen MSAA 지원 framebuffer(frameBuffer) -> non-multisampled framebuffer(intermediateFBO) 로 Blitting */

		// Blitting source framebuffer 는 GL_READ_FRAMEBUFFER 상태에 바인딩한다. (관련 필기 하단 참고)
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);

		// Blitting target framebuffer 는 GL_READ_FRAMEBUFFER 상태에 바인딩한다.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);

		// Blitting 을 수행함으로써, multisampled buffer 에 저장된 색상 버퍼가 
		// screenTexture 텍스쳐 객체(color attachment) 로 저장됨.
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);


		/* second pass (default framebuffer 에 렌더링) */

		// 스크린 평면을 렌더링할 default framebuffer 을 다시 바인딩
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// default framebuffer 에는 스크린 평면만 렌더링하고, 스크린 평면의 프래그먼트가 폐기되면 안되므로, depth test 를 비활성화
		glDisable(GL_DEPTH_TEST);

		// 색상 버퍼를 흰색으로 초기화 (사실 스크린 평면이 배경을 가득 채우고 있어서 어차피 눈에 안보임)
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);


		/* 스크린 평면 그리기 */

		// 스크린 평면에 적용할 쉐이더 프로그램 객체 바인딩
		screenShader.use();

		// 스크린 평면에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(quadVAO);

		// 이 예제에서는 모든 텍스쳐 객체가 0번 texture unit 을 공유할 것이므로, 0번 위치에 텍스쳐 객체가 바인딩되도록 활성화
		glActiveTexture(GL_TEXTURE0);

		// 스크린 평면 텍스쳐 객체도 0번 texture unit 을 사용하므로, 0번 위치에 해당 텍스쳐(color attachment 텍스쳐) 바인딩
		glBindTexture(GL_TEXTURE_2D, screenTexture);

		// 스크린 평면 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwSwapBuffers(window);

		// 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
		glfwPollEvents();
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);

	// while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제
	glfwTerminate();

	return 0;
}

// 전방선언된 콜백함수 정의
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
	stb_image.h

	주요 이미지 파일 포맷을 로드할 수 있는
	싱글 헤더 이미지로드 라이브러리.

	#define 매크로 전처리기를 통해
	특정 매크로를 선언함으로써, 헤더파일 내에서
	해당 매크로 영역의 코드만 include 할 수 있도록 함.

	실제로 stb_image.h 안에 보면

	#ifdef STB_IMAGE_IMPLEMENTATION
	~
	#endif

	요렇게 전처리기가 정의되어 있는 부분이 있음.
	이 부분의 코드들만 include 하겠다는 것이지!
*/

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
	MSAA(Multi Sampling Anti Aliasing)


	MSAA 알고리즘은 aliasing 현상을 해결하기 위해
	그래픽 카드들의 OpenGL 드라이버에 구현되어 있는
	Multi sampling 알고리즘이라고 보면 됨.

	기본적으로, screen 의 각 픽셀 당 sample point 를
	여러 개, 여러 위치에 둠으로써(== subsamples),

	rasterization 과정에서 삼각형 같은 primitive 영역 내에
	각 픽셀의 subsample 이 몇 개 포함되는지에 따라
	해당 픽셀에서 실행된 프래그먼트 쉐이더의 결과값(== 색상값)을
	얼마만큼 반영할 것인지 결정하는 알고리즘으로 보면 됨.

	자세한 내용은 LearnOpenGL 본문에 다 나와있으니 참고하면 되고...

	여기서 주의할 점은,
	MSAA 알고리즘은 기본적으로 대부분의 그래픽 카드에 구현된
	OpenGL 드라이버에서 기본으로 활성화되어 있는 상태임.

	그러나, MSAA 알고리즘은 subsample 개수가 늘어나면,
	그에 비례해서 프레임 버퍼 및 각종 버퍼들의 메모리 사이즈가 늘어나는
	성능 상의 단점이 있기 때문에, 어떤 드라이버에서는 해당 알고리즘을 비활성화 시켜놓음.

	따라서, 기본값이 활성화 상태인 드라이버 상에서는
	glEnable(GL_MULTISAMPLE); 라는 코드가 중복 활성화로도 볼 수 있지만,
	몇몇의 비활성화된 상태의 드라이버들까지 고려하면,
	MSAA 를 확실하게 활성화한다는 의미에서 해당 활성화 코드를 명시하는 게 좋음!
*/

/*
	Blitting multisampled buffer


	여러 개의 subsample 이 있는 multisampled buffer 는 
	한 픽셀에 대해 여러 개의 색상 값을 가지고 있음. 

	이는 일반적인 off-screen framebuffer 와는 다른 특성이며,
	이러한 특성으로 인해 multisampled buffer 는 
	shader 내에서의 텍스쳐 샘플링같은 작업에 직접적으로 사용되지는 못함.

	이를 해결하기 위해, multisampled buffer 를
	일반적인 off-screen framebuffer 로 변환하는 작업이 필요한데,
	이를 'Blitting' 이라고 함.


	'Blitting'은 "Block Image Transfer"의 줄임말로, 
	이미지나 버퍼와 같은 데이터 블록을 다른 위치로 전송하는 과정을 뜻함.

	'Blitting' 과정을 자세히 설명하자면,
	먼저 'resolve' 라는 과정을 거쳐야 함.

	각 subsample 에서 계산된 색상 값들을 적절한 방법으로 결합하거나 처리하여 
	최종적으로 하나의 픽셀에 대한 색상 값을 얻게 되는데, 
	이 과정을 LearnOpenGL 본문에서는 "resolve"라고 부름. 
	
	이후 별도의 non-multisampled 색상 버퍼(== 일반적인 off-screen framebuffer)로 
	그 값을 복사하는 것이 일반적임.


	OpenGL에서 glBlitFramebuffer() 함수는
	framebuffer 의 특정 영역을 다른 framebuffeer 로 블릿(blit)하는 데 사용됨.

	이 함수를 통해 multisampled buffer 에서 일반(non-multisampled) framebuffer 로
	이미지를 복사하면서 동시에 multisampled 데이터를 
	'resolve' 하는 것으로 보면 됨.
*/

/*
	framebuffer 바인딩 시, 
	GL_FRAMEBUFFER vs GL_READ_FRAMEBUFFER vs GL_DRAW_FRAMEBUFFER


	OpenGL 에서 glBindFramebuffer() 함수를 사용해서
	프레임버퍼 객체를 바인딩할 때, 위와 같이 3개의 상태값에 바인딩할 수 있음.

	첫 번째 상태값인 GL_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
	해당 프레임버퍼는 GL_READ_FRAMEBUFFER 와 GL_DRAW_FRAMEBUFFER 모두에
	바인딩된 것과 동일하게 작동함.

	이는 바인딩된 해당 프레임버퍼가
	'프레임버퍼 읽기 관련 연산'과 '프레임버퍼 쓰기 관련 연산' 모두에
	영향을 받게 됨을 의미함.

	두 번째 상태값인 GL_READ_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
	해당 프레인버퍼는 '프레임버퍼 읽기 관련 연산에만' 영향을 받게 됨.

	세 번째 상태값인 GL_DRAW_FRAMEBUFFER 에 프레임버퍼를 바인딩하면,
	해당 프레인버퍼는 '프레임버퍼 쓰기 관련 연산에만' 영향을 받게 됨.


	ex> 
	glReadPixels() 같은 함수는 '프레임버퍼 읽기 관련 연산'에 해당하므로,
	GL_READ_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼 또는
	GL_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼로부터 pixel 값을 읽어올 것임!

	반면, glClear() 같은 함수는 '프레임버퍼 쓰기 관련 연산'에 해당하므로,
	GL_DRAW_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼 또는
	GL_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼를 초기화할 것임!


	반대로, 어떤 함수는 '읽기 전용 프레임버퍼'와 '쓰기 전용 프레임버퍼'를
	각각 필요로하는 케이스도 있는데, 그 대표적인 케이스가 바로 glBlitFramebuffer() 라고 보면 됨.

	glBlitFramebuffer() 함수를 호출하면,
	해당 함수를 호출한 시점에

	GL_READ_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼는 
	Blitting 과정에서 데이터를 읽어올 source framebuffer 로 인식하며,

	GL_DRAW_FRAMEBUFFER 상태에 바인딩되어 있는 프레임버퍼는
	Blitting 과정에서 읽어온 데이터를 복사할 target framebuffer 로 인식함.
*/