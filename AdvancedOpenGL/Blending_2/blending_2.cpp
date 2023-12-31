#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리

// stb_image.h 헤더파일 라이브러리 관련 설명 하단 필기 참고
#define STB_IMAGE_IMPLEMENTATION
#include "MyHeaders/stb_image.h"

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"
#include "MyHeaders/camera.h"

#include <iostream>

// std::map 자료구조를 사용하기 위해 포함 -> 추가한 <key, value> 쌍을 key 값의 오름차순으로 정렬함
#include <map>

using namespace std;

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void processInput(GLFWwindow* window, Shader ourShader); // GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언
unsigned int loadTexture(const char* path); // 텍스쳐 이미지 로드 및 객체 생성 함수 선언 (텍스쳐 객체 참조 id 반환)

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

	// GLFW 윈도우(창) 설정 구성
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // GLFW 에게 OpenGL 3.3 버전 사용 명시
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // GLFW 에게 core-profile 버전 사용 명시 (Core-profile vs Immediate mode 비교글 참고)

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
		glfwTerminate(); // 현재 남아있는 glfw 윈도우 제거, 리소스 메모리 해제, 라이브러리 초기화 이전 상태로 되돌림. (glfwInit() 과 반대 역할!))
		return -1;
	}

	glfwMakeContextCurrent(window); // 새로운 GLFWwindow 윈도우 객체를 만들면, 해당 윈도우의 OpenGL 컨텍스트를 현재 실행중인 스레드의 현재 컨텍스트로 지정

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // GLFWwindow 창 크기 변경(resize) 감지 시, 발생시킬 리사이징 콜백함수 등록 (콜백함수는 항상 게임루프 시작 전 등록!)
	glfwSetCursorPosCallback(window, mouse_callback); // GLFWwindow 에 마우스 커서 입력 감지 시, 발생시킬 콜백함수 등록
	glfwSetScrollCallback(window, scroll_callback); // GLFWwindow 에 마우스 스크롤 입력 감지 시, 발생시킬 콜백함수 등록

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

	// 각 프래그먼트에 대한 깊이 버퍼(z-buffer)를 새로운 프래그먼트의 깊이값과 비교해서
	// 프래그먼트 값을 덮어쓸 지 말 지를 결정하는 Depth Test(깊이 테스팅) 상태를 활성화함
	// OpenGL 컨텍스트의 state-setting 함수 중 하나겠지! 
	glEnable(GL_DEPTH_TEST);

	// Blending 활성화
	glEnable(GL_BLEND);

	// depth buffer 깊이 테스트하는 비교 연산 모드를 GL_LESS (기본 모드) 로 지정
	glDepthFunc(GL_LESS);

	// Blending 연산 모드를 (fragment 색상 * fragment alpha) + (color buffer 색상 * (1 - fragment alpha)) 형태로 설정
	// 해당 블렌딩 함수 모드는 반투명 오브젝트를 그릴 때, 가장 많이 사용되는 연산 모드!
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Shader 클래스를 생성함으로써, 쉐이더 객체 / 프로그램 객체 생성 및 컴파일 / 링킹
	Shader shader("MyShaders/blending.vs", "MyShaders/blending.fs");

	// 큐브의 정점 데이터 배열 초기화
	float cubeVertices[] = {
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	// 바닥 평면의 정점 데이터 배열 초기화
	float planeVertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};

	// blending_transparent_window.png 텍스쳐를 적용할 QuadMesh 의 정점 데이터 배열 초기화
	float  transparentVertices[] = {
		// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};


	/* 큐브에 대한 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	unsigned int cubeVBO, cubeVAO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &cubeVAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &cubeVBO); // VBO(Vertex Buffer Object) 객체 생성 > VBO 객체를 만들어서 정점 데이터를 GPU 로 전송해야 한 번에 많은 양의 정점 데이터를 전송할 수 있음!

	glBindVertexArray(cubeVAO); // VAO 객체 먼저 컨텍스트에 바인딩(연결)함. > 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // OpenGL 컨텍스트 중에서, VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.

	// VBO 객체 설정
	// 정점 위치 데이터 해석 방식 설정
	glEnableVertexAttribArray(0); // 버텍스 쉐이더의 0번 location 변수만 사용하도록 활성화한 것!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// 정점 uv 데이터 해석 방식 설정
	glEnableVertexAttribArray(1); // 버텍스 쉐이더의 1번 location 변수만 사용하도록 활성화한 것!
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	// cubeVAO 객체에 저장해둘 cubeVBO 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);


	/* 바닥 평면에 대한 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	unsigned int planeVAO, planeVBO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &planeVAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &planeVBO); // VBO(Vertex Buffer Object) 객체 생성

	glBindVertexArray(planeVAO); // 바닥 평면에 대한 렌더링 정보를 저장하기 위해 VAO 객체를 컨텍스트에 바인딩함.

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO); // 광원 큐브와 빛을 받는 큐브 모두 동일한 버텍스 데이터(위치값)를 사용하므로, 버텍스 데이터가 담긴 VBO 객체는 공유해도 됨.
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.

	// VBO 객체 설정
	// 정점 위치 데이터 해석 방식 설정
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// 정점 uv 데이터 해석 방식 설정
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


	/* QuadMesh 에 대한 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩 */

	unsigned int transparentVAO, transparentVBO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &transparentVAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &transparentVBO); // VBO(Vertex Buffer Object) 객체 생성

	glBindVertexArray(transparentVAO); // QuadMesh 에 대한 렌더링 정보를 저장하기 위해 VAO 객체를 컨텍스트에 바인딩함.

	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO); // QuadMesh 의 VBO 객체 바인딩.
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.

	// VBO 객체 설정
	// 정점 위치 데이터 해석 방식 설정
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// 정점 uv 데이터 해석 방식 설정
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


	// transparentVAO 객체에 저장해둘 transparentVBO 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);

	// 텍스쳐 로드 및 텍스쳐 객체 생성
	unsigned int cubeTexture = loadTexture("images/marble.jpg"); // 큐브에 적용할 텍스쳐 객체의 참조 ID 저장
	unsigned int floorTexture = loadTexture("images/metal.png"); // 바닥 평면에 적용할 텍스쳐 객체의 참조 ID 저장
	unsigned int transparentTexture = loadTexture("images/blending_transparent_window.png"); // QuadMesh 에 적용할 텍스쳐 객체의 참조 ID 저장

	// 투명 텍스쳐(blending_transparent_window.png)를 적용할 QuadMesh 의 위치값을 동적 배열 vector 에 초기화하여 저장
	vector<glm::vec3> windows
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 0.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f),
	};

	// 프래그먼트 쉐이더에 선언된 uniform sampler 변수에 texture unit 위치값 전송
	// 이 예제에서는 두 텍스쳐 객체가 하나의 sampler 변수를 공유해서 사용하므로,
	// texture unit 또한 0번 위치를 공유해서 사용할 것임!
	shader.use(); // 쉐이더 프로그램 바인딩
	shader.setInt("texture1", 0);

	// while 문으로 렌더링 루프 구현
	while (!glfwWindowShouldClose(window))
	{
		// 카메라 이동속도 보정을 위한 deltaTime 계산
		float currentFrame = static_cast<float>(glfwGetTime()); // 현재 프레임 경과시간
		deltaTime = currentFrame - lastFrame; // 현재 프레임 경과시간 - 마지막 프레임 경과시간 = 두 프레임 사이의 시간 간격
		lastFrame = currentFrame; // 마지막 프레임 경과시간을 현재 프레임 경과시간으로 업데이트!

		processInput(window, shader); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리


		/* 카메라로부터의 거리에 따라 오름차순으로 windows 동적 배열의 위치값을 정렬! */

		// <key, value> 쌍을 추가할 때마다 key 값의 오름차순 정렬하는 std::map 타입 선언 (std::map 자료구조 관련 하단 필기 참고)
		std::map<float, glm::vec3> sorted;

		// QuadMesh 의 위치값이 담긴 동적 배열 windows 를 반복 순회
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			// std::map 구조에 key 값으로 추가할 '카메라로부터의 거리' 계산
			float distance = glm::length(camera.Position - windows[i]);

			// std::map 에 distance 를 key 값으로 하여 QuadMesh 의 위치값 추가 -> 카메라로부터의 거리 순으로 오름차순 정렬!
			sorted[distance] = windows[i];
		}


		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		// 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼를 초기화함. 
		// glEnable() 로 깊이 테스팅을 활성화한 상태이므로, 이전 프레임에 그렸던 깊이 버퍼도 초기화하도록,
		// 깊이 버퍼 제거를 명시하는 비트 단위 연산을 수행하여 비트 플래그 설정 (state-using)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		shader.use(); // 큐브에 적용할 쉐이더 프로그램 객체 바인딩

		// 모델 행렬을 단위행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 카메라 줌 효과를 구현하기 위해 fov 값을 실시간으로 변경해야 하므로,
		// fov 값으로 계산되는 투영행렬을 런타임에 매번 다시 계산해서 쉐이더 프로그램으로 전송해줘야 함. > 게임 루프에서 계산 및 전송
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // 투영 행렬 생성

		shader.setMat4("view", view); // 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송
		shader.setMat4("projection", projection); // 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송

		shader.setMat4("model", model); // 최종 계산된 모델 행렬을 바인딩된 쉐이더 프로그램의 유니폼 변수로 전송


		/* 첫 번째 큐브 그리기 */

		// 큐브에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(cubeVAO);

		// 이 예제에서는 모든 텍스쳐 객체가 0번 texture unit 을 공유할 것이므로, 0번 위치에 텍스쳐 객체가 바인딩되도록 활성화
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);

		// 첫 번째 큐브 위치로 이동시키는 이동행렬을 모델 행렬에 적용
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader.setMat4("model", model);

		// 첫 번째 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* 두 번째 큐브 그리기 */

		// 모델행렬을 다시 단위행렬로 초기화
		model = glm::mat4(1.0f);

		// 두 번째 큐브 위치로 이동시키는 이동행렬을 모델 행렬에 적용
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		shader.setMat4("model", model);

		// 두 번째 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* 바닥 평면 그리기 */

		// 바닥 평면에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(planeVAO);

		// 바닥 평면 텍스쳐 객체도 0번 texture unit 을 공유하므로, 이번엔 0번 위치에 바닥 평면 텍스쳐가 바인딩되도록 활성화
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		// 바닥 평면 위치로 이동시키는 이동행렬(단위행렬 -> 별다른 이동 x)을 모델 행렬에 적용
		shader.setMat4("model", glm::mat4(1.0));

		// 바닥 평면 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 6);


		/* Alpha Testing 을 적용할 QuadMesh 그리기 */

		// QaudMesh 에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(transparentVAO);

		// QuadMesh 텍스쳐 객체도 0번 texture unit 을 공유하므로, 이번엔 0번 위치에 QuadMesh 텍스쳐가 바인딩되도록 활성화
		glBindTexture(GL_TEXTURE_2D, transparentTexture);

		// 카메라로부터의 거리에 따라 오름차순 정렬된 QuadMesh 위치값들을 역순으로 순회 -> 즉, 내림차순 순회! (역순 이터레이터 관련 설명 하단 필기 참고)
		for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
		{
			// std::map 구조의 역순 이터레이터 it 에 저장된 <const key, T> 쌍의 T 값(= 위치값)으로 이동 행렬 계산
			model = glm::mat4(1.0f);
			model = glm::translate(model, it->second);

			// 계산된 각 모델 행렬을 쉐이더 프로그램으로 전송
			shader.setMat4("model", model);

			// QuadMesh 그리기 명령
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}


		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	// 이들은 미리 생성 후 저장해두는 이유는, 렌더링이 진행중에 필요한 객체들을 다시 바인딩하여 교체하기 위한 목적이므로,
	// 렌더링이 끝났다면 더 이상 메모리에 남겨둘 이유가 없음!!
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteVertexArrays(1, &transparentVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &transparentVBO);

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
void processInput(GLFWwindow* window, Shader ourShader)
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

// 텍스쳐 이미지 로드 및 객체 생성 함수 구현부 (텍스쳐 객체 참조 id 반환)
unsigned int loadTexture(const char* path)
{
	unsigned int textureID; // 텍스쳐 객체(object) 참조 id 를 저장할 변수 선언
	glGenTextures(1, &textureID); // 텍스쳐 객체 생성

	int width, height, nrComponents; // 로드한 이미지의 width, height, 색상 채널 개수를 저장할 변수 선언

	// 이미지 데이터 가져와서 char 타입의 bytes 데이터로 저장. 
	// 이미지 width, height, 색상 채널 변수의 주소값도 넘겨줌으로써, 해당 함수 내부에서 값을 변경. -> 출력변수 역할
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		// 이미지 데이터 로드 성공 시 처리

		// 이미지 데이터의 색상 채널 개수에 따라 glTexImage2D() 에 넘겨줄 픽셀 데이터 포맷의 ENUM 값을 결정
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		// 텍스쳐 객체 바인딩 및 로드한 이미지 데이터 쓰기
		glBindTexture(GL_TEXTURE_2D, textureID); // GL_TEXTURE_2D 타입의 상태에 텍스쳐 객체 바인딩 > 이후 텍스쳐 객체 설정 명령은 바인딩된 텍스쳐 객체에 적용.
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); // 로드한 이미지 데이터를 현재 바인딩된 텍스쳐 객체에 덮어쓰기
		glGenerateMipmap(GL_TEXTURE_2D); // 현재 바인딩된 텍스쳐 객체에 필요한 모든 단계의 Mipmap 을 자동 생성함. 

		// 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체 설정하기
		// Texture Wrapping 모드를 반복 모드로 설정 ([(0, 0), (1, 1)] 범위를 벗어나는 텍스쳐 좌표에 대한 처리)
		// * RGBA 포맷에서 Texture Wrapping 설정 관련 하단 필기 참고!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		// 텍스쳐 축소/확대 및 Mipmap 교체 시 Texture Filtering (텍셀 필터링(보간)) 모드 설정 (관련 필기 정리 하단 참고)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		// 이미지 데이터 로드 실패 시 처리
		std::cout << "Texture failed to load at path: " << path << std::endl;
	}

	// 텍스쳐 객체에 이미지 데이터를 전달하고, 밉맵까지 생성 완료했다면, 로드한 이미지 데이터는 항상 메모리 해제할 것!
	stbi_image_free(data);

	// 텍스쳐 객체 참조 ID 반환
	return textureID;
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
	RGBA 포맷에서 Texture Wrapping 모드 설정 관련


	이 때, 투명도 값인 Alpha 채널이 존재하는 텍스쳐일 경우,

	Texture Wrapping 모드가 반복 모드일 때,
	bottom border 에 존재하는 픽셀들이

	완전 투명해야 할 top border 에
	희끄무레하게 반복되는 현상이 발생할 수 있으므로,

	이를 방지하기 위해, 투명도 값이 존재하는 GL_RGBA 포맷일 경우,
	Texture Wrapping 모드를 GL_CLAMP_TO_EDGE, 즉,
	border 의 픽셀을 반복적으로 샘플링하는 방식을 취하도록 함!
*/

/*
	std::map<const key, T> 자료구조는 어떻게 오름차순으로 자동 정렬 하는가?

	
	std::map 자료구조는 
	<key, value> 쌍의 데이터를 추가할 시,
	key 값의 오름차순으로 데이터를 자동 정렬 해주는 자료구조.

	그런데, std::map 은 퀵 정렬 같은 정렬 알고리즘이 아니며,
	오로지 '자료구조로써' 데이터들을 오름차순으로 관리해 줌.

	이게 어떻게 가능하냐?

	그 비결은 std::map 이 '레드블랙트리' 기반으로 구현되어 있기 때문!

	레드블랙트리는
	최소한의 높이값을 유지하는 균형있는 이진 탐색 트리의 한 종류로써,
	
	레드블랙트리는 항상
	왼쪽 하위트리는 부모보다 작고, 
	오른쪽 하위트리는 부모보다 큰 노드들을 추가하도록 설계되어 있음.

	따라서, 왼쪽 노드 -> 오른쪽 노드로 갈수록
	데이터가 오름차순으로 저장되어 있다고도 할 수 있겠지?

	따라서, '데이터를 레드블랙트리 구조로 관리하는 것 만으로도'
	어느 정도는 '데이터를 오름차순으로 관리하고 있다' 라고 볼 수 있는 것임!

	이처럼, std::map, std::set 같은 STL 컨테이너들은
	내부적으로 레드블랙트리를 사용하여 데이터를 저장함으로써,
	정렬된 데이터를 쉽게 다룰 수 있도록 한 것임!
*/

/*
	QuadMesh 를 카메라로부터의 거리에 따라 정렬한 이유는 무엇인가?
	또, 카메라로부터의 거리가 먼 순서로 그리기 명령을 호출한 이유는 무엇인가?


	이는, Blending 기법에서 흔히 발생하는 문제를 
	해결하기 위한 처리라고 보면 됨.

	(반)투명 오브젝트를 blending 기법으로 렌더링할 때,
	
	depth test 에 의해
	(반)투명 오브젝트의 뒷쪽에 있는 오브젝트들의 프래그먼트가
	discard 되는 이슈가 굉장히 자주 발생함.

	이렇게 렌더링된 오브젝트를 보면 매우 어색하게 보이는데,
	분명 (반)투명한 오브젝트인데 뒤에 오브젝트들이 안보이는게 되는 것임.


	이러한 현상은 주로 어떨 때 발생하냐면,
	뒷쪽 오브젝트보다 (반)투명 오브젝트가 먼저 그려질 때 발생하게 됨.

	왜냐? 
	(반)투명 오브젝트가 먼저 그려지면,
	현재 color buffer 에 저장된 색상과 (반)투명 오브젝트의 색상끼리만
	blending 을 수행하여, 그 결과값이 color buffer 에 업데이트 되고,

	이로 인해 나중에 그려지는 뒷쪽 오브젝트의 색상은
	blending 결과 업데이트된 color buffer 에도 반영이 안됨.

	뿐만 아니라, depth test 에 의해
	나중에 그려지는 뒷쪽 오브젝트의 프래그먼트들이 모두 탈락함으로써,
	color buffer 에도 색상을 남기지 못한 채 프래먼트들이 discard 되어버리는 것임!


	그렇다면, 이를 해결하려면 어떻게 해야겠어?

	뒷쪽 오브젝트들의 프래그먼트들은 
	먼저 그리나, 나중에 그리나, depth test 에 의해
	어차피 discard 되어버릴 운명이지?

	그렇지만, (반)투명 오브젝트들보다 먼저 그리기라도 한다면,
	자신의 색상만큼은 'color buffer 에 남길 수' 있겠지!!

	그렇게 되면,
	(반)투명 오브젝트들이 그려지는 시점에
	뒷쪽 오브젝트들의 프래그먼트들은 모두 discard 되겠지만,
	
	blending 연산 시, 
	뒷쪽 오브젝트들의 색상이 color buffer 에 혼합된 채로 남아있으므로,
	'뒷쪽 오브젝트들의 색상이 반영된 color buffer 와 blending 연산'을 수행할 수 있게 되고,
	이로써 뒷쪽 오브젝트들의 프래그먼트들은 버려지더라도, 
	
	색상만큼은 color buffer 에 살아남아 반영될 수 있다는 것이지!

	
	이러한 아이디어에 착안해서 렌더링 순서를 정하자면,

	1. 우선 불투명 오브젝트들을 모두 먼저 그림으로써 color buffer 에 자기 색상을 남길 수 있도록 하고,

	2. (반)투명 오브젝트들을 카메라로부터의 거리에 따라 정렬하고,

	3. (반)투명 오브젝트들을 카메라로부터의 거리가 멀수록, 
	즉, depth test 에 의해 discard 될 확률이 높은 오브젝트들 순으로 그려줌으로써,
	마찬가지로 color buffer 에 자기 색상을 남길 수 있도록 하는 것이지!


	이러한 이유로 QuadMesh 의 위치값과 렌더링 순서를
	카메라로부터의 거리에 따라 정렬해서 정했던 것임!
*/

/*
	std::map::reverse_iterator 타입은 뭘까?


	이는 std::map 구조에 오름차순으로 저장된 <const key, T> 쌍의
	데이터들을 역순으로 순회할 수 있게 해주는 이터레이터 라고 보면 됨.

	그래서, 이터레이터의 초기값 it 를 sorted.rbegin() 으로 할당했는데,
	이는 std::map 구조의 역방향 시작 지점의 요소를 반환하며, (reverse-begin)
	반대로, 해당 역순 반복 순회를 it 가 sorted.rend() 가 될때까지 반복하도록 설정했는데,
	이는 std::map 구조의 역방향 종료 지점의 요소를 반환함. (reverse-end)

	또한, 이터레이터 it->second 와 같이 포인터 멤버에 접근하고 있는데,
	이는 이터레이터가 <const key, T> 형태의 <key, value> 쌍에서
	T, 즉, value 를 가리키는 포인터 멤버라고 보면 됨.

	그렇다며느 it->first 는
	key 값에 접근하는 포인터 멤버라고 보면 되겠지?

	
	이처럼 카메라로부터의 거리값의 오름차순으로 정렬된 데이터를
	역순으로, 즉, 내림차순으로 순회하며 그리기 명령을 수행했다는 것은,

	카메라로부터 거리가 먼 오브젝트부터 먼저 그리도록 했다는 의미라고 보면 됨!
*/