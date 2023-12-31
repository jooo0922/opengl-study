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


	/* 깊이 테스트 및 스텐실 테스트 관련 OpenGL 상태 설정 */

	// 깊이 테스트 활성화
	glEnable(GL_DEPTH_TEST);

	// 깊이 버퍼와 현재 프래그먼트의 깊이값 비교 연산 모드를 GL_LESS 로 설정 (기본 모드)
	glDepthFunc(GL_LESS);

	// 스텐실 테스트 활성화
	glEnable(GL_STENCIL_TEST);

	// 스텐실 버퍼와 기준값(reference value) 비교 연산 모드를 GL_NOTEQUAL 로 설정
	// 참고로 세 번째 인자는, 스텐실 버퍼와 기준값을 서로 비교하기 전, 두 값을 bitwise AND 연산(&)으로 비트 연산시킬 값
	// 참고로, 0xFF 는 이진수로 1111 1111 에 해당하며, 이 이진수와 AND 연산을 하면 항상 자기 자신이 나옴!
	// https://github.com/jooo0922/cpp-study/blob/main/TBCppStudy/Chapter3_8/Chapter3_8.cpp 참고
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

	// 스텐실 버퍼 쓰기(write) 방식 설정
	// -> 아래 방식은 '모든 테스트를 통과한 프래그먼트의 스텐실 버퍼 값을 glStencilFunc() 에서 설정한 기준값 1 로 덮어씀'
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


	/* 쉐이더 프로그램 생성 */

	// Shader 클래스를 생성함으로써, 쉐이더 객체 / 프로그램 객체 생성 및 컴파일 / 링킹
	Shader shader("MyShaders/stencil_testing.vs", "MyShaders/stencil_testing.fs");

	// object outlining 에 적용할 쉐이더 객체 추가 생성
	Shader shaderSingleColor("MyShaders/stencil_testing.vs", "MyShaders/stencil_single_color.fs"); 


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

	// planeVAO 객체에 저장해둘 cubeVBO 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);

	// 텍스쳐 로드 및 텍스쳐 객체 생성
	unsigned int cubeTexture = loadTexture("images/marble.jpg"); // 큐브에 적용할 텍스쳐 객체의 참조 ID 저장
	unsigned int floorTexture = loadTexture("images/metal.png"); // 바닥 평면에 적용할 텍스쳐 객체의 참조 ID 저장

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

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		// 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// 색상 버퍼, 깊이 버퍼, 스텐실 버퍼 초기화를 명시하는 비트 단위 연산을 수행하여 비트 플래그 설정 (state-using)
		// 스텐실 버퍼 초기화 값은 glClearStencil() 로 설정할 수 있는데, 기본값은 0 -> 즉, 스텐실 버퍼의 값들을 모두 0으로 초기화함!
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		/* 각 쉐이더 프로그램에 전송할 변환 행렬 계산 */

		// 모델 행렬을 단위행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 카메라 줌 효과를 구현하기 위해 fov 값을 실시간으로 변경해야 하므로,
		// fov 값으로 계산되는 투영행렬을 런타임에 매번 다시 계산해서 쉐이더 프로그램으로 전송해줘야 함. > 게임 루프에서 계산 및 전송
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // 투영 행렬 생성


		/* Object outlining 에 적용할 쉐이더 프래그램 객체 바인딩 및 uniform 변수 전송 */

		shaderSingleColor.use();

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송
		shaderSingleColor.setMat4("view", view);

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송
		shaderSingleColor.setMat4("projection", projection);


		/* 큐브 및 바닥 평면에 적용할 쉐이더 프로그램 객체 바인딩 및 uniform 변수 전송 */
		
		shader.use();

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송
		shader.setMat4("view", view); 

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송
		shader.setMat4("projection", projection); 


		/* 바닥 평면 그리기 */

		// 모든 테스트를 통과한 바닥 평면의 프래그먼트에 스텐실 버퍼 값을 덮어쓰기 전, 0x00(0000 0000) 과 bitwise AND 연산을 해줌
		// 0x00 과 AND 연산하면 모든 비트는 0 이 되므로, 결국 모든 스텐실 버퍼 값을 0으로 초기화하는 glClear(GL_STENCIL_BUFFER_BIT) 와 동일한 기능!
		// 이를 통해, 바닥 평면을 그릴 때에는 스텐실 버퍼를 덮어쓰지 못하도록 비활성화 함!
		glStencilMask(0x00);

		// 바닥 평면에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(planeVAO);

		// 바닥 평면 텍스쳐 객체도 0번 texture unit 을 공유하므로, 이번엔 0번 위치에 바닥 평면 텍스쳐가 바인딩되도록 활성화
		glBindTexture(GL_TEXTURE_2D, floorTexture);

		// 바닥 평면 위치로 이동시키는 이동행렬(단위행렬 -> 별다른 이동 x)을 모델 행렬에 적용
		shader.setMat4("model", glm::mat4(1.0));

		// 바닥 평면 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 바닥 평면을 다 그렸으면 바인딩한 VAO 객체 해제
		glBindVertexArray(0);


		/* 첫 번째 큐브 그리기 */

		// 원본 큐브를 그릴 때에는 모든 스텐실 테스트를 통과시킴 -> 원본 큐브 영역의 스텐실 버퍼 값을 모두 1로 덮어쓰기 위함!
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		// 모든 테스트를 통과한 원본 큐브의 프래그먼트에 스텐실 버퍼 값을 덮어쓰기 전, 0xFF(1111 1111) 과 bitwise AND 연산을 해줌
		// 0xFF 와 AND 연산하면 모든 비트는 자기 자신이 나오므로, 결국 스텐실 버퍼 값 덮어쓰기를 활성화 하겠다는 뜻!
		glStencilMask(0xFF);

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


		/* Outlining 큐브 그리기 */

		/* 첫 번째 Outlining 큐브 그리기 */

		// Outlining 큐브를 그릴 때에는, 스텐실 버퍼 값이 기준값 1과 다른 프래그먼트만 통과시키도록 stencil test 함수를 변경
		// -> 앞서 스텐실 버퍼 값을 1 로 덮어쓴 원본 큐브 영역과 겹치는 영역의 프래그먼트들을 모두 discard 시키려는 것!
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

		// 이번에도 스텐실 버퍼를 덮어쓰기 전 값을 0 으로 초기화함. -> 스텐실 버퍼 덮어쓰기 비활성화
		// 이게 왜 덮어쓰기 비활성화와 같냐면, 방금 전 변경한 Stencil Test 함수로 인해 스텐실 버퍼 값이 1 인 영역은 어차피 테스트를 통과 못하니 버퍼를 덮어쓸 일이 없고,
		// 나머지 테스트를 통과한 영역(스텐실 버퍼 값이 0 인 영역)은 기준값 1로 덮어쓰기 직전에 0x00 과 bitwise AND 연산에 의해 0 으로 초기화되므로, 
		// 사실상 기존 스텐실 버퍼 값에서 달라지는 게 없어짐!
		glStencilMask(0x00);

		// Outlining 큐브는 외곽선 강조를 위한 UI 이므로, 깊이값과 관계없이 항상 맨 앞에 그려지도록 깊이 테스트 비활성화
		glDisable(GL_DEPTH_TEST);

		// Outlining 큐브에 적용할 쉐이더 프로그램 바인딩
		shaderSingleColor.use();

		// Outlining 큐브는 원본 큐브보다 살짝 크기를 키워야 하므로, 크기 행렬 계산 시 사용할 scale 값을 우선 초기화
		float scale = 1.1f;

		// 큐브에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(cubeVAO);

		// 첫 번째 Outlining 큐브에 적용할 모델 행렬에 초기화
		model = glm::mat4(1.0f);

		// 첫 번째 Outlining 큐브 위치로 이동시키는 이동행렬을 모델 행렬에 적용
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));

		// 첫 번째 Outlining 큐브를 확대하는 크기행렬을 모델 행렬에 적용
		model = glm::scale(model, glm::vec3(scale, scale, scale));

		// 첫 번째 Outlining 큐브에 적용할 모델 행렬을 쉐이더 프로그램에 전송
		shaderSingleColor.setMat4("model", model);

		// 첫 번째 Outlining 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* 두 번째 Outlining 큐브 그리기 */

		// 두 번째 Outlining 큐브에 적용할 모델 행렬에 초기화
		model = glm::mat4(1.0f);

		// 두 번째 Outlining 큐브 위치로 이동시키는 이동행렬을 모델 행렬에 적용
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));

		// 두 번째 Outlining 큐브를 확대하는 크기행렬을 모델 행렬에 적용
		model = glm::scale(model, glm::vec3(scale, scale, scale));

		// 두 번째 Outlining 큐브에 적용할 모델 행렬을 쉐이더 프로그램에 전송
		shaderSingleColor.setMat4("model", model);

		// 두 번째 Outlining 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// 모든 큐브들을 다 그렸으면 바인딩한 큐브 VAO 객체 해제
		glBindVertexArray(0);


		/* Stencil Test & Depth Test 관련 설정 초기화 */

		// 스텐실 버퍼 값 덮어쓰기 활성화
		glStencilMask(0xFF);

		// Stencil Test 함수를 GL_ALWAYS 로 변경하여 모든 프래그먼트가 테스트를 통과하도록 함.
		// 테스트를 통과한 모든 프래그먼트들은 스텐실 버퍼 값을 기준값인 0 으로 덮어씀 -> glClear(GL_STENCIL_BUFFER_BIT) 과 동일한 역할!
		// 다음 프레임에서 바닥 평면부터 그리기 시작할 때, 바닥 평면의 모든 프래그먼트들의 스텐실 버퍼 값을 0으로 덮어쓰려 하겠군!
		// -> 근데 어차피 바닥 평면에 덮어쓸 스텐실 버퍼 값들은 glStencilMask(0x00); 에 의해 0x00 과 bitwise AND 연산하여 0 으로 되어버린다고 했었지? 
		glStencilFunc(GL_ALWAYS, 0, 0xFF);

		// Outlining 큐브를 그릴 때 임시로 비활성화 해둔 깊이 테스트를 다시 활성화
		glEnable(GL_DEPTH_TEST);


		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	// 이들은 미리 생성 후 저장해두는 이유는, 렌더링이 진행중에 필요한 객체들을 다시 바인딩하여 교체하기 위한 목적이므로,
	// 렌더링이 끝났다면 더 이상 메모리에 남겨둘 이유가 없음!!
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &planeVBO);

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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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