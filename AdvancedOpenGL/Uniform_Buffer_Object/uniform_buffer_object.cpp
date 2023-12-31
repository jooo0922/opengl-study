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

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos); // GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); // GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void processInput(GLFWwindow* window, Shader ourShader); // GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언

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
	glEnable(GL_DEPTH_TEST);

	// 각 큐브마다 서로 다른 색상을 출력하는 4개의 쉐이더 객체 생성
	Shader shaderRed("MyShaders/uniform_buffer_object.vs", "MyShaders/red.fs");
	Shader shaderGreen("MyShaders/uniform_buffer_object.vs", "MyShaders/green.fs");
	Shader shaderBlue("MyShaders/uniform_buffer_object.vs", "MyShaders/blue.fs");
	Shader shaderYellow("MyShaders/uniform_buffer_object.vs", "MyShaders/yellow.fs");

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


	/* Uniform Buffer Object (이하 'UBO') 사용하기 (UBO 관련 하단 필기 참고) */

	/* Uniform block 바인딩 (UBO 와 Uniform blocks 관계 하단 필기 참고) */

	// 각 큐브마다 적용될 쉐이더 객체의 Uniform block index 가져오기
	unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(shaderRed.ID, "Matrices");
	unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(shaderGreen.ID, "Matrices");
	unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(shaderBlue.ID, "Matrices");
	unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(shaderYellow.ID, "Matrices");

	// 각 쉐이더 프로그램의 Uniform block 들을 모두 0번 binding point 에 연결
	// 추후 UBO 객체도 0번 binding point 에 바인딩 할 것임!
	glUniformBlockBinding(shaderRed.ID, uniformBlockIndexRed, 0);
	glUniformBlockBinding(shaderGreen.ID, uniformBlockIndexGreen, 0);
	glUniformBlockBinding(shaderBlue.ID, uniformBlockIndexBlue, 0);
	glUniformBlockBinding(shaderYellow.ID, uniformBlockIndexYellow, 0);

	
	/* UBO 객체 생성 및 설정 */
	
	// UBO 객체 참조 ID 가 저장될 변수 선언
	unsigned int uboMatrices;

	// UBO 객체 생성
	glGenBuffers(1, &uboMatrices);

	// GL_UNIFORM_BUFFER 상태에 바인딩
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	// 저장할 데이터 크기(mat4 행렬 2개)만큼 UBO 객체에 메모리 할당
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);

	// UBO 객체 바인딩 해제
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// UBO 객체를 0번 binding point 에 바인딩
	// -> 0번 offset 부터 mat4 행렬 2개의 byte 크기까지의 범위만큼 (== 그냥 UBO 메모리 전체 사이즈만큼) 바인딩
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));


	/* projection 행렬 데이터를 UBO 객체에 쓰기 */
	
	// 사실 projection 행렬도 렌더링 루프 안에서 계산해주는 게 더 좋지만,
	// 일단 fov 값을 변경하지 않고, zoom-in/out 동작을 사용하지 않는다는 가정하에
	// 자주 변경될 값은 없으므로, 렌더링 루프 진입 전에 UBO 객체에 저장함!
	
	// 투영 행렬 생성
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f); 

	// 투영행렬을 덮어쓸 UBO 객체 바인딩
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

	// 0번 offset 지점부터 mat4 타입의 byte size 까지 투영행렬 데이터 덮어쓰기 (glm::value_ptr() 함수 설명 하단 참고)
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));

	// 투영행렬을 UBO 객체에 덮어쓰기르 완료했다면 객체 바인딩 해제
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	// while 문으로 렌더링 루프 구현
	while (!glfwWindowShouldClose(window))
	{
		// 카메라 이동속도 보정을 위한 deltaTime 계산
		float currentFrame = static_cast<float>(glfwGetTime()); // 현재 프레임 경과시간
		deltaTime = currentFrame - lastFrame; // 현재 프레임 경과시간 - 마지막 프레임 경과시간 = 두 프레임 사이의 시간 간격
		lastFrame = currentFrame; // 마지막 프레임 경과시간을 현재 프레임 경과시간으로 업데이트!

		processInput(window, shaderRed); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// 색상 버퍼 및 깊이 버퍼 초기화 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		/* view 행렬 데이터를 UBO 객체에 쓰기(업데이트) */

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();

		// 뷰 행렬을 덮어쓸 UBO 객체 바인딩
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);

		// 첫 번째 mat4 타입의 byte 까지 저장된 (== projection 행렬이 저장된) 지점부터 
		// mat4 타입의 byte 지점까지 투영행렬 데이터 덮어쓰기
		// -> 항상 std140 표준 메모리 레이아웃 기준 offset 값은 '저장될 데이터 크기의 배수' 로 지정해줄 것!
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		// 뷰 행렬을 UBO 객체에 덮어쓰기르 완료했다면 객체 바인딩 해제
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		/* 빨간색 큐브 그리기 */

		// 큐브에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		glBindVertexArray(cubeVAO);

		// 빨간색 큐브에 적용할 쉐이더 프로그램 객체 바인딩
		shaderRed.use(); 

		// 모델 행렬을 단위행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// 빨간색 큐브에 적용할 모델행렬 계산
		model = glm::translate(model, glm::vec3(-0.75f, 0.75f, 0.0f));

		// 빨간색 큐브에 적용할 모델행렬을 uniform 변수로 전송 (model 행렬은 큐브마다 다르니 UBO 에 저장해서 쓰기에 부적합)
		shaderRed.setMat4("model", model);

		// 빨간색 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36); 


		/* 초록색 큐브 그리기 */

		// 초록색 큐브에 적용할 쉐이더 프로그램 객체 바인딩
		shaderGreen.use();

		// 모델 행렬을 단위행렬로 초기화
		model = glm::mat4(1.0f);

		// 초록색 큐브에 적용할 모델행렬 계산
		model = glm::translate(model, glm::vec3(0.75f, 0.75f, 0.0f));

		// 초록색 큐브에 적용할 모델행렬을 uniform 변수로 전송 (model 행렬은 큐브마다 다르니 UBO 에 저장해서 쓰기에 부적합)
		shaderGreen.setMat4("model", model);

		// 초록색 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* 노란색 큐브 그리기 */

		// 노란색 큐브에 적용할 쉐이더 프로그램 객체 바인딩
		shaderYellow.use();

		// 모델 행렬을 단위행렬로 초기화
		model = glm::mat4(1.0f);

		// 노란색 큐브에 적용할 모델행렬 계산
		model = glm::translate(model, glm::vec3(-0.75f, -0.75f, 0.0f));

		// 노란색 큐브에 적용할 모델행렬을 uniform 변수로 전송 (model 행렬은 큐브마다 다르니 UBO 에 저장해서 쓰기에 부적합)
		shaderYellow.setMat4("model", model);

		// 노란색 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


		/* 파란색 큐브 그리기 */

		// 파란색 큐브에 적용할 쉐이더 프로그램 객체 바인딩
		shaderBlue.use();

		// 모델 행렬을 단위행렬로 초기화
		model = glm::mat4(1.0f);

		// 파란색 큐브에 적용할 모델행렬 계산
		model = glm::translate(model, glm::vec3(0.75f, -0.75f, 0.0f));

		// 파란색 큐브에 적용할 모델행렬을 uniform 변수로 전송 (model 행렬은 큐브마다 다르니 UBO 에 저장해서 쓰기에 부적합)
		shaderBlue.setMat4("model", model);

		// 파란색 큐브 그리기 명령
		glDrawArrays(GL_TRIANGLES, 0, 36);


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
	Uniform Buffer Object (이하 'UBO') 란 무엇인가?


	사실 UBO 는 LearnOpenGL 본문에서 개념을 가장 잘 설명해주고 있지만,
	간단하게 정리하면 다음과 같음.

	우리가 쉐이더에 glUniform~() 이런 함수를 사용해서 
	uniform 변수에 값을 전송하게 되는데,
	(우리는 shader_s.h 에 클래스로 이 작업을 추상화 했었지?)

	각 쉐이더 프로그램마다 동일한 uniform 값을
	반복적으로 전송해야 하는 상황이 있음.

	예를 들어, 변환행렬 중 view, projection 행렬은
	사실 카메라의 움직임 및 투영과 관련된 변환이기 때문에
	일반적으로 scene 안의 모든 오브젝트들에 동일하게 적용됨.

	따라서, 그 오브젝트들이 바인딩하여 사용하고 있는
	모든 쉐이더 프로그램들에 매번 똑같은 view, projection 행렬을
	반복해서 전송해왔었지.


	차라리 이렇게 할거면,
	이러한 반복적으로 사용되는 uniform 데이터들은
	fixed GPU 메모리 영역에 한 번만 올려두고,

	마치 global(전역변수) 처럼 모든 쉐이더 프로그램들이 
	해당 global uniform 변수를 가져다가 사용하는 방식이
	더 효율적이겠지?

	이럴 때, global 하게 사용할 uniform 변수들을
	저장할 수 있게 해주는 OpenGL 버퍼 객체가 UBO 인 것임!
*/

/*
	Uniform Buffer Object(UBO) 와 Uniform block 의 관계


	Uniform block 에 대한 설명은
	버텍스 쉐이더에 정리해놨으니 참고하면 됨.

	일단, 각 Uniform block 들이
	UBO 에 쓰여진 데이터를 가져다가 쓰려면
	둘을 서로 연결(linking) 시켜줘야 함.

	
	이때, OpenGL Context 에서 지원하는
	'binding point' 라는 개념이 등장함.

	Uniform Buffer Object 를 하나만 사용하지는 않기 때문에
	여러 개의 UBO 객체를 만들어서 binding 해야하는 상황이 생길 수 있음.

	이럴 경우를 대비해서,
	GL_UNIFORM_BUFFER 상태에 UBO 객체를 바인딩할 때,
	"몇 번 위치에 바인딩할 것이냐" 를 설정해서
	각 UBO 객체들이 바인딩된 위치를 구분할 수 있도록 해준 index 값이
	'binding point' 라고 보면 됨.

	그렇다면, 이쯤 되면
	결국 어떤 Uniform block 이 특정 UBO 객체에 
	쓰여진 데이터를 사용하고자 한다면,

	해당 UBO 객체가 바인딩된 binding point 에
	Uniform block 도 똑같이 바인딩해주면 된다는 게 감이 오지?

	이때 사용하는 OpenGL 함수가
	glUniformBlockBinding() 이라고 보면 됨.

	또한, 하나의 쉐이더 프로그램에서도
	여러 개의 Uniform block 을 선언할 수 있기 때문에,
	
	"몇 번째 Uniform block 을 바인딩 할거냐" 를 알려주는 index 값을 
	glGetUniformBlockIndex() 함수로 얻어와서
	하나의 쉐이더 안에서 선언된 각 Uniform block 을 구분할 수 있도록 해줌! 
*/

/*
	glm::value_ptr()


	glBufferSubData() 로 행렬 데이터를
	UBO 객체에 덮어쓸 때,

	마지막 인자 전달 시, glm::value_ptr() 함수를 사용했었지?
	
	얘는 glm 라이브러리에서 취급하는 
	데이터 타입을 가리키는 포인터 함수를 반환함.

	예를 들어, glm::value_ptr(glm::mat4 타입의 변수)
	이런 식으로 glm::mat4 타입의 변수를 매개변수로 넘겨줬다면,
	glm::mat4 타입 데이터의 첫 번째 요소가 저장된 메모리 공간을
	가리키는 주소값인 glm::mat4* 를 반환하는 것이지!

	이게 필요한 이유가 뭐냐면,
	glBufferSubData() 같은 OpenGL 함수들을 통해
	행렬이나 벡터같은 glm의 수학적 데이터 타입을 전달하려면
	
	해당 glm 데이터 타입을 가리키는 포인터 형태로
	전달해줘야 하기 때문!
*/