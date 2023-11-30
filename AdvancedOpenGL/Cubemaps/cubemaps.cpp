#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리
#include <algorithm> // std::min(), std::max() 를 사용하기 위해 포함한 라이브러리

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"
#include "MyHeaders/camera.h"
#include "MyHeaders/model.h"

#include <iostream>

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void processInput(GLFWwindow* window, Shader ourShader); // GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언
unsigned int loadCubemap(std::vector<std::string> faces); // 큐브맵 텍스쳐 로드 함수 선언

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

	// 텍스쳐 이미지 로드 후, y축 방향으로 뒤집어 줌 > OpenGL 이 텍스쳐 좌표를 읽는 방향과 이미지의 픽셀 좌표가 반대라서!
	stbi_set_flip_vertically_on_load(true);

	// 각 프래그먼트에 대한 깊이 버퍼(z-buffer)를 새로운 프래그먼트의 깊이값과 비교해서
	// 프래그먼트 값을 덮어쓸 지 말 지를 결정하는 Depth Test(깊이 테스팅) 상태를 활성화함
	// OpenGL 컨텍스트의 state-setting 함수 중 하나겠지! 
	glEnable(GL_DEPTH_TEST);

	// Shader 클래스를 생성함으로써, 쉐이더 객체 / 프로그램 객체 생성 및 컴파일 / 링킹
	//Shader ourShader("MyShaders/cubemaps.vs", "MyShaders/cubemaps.fs"); // 로드한 3D 모델에 텍스쳐를 적용하는 Shader 객체 생성
	Shader ourShader("MyShaders/cubemaps.vs", "MyShaders/cubemaps_refraction.fs"); // 큐브맵 굴절 재질을 계산하는 프래그먼트 쉐이더 적용

	// skybox 에 사용할 Shader 클래스 생성
	Shader skyboxShader("MyShaders/skybox.vs", "MyShaders/skybox.fs");

	// Model 클래스를 생성함으로써, 생성자 함수에서 Assimp 라이브러리로 즉시 3D 모델을 불러옴
	Model ourModel("resources/models/backpack/backpack.obj");

	// skybox 정점 데이터를 정적 배열로 선언 및 초기화
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};


	/* skybox VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩 */
	
	unsigned int skyboxVAO, skyboxVBO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &skyboxVAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &skyboxVBO); // VBO(Vertex Buffer Object) 객체 생성 > VBO 객체를 만들어서 정점 데이터를 GPU 로 전송해야 한 번에 많은 양의 정점 데이터를 전송할 수 있음!

	glBindVertexArray(skyboxVAO);  // VAO 객체 먼저 컨텍스트에 바인딩(연결)함. > 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO); // VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.

	// VBO 객체 설정
	// 정점 위치 데이터 해석 방식 설정
	glEnableVertexAttribArray(0); // 버텍스 쉐이더의 0번 location 변수만 사용하도록 활성화한 것!
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// skyboxVAO 객체에 저장해 줄 설정들을 끝마쳤으므로, 컨텍스트에서 바인딩 해제
	glBindVertexArray(0);


	/* 큐브맵 텍스쳐 로드 및 생성 */

	// 큐브맵 이미지는 y축 방향을 뒤집어서 로드하면 오히려 거꾸로 렌더링됨. -> 이미지 y축 방향을 뒤집지 않도록 설정!
	stbi_set_flip_vertically_on_load(false);

	// 큐브맵의 각 텍스쳐 이미지 url 을 Texture target Enum 방향 순서에 맞게 동적 배열(vector)에 초기화 
	std::vector<std::string> faces
	{
		"resources/textures/skybox/right.jpg",
		"resources/textures/skybox/left.jpg",
		"resources/textures/skybox/top.jpg",
		"resources/textures/skybox/bottom.jpg",
		"resources/textures/skybox/front.jpg",
		"resources/textures/skybox/back.jpg",
	};

	// 큐브맵 텍스쳐 로드 및 텍스쳐 객체 참조 ID 반환
	unsigned int cubemapTexture = loadCubemap(faces);

	
	/* skybox 쉐이더 객체 설정 */

	// skybox 쉐이더 프로그램 바인딩
	skyboxShader.use();

	// skybox 쉐이더에 큐브맵 텍스쳐 객체를 바인딩할 texture unit 위치값 전송
	skyboxShader.setInt("skybox", 0);


	/* ourShader 쉐이더 객체 설정 */

	// ourShader 쉐이더 프로그램 바인딩
	ourShader.use();

	// ourShader 쉐이더에 큐브맵 텍스쳐 객체를 바인딩할 texture unit 위치값 전송 (모든 텍스쳐는 0번 unit 사용 통일)
	ourShader.setInt("skybox", 0);


	// while 문으로 렌더링 루프 구현
	// glfwWindowShouldClose(GLFWwindow* window) 로 현재 루프 시작 전, GLFWwindow 를 종료하라는 명령이 있었는지 검사.
	while (!glfwWindowShouldClose(window))
	{
		// 카메라 이동속도 보정을 위한 deltaTime 계산
		float currentFrame = static_cast<float>(glfwGetTime()); // 현재 프레임 경과시간
		deltaTime = currentFrame - lastFrame; // 현재 프레임 경과시간 - 마지막 프레임 경과시간 = 두 프레임 사이의 시간 간격
		lastFrame = currentFrame; // 마지막 프레임 경과시간을 현재 프레임 경과시간으로 업데이트!

		processInput(window, ourShader); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		// 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼를 초기화함. 
		// glEnable() 로 깊이 테스팅을 활성화한 상태이므로, 이전 프레임에 그렸던 깊이 버퍼도 초기화하도록,
		// 깊이 버퍼 제거를 명시하는 비트 단위 연산을 수행하여 비트 플래그 설정 (state-using)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		/* Assimp 로 업로드한 3D 모델 그리기 */

		// Assimp 로 업로드한 3D 모델에 적용할 쉐이더 프로그램 객체 바인딩
		ourShader.use();

		// 카메라 줌 효과를 구현하기 위해 fov 값을 실시간으로 변경해야 하므로,
		// fov 값으로 계산되는 투영행렬을 런타임에 매번 다시 계산해서 쉐이더 프로그램으로 전송해줘야 함. > 게임 루프에서 계산 및 전송
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); // 투영 행렬 생성

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		ourShader.setMat4("projection", projection); // 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송
		ourShader.setMat4("view", view); // 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송

		glm::mat4 model = glm::mat4(1.0f); // 모델 행렬을 단위행렬로 초기화
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 모델을 원점으로 이동시키는 이동행렬 적용
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // 모델의 크기를 조정하는 크기행렬 적용
		ourShader.setMat4("model", model); // 최종 계산된 모델 행렬을 바인딩된 쉐이더 프로그램의 유니폼 변수로 전송
		ourShader.setVec3("cameraPos", camera.Position); // 쉐이더 내부에서 뷰 벡터 계산을 위해 카메라 위치 전송

		// Model 클래스의 Draw 멤버함수 호출 > 해당 Model 에 포함된 모든 Mesh 인스턴스의 Draw 멤버함수를 호출함
		ourModel.Draw(ourShader);


		/* skybox 그리기 */

		// skybox 렌더링 최적화를 위해 skybox 그리기 명령은 가장 마지막에 호출한다. (skybox.vs 관련 필기 참고)

		// 깊이테스트 함수 변경 (깊이 테스트 함수 변경 관련 필기 참고)
		glDepthFunc(GL_LEQUAL);

		// skybox 쉐이더 프로그램 바인딩
		skyboxShader.use();

		// 카메라 뷰 행렬에서 이동행렬 컴포넌트를 제외한 좌상단 3*3 행렬만 쉐이더로 전달함 
		// (https://learnopengl.com/Lighting/Basic-Lighting 참고. 노말행렬 계산 시, 모델행렬에서 이동행렬 파트만 제외한 것과 동일한 원리!)
		// why? 카메라가 '이동'하더라도 skybox 마저도 카메라 반대방향으로 이동하면 안되니까! -> skybox 는 카메라가 움직이더라도 배경이니까 가만히 있어야 함!
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송
		skyboxShader.setMat4("view", view);

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송
		skyboxShader.setMat4("projection", projection);
		
		// skybox 에 적용할 VAO 객체 바인딩
		glBindVertexArray(skyboxVAO);

		// 이 예제에서는 모든 텍스쳐 객체가 0번 texture unit 을 공유할 것이므로, 0번 위치에 텍스쳐 객체가 바인딩되도록 활성화
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

		// skybox 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// skyboxVAO 객체 바인딩 해제
		glBindVertexArray(0);

		// 깊이테스트 함수를 기본값으로 원상복구
		glDepthFunc(GL_LESS);


		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate(); // while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제

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

// 큐브맵 텍스쳐 로드 함수 (주의할 점 하단 필기 참고)
unsigned int loadCubemap(std::vector<std::string> faces)
{
	// 텍스쳐 객체(object) 참조 id 를 저장할 변수 선언
	unsigned int textureID; 
	
	// 텍스쳐 객체 생성
	glGenTextures(1, &textureID);

	// 생성한 텍스쳐 객체 바인딩
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	// 로드한 이미지의 width, height, 색상 채널 개수를 저장할 변수 선언
	int width, height, nrChannels;

	// 반복문을 순회하며 6개의 텍스쳐 이미지 로드
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// 이미지 데이터 가져와서 char 타입의 bytes 데이터로 저장. 
		// 이미지 width, height, 색상 채널 변수의 주소값도 넘겨줌으로써, 해당 함수 내부에서 값을 변경. -> 출력변수 역할
		// std::string 컨테이너를 그대로 전달할 수 없으므로, c-style 문자열(char*) 타입으로 변환하여 url 전달 (.c_str())
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

		if (data)
		{
			// 이미지 데이터 로드 성공 시 처리

			// 로드한 이미지 데이터를 현재 바인딩된 텍스쳐 객체에 덮어쓰기
			// Texture Target Enum 에 대응되는 정수값을 반복문 index 를 더해가며 순서대로 계산
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			// 이미지 데이터 로드 실패 시 처리
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
		}

		// 텍스쳐 객체에 이미지 데이터를 전달하고, 밉맵까지 생성 완료했다면, 로드한 이미지 데이터는 항상 메모리 해제할 것!
		stbi_image_free(data);
	}

	// 현재 GL_TEXTURE_CUBE_MAP 상태에 바인딩된 텍스쳐 객체 설정하기
	// 텍스쳐 축소/확대 및 Mipmap 교체 시 Texture Filtering (텍셀 필터링(보간)) 모드 설정 (관련 필기 정리 하단 참고)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Texture Wrapping 모드를 반복 모드로 설정 ([(0, 0), (1, 1)] 범위를 벗어나는 텍스쳐 좌표에 대한 처리)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // 텍스쳐의 3차원 좌표(z값)에 대한 반복 모드 설정(-> 큐브맵 텍스쳐니까!)

	// 텍스쳐 객체 참조 ID 반환
	return textureID;
}

/*
	큐브맵 텍스쳐 로드 함수는
	일반 텍스쳐 로드 함수와 크게 다르지는 않음.

	다만, 큐브맵 텍스쳐는 6개의 텍스쳐 이미지를
	반복문을 순회하면서 로드해야 한다는 점.

	또한, 6개 방향의 텍스쳐 이미지의
	Texture target Enum 순서에 따라 차례대로 로드할 수 있도록
	이미지 url 가 담긴 vector(동적배열)을 6개 방향의 순서에 따라 정렬하여
	매개변수로 전달해줘야 한다는 점을 주의해야 한다!

	6개 방향의 Texture target Enum 순서는 아래와 같이 정렬되도록 할 것! 
	
	Texture target Enum	    int 
	+X (right)				0
	-X (left)				1
	+Y (top)				2
	-Y (bottom)				3
	+Z (front)				4
	-Z (back)				5
*/

/*
	skybox 렌더링 시, 깊이 테스트 함수를 GL_LEQUAL 로 설정하는 이유?


	skybox.vs 파일에서 관련 필기를 참고해보면,
	skybox 는 렌더링 최적화를 위해 깊이값이 모두 1 로 계산될 것임.
	
	그런데, depth buffer 의 초기값 또한 항상 1 로 설정되기 때문에, 
	깊이테스트 기본 모드인 GL_LESS 로 수행하면,

	skybox 의 깊이값과 depth buffer 의 초기값이 동일하여
	skybox 의 모든 프래그먼트들이 discard 되어버리는 문제가 발생함.

	이를 방지하기 위해,
	깊이 테스트 모드를 GL_LEQUAL 로 설정함으로써,
	현재 depth buffer 의 값보다 '작거나 같으면'
	깊이 테스트를 통과할 수 있도록 변경한 것임!
*/