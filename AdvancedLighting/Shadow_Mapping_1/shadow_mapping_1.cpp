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

// 텍스쳐 이미지 로드 및 객체 생성 함수 선언 (텍스쳐 객체 참조 id 반환)
unsigned int loadTexture(const char* path);

// shadow map 에 깊이 버퍼를 저장할 씬을 렌더링하는 함수 선언
void renderScene(const Shader& shader);

// 씬에 큐브를 렌더링하는 함수 선언
void renderCube();

// shadow map 을 샘플링하여 깊이 버퍼를 시각화할 QuadMesh 를 렌더링하는 함수 선언
void renderQuad();


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

// Plane VAO 객체(object) 참조 id 를 저장할 변수를 전역으로 선언 (why? renderScene() 함수에서도 참조해야 함.)
unsigned int planeVAO;

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

	// 큐브 렌더링 시 적용할 쉐이더 객체 생성
	Shader shader("MyShaders/shadow_mapping_depth.vs", "MyShaders/shadow_mapping_depth.fs");

	// 바닥 평면의 정점 데이터 정적 배열 초기화
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

		 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
		-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
		 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
	};


	/* 바닥 평면의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	// VBO, VAO 객체(object) 참조 id 를 저장할 변수
	unsigned int planeVBO;

	// VAO(Vertex Array Object) 객체 생성
	glGenVertexArrays(1, &planeVAO);

	// VBO(Vertex Buffer Object) 객체 생성
	glGenBuffers(1, &planeVBO);

	// VAO 객체 먼저 컨텍스트에 바인딩(연결)함. 
	// -> 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.
	glBindVertexArray(planeVAO);

	// VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	// 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

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


	/* 텍스쳐 객체 생성 및 쉐이더 프로그램 전송 */

	// 텍스쳐 객체 생성
	unsigned int woodTexture = loadTexture("resources/textures/wood.png");


	/* shadow map 을 생성하기 위한 프레임버퍼 생성 및 설정 */

	// shadow map 해상도 정의
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	// FBO(FrameBufferObject) 객체 생성
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// FBO 객체에 attach 할 텍스쳐 객체(== shadow map 텍스쳐) 생성 및 바인딩
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	// 텍스쳐 객체 메모리 공간 할당 (loadTexture() 와 달리 할당된 메모리에 이미지 데이터를 덮어쓰지 않음! -> 대신 FBO 에서 렌더링된 데이터를 덮어쓸 거니까!)
	// 텍스쳐 객체의 해상도는 shadow map 해상도와 일치시킴 -> 왜냐? 이 텍스쳐는 'shadow map'으로 사용할 거니까!
	// 또한, 이 텍스쳐에는 깊이값만 덮어쓸 것이므로(== 깊이 버퍼 attachment), GL_RGB 가 아닌, 'GL_DEPTH_COMPONENT'으로 텍스쳐 포맷을 지정한다! 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	// Texture Filtering(텍셀 필터링(보간)) 모드 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Texture Wrapping 모드 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 생성했던 FBO 객체 바인딩
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	// FBO 객체에 생성한 텍스쳐 객체 attach (자세한 매개변수 설명은 LearnOpenGL 본문 참고!)
	// shadow map 텍스쳐 객체에는 최종 depth buffer 만 저장하면 되므로, depth attachment 만 적용함!
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	// shadow map 을 위해 생성한 프레임버퍼는 색상 버퍼를 필요로 하지 않음.
	// -> 따라서, 현재 GL_FRAMEBUFFER 에 바인딩된 프레임버퍼가 색상 버퍼를 이용해서 실제 화면에 렌더링하지 않음을 명시하면,
	// 색상 버퍼(== color attachment)에 데이터를 굳이 저장하지 않아도 되도록 설정할 수 있음!
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// 생성한 FBO 객체 설정 완료 후, 다시 default framebuffer 바인딩하여 원상복구 (참고로, default framebuffer 의 참조 id 가 0임!)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// 프래그먼트 쉐이더에 선언된 uniform sampler 변수에 0번 texture unit 위치값 전송
	shader.use();
	shader.setInt("texture1", 0);


	// 광원 위치값 초기화
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

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
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// 색상 버퍼 및 깊이 버퍼 초기화 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		/* 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함. */


		/* 변환행렬 계산 및 쉐이더 객체에 전송 */

		// 변환행렬을 전송할 쉐이더 프로그램 바인딩
		shader.use();

		// 카메라의 zoom 값으로부터 투영 행렬 계산
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 계산된 투영행렬을 쉐이더 프로그램에 전송
		shader.setMat4("projection", projection);

		// 계산된 뷰 행렬을 쉐이더 프로그램에 전송
		shader.setMat4("view", view);

		// 모델행렬 전송 생략 -> 큐브를 변환하지 않음!


		/* 기타 uniform 변수들 쉐이더 객체에 전송 */

		// 카메라 위치값 쉐이더 프로그램에 전송
		shader.setVec3("viewPos", camera.Position);

		// 광원 위치값 쉐이더 프로그램에 전송
		shader.setVec3("lightPos", lightPos);


		/* 바닥 평면 그리기 */

		// 바닥 평면에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(planeVAO);

		// 이 예제에서는 모든 텍스쳐 객체가 0번 texture unit 을 공유할 것이므로, 0번 위치에 텍스쳐 객체가 바인딩되도록 활성화
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);

		// 바닥 평면 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwSwapBuffers(window);

		// 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
		glfwPollEvents();
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

	// while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제
	glfwTerminate();

	return 0;
}


/* 전방선언된 콜백함수 정의 */


/* shadow map 에 깊이 버퍼를 저장할 씬을 렌더링하는 함수 선언 */
void renderScene(const Shader& shader)
{
	/* 바닥 평면 그리기 */

	// 바닥 평면에 적용할 모델행렬 초기화 (단위행렬 사용 -> 변환 x)
	glm::mat4 model = glm::mat4(1.0f);

	// 매개변수로 전달받은 쉐이더 객체에 모델행렬 전송
	shader.setMat4("model", model);

	// 바닥 평면에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
	glBindVertexArray(planeVAO);

	// 바닥 평면 그리기 명령
	glDrawArrays(GL_TRIANGLES, 0, 6);


	/* 첫 번째 큐브 그리기 */

	// 첫 번째 큐브에 적용할 모델행렬 계산
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));

	// 매개변수로 전달받은 쉐이더 객체에 모델행렬 전송
	shader.setMat4("model", model);

	// 큐브 렌더링 함수 실행
	renderCube();


	/* 두 번째 큐브 그리기 */

	// 두 번째 큐브에 적용할 모델행렬 계산
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.5f));

	// 매개변수로 전달받은 쉐이더 객체에 모델행렬 전송
	shader.setMat4("model", model);

	// 큐브 렌더링 함수 실행
	renderCube();


	/* 세 번째 큐브 그리기 */

	// 세 번째 큐브에 적용할 모델행렬 계산
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0f));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
	model = glm::scale(model, glm::vec3(0.25f));

	// 매개변수로 전달받은 쉐이더 객체에 모델행렬 전송
	shader.setMat4("model", model);

	// 큐브 렌더링 함수 실행
	renderCube();
}


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
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// 그리기 명령 종료 후, VAO 객체 바인딩 해제
	glBindVertexArray(0);
}


/* 깊이 버퍼를 시각화할 QuadMesh 를 렌더링하는 함수 구현 */

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