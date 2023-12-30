#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리
#include <algorithm> // std::min(), std::max() 를 사용하기 위해 포함한 라이브러리
#include <cstdlib> // std::rand(), std::srand() 등의 랜덤값 생성 함수 사용 가능

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"
#include "MyHeaders/camera.h"
#include "MyHeaders/model.h"

#include <iostream>

/* 콜백함수 전방선언 */

// GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 

// GLFW 윈도우에 마우스 입력 감지 시, 호출할 콜백함수
void mouse_callback(GLFWwindow* window, double xpos, double ypos); 

// GLFW 윈도우에 스크롤 입력 감지 시, 호출할 콜백함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset); 

// GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언
void processInput(GLFWwindow* window, Shader ourShader); 


/* 윈도우 창 생성 옵션 */ 

// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이


/* 카메라 클래스 생성 */

// 카메라 위치값만 매개변수로 전달
Camera camera(glm::vec3(0.0f, 0.0f, 155.0f));


/* 마우스 입력 관련 전역변수 선언 */

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

	// 새로운 GLFWwindow 윈도우 객체를 만들면, 해당 윈도우의 OpenGL 컨텍스트를 현재 실행중인 스레드의 현재 컨텍스트로 지정
	glfwMakeContextCurrent(window); 

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

	// 텍스쳐 이미지 로드 후, y축 방향으로 뒤집어 줌 > OpenGL 이 텍스쳐 좌표를 읽는 방향과 이미지의 픽셀 좌표가 반대라서!
	stbi_set_flip_vertically_on_load(true);

	// Depth Test(깊이 테스팅) 상태를 활성화함
	glEnable(GL_DEPTH_TEST);


	/* Shader 객체 생성 */

	// asteroid 에 적용할 쉐이더 객체 생성
	Shader asteroidShader("MyShaders/asteroid.vs", "MyShaders/asteroid.fs");

	// planet 에 적용할 쉐이더 객체 생성
	Shader planetShader("MyShaders/planet.vs", "MyShaders/planet.fs"); 


	/* Model 객체 생성 */

	// Model 클래스를 생성함으로써, 생성자 함수에서 Assimp 라이브러리로 즉시 3D 모델을 불러옴
	
	// rock 모델 로딩
	Model rock("resources/models/planet/rock.obj");
	
	// planet 모델 로딩
	Model planet("resources/models/planet/planet.obj");


	/* 각 asteroid 에 적용할 모델행렬 계산 */

	// 전체 asteroid 개수
	unsigned int amount = 100000;

	// 각 모델행렬을 캐싱해 둘 동적 할당 배열 변수 선언
	glm::mat4* modelMatrices;

	// 정적배열 변수에 힙 메모리 동적 할당
	modelMatrices = new glm::mat4[amount];

	// 난수생성 함수 rand() 가 내부적으로 사용할 seed 값을 현재 경과시간으로 지정
	// 시간값을 정적 캐스팅하여 난수 생성 함수의 seed 값으로 생성하는 기법 https://github.com/jooo0922/cpp-study/blob/main/TBCppStudy/Chapter5_09/Chapter5_09.cpp 참고
	srand(static_cast<unsigned int>(glfwGetTime()));

	// planet 에서 asteroid ring 가운데 지점까지의 반경
	float radius = 150.0f;

	// asteroid ring 중점에서 안쪽 및 바깥쪽까지의 폭(간격, offset)
	float offset = 25.0f;

	// 반복문에서 asteroid 개수만큼 모델행렬 계산하여 정적배열에 캐싱
	for (unsigned int i = 0; i < amount; i++)
	{
		// 현재 asteroid 의 모델행렬을 단위행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// xz 평면(== asteroid ring) 상에서 asteroid 가 위치할 원의 각도 계산
		float angle = (float)i / (float)amount * 360.f;

		// asteroid ring 의 가운데 지점으로부터 각 asteroid 를 떨어트릴 간격의 변위값(displacement) 계산
		// -> [-25.0f, 25.0f] 사이의 랜덤값으로 계산됨 (즉, [-offset, offset] 사이의 랜덤값)
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		// 원의 좌표 공식(sinθ * 원의 반지름)에 랜덤한 변위값을 더해 asteroid x좌표값 계산
		float x = sin(angle) * radius + displacement;
		
		// y좌표값 계산을 위해 랜덤한 변위값 재계산
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		// y좌표값(== asteroid ring 높이값)은 항상 변위값(== asteroid ring 폭)의 40% 정도로 설정
		float y = displacement * 0.4f;

		// z좌표값 계산을 위해 랜덤한 변위값 재계산
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;

		// 원의 좌표 공식(cosθ * 원의 반지름)에 랜덤한 변위값을 더해 asteroid z좌표값 계산
		float z = cos(angle) * radius + displacement;

		// 랜덤하게 계산된 x, y, z 좌표값으로 이동행렬 계산
		model = glm::translate(model, glm::vec3(x, y, z));

		// [0.05f, 0.25f] 사이의 랜덤한 scale 값을 구해 크기행렬 계산
		float scale = (rand() % 20) / 100.0f + 0.05f;

		// 회전축 glm::vec3(0.4f, 0.6f, 0.8f) 을 기준으로 [0, 360] 도 사이의 랜덤한 회전각만큼 회전시킴
		// 회전행렬 계산 관련 https://github.com/jooo0922/opengl-study/blob/main/GettingStarted/Coordinate_Systems/coordinate_systems.cpp 참고
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 계산이 완료된 모델행렬을 동적 할당 배열에 순서대로 캐싱
		modelMatrices[i] = model;
	}


	/* Instanced Array 로 전송할 모델행렬 데이터를 덮어쓸 VBO 객체 생성 */
	
	// VBO 객체 참조 id 를 저장할 변수
	unsigned int buffer;

	// VBO(Vertex Buffer Object) 객체 생성
	glGenBuffers(1, &buffer);

	// VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	// Instanced Array 에 전송할 데이터를 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
	// (참고로, 정적 배열을 전송할 때에는, 동적 할당 배열의 첫 번째 요소의 주소값만 전달하면 됨! why? 이 자체가 배열의 주소값과 같으니까!)
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);


	/* Instance 단위로 업데이트할 attribute(== 모델행렬 데이터)의 해석 방식 설정 */

	// mat4 타입 attribute 변수 데이터 해석 방식 관련 하단 필기 참고!
	
	// Mesh 클래스 > VAO 참조변수 encapsulation 해제 관련 하단 필기 참고!

	// rock Model 클래스가 포함하고 있는 모든 mesh 객체들을 순회하며 VAO 객체에 접근 및 설정
	for (unsigned int i = 0; i < rock.meshes.size(); i++)
	{
		// 원래는 캡슐화되어 있던 Mesh 의 VAO 참조변수에 임시 접근
		unsigned int VAO = rock.meshes[i].VAO;

		// 해당 VAO 객체 바인딩
		glBindVertexArray(VAO);

		// 3번 location 변수(aInstanceMatrix)를 사용하도록 활성화 
		// (== mat4 타입 instanced array 변수가 4개의 vec4 로 쪼개졌을 때, 첫 번째 vec4 attribute 변수)
		glEnableVertexAttribArray(3);

		// 인스턴스 단위로 각 정점에 적용할 모델행렬 데이터(쪼개진 4개의 vec4 중, 첫 번째 vec4) 해석 방식 정의
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);

		// 4번 location 변수(aInstanceMatrix)를 사용하도록 활성화 
		// (== mat4 타입 instanced array 변수가 4개의 vec4 로 쪼개졌을 때, 두 번째 vec4 attribute 변수)
		glEnableVertexAttribArray(4);

		// 인스턴스 단위로 각 정점에 적용할 모델행렬 데이터(쪼개진 4개의 vec4 중, 두 번째 vec4) 해석 방식 정의
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));

		// 5번 location 변수(aInstanceMatrix)를 사용하도록 활성화 
		// (== mat4 타입 instanced array 변수가 4개의 vec4 로 쪼개졌을 때, 세 번째 vec4 attribute 변수)
		glEnableVertexAttribArray(5);

		// 인스턴스 단위로 각 정점에 적용할 모델행렬 데이터(쪼개진 4개의 vec4 중, 세 번째 vec4) 해석 방식 정의
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));

		// 6번 location 변수(aInstanceMatrix)를 사용하도록 활성화 
		// (== mat4 타입 instanced array 변수가 4개의 vec4 로 쪼개졌을 때, 네 번째 vec4 attribute 변수)
		glEnableVertexAttribArray(6);

		// 인스턴스 단위로 각 정점에 적용할 모델행렬 데이터(쪼개진 4개의 vec4 중, 네 번째 vec4) 해석 방식 정의
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		// instanced array 로 사용될 쪼개진 4개의 3, 4, 5, 6번 location 변수의 업데이트 단위를 vertex -> instance 로 변경
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		// 데이터 해석 방식 설정 완료 후, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
		glBindVertexArray(0);
	}


	/* while 문으로 렌더링 루프 구현 */

	while (!glfwWindowShouldClose(window))
	{
		/* 카메라 이동속도 보정을 위한 deltaTime 계산 */

		// 현재 프레임 경과시간
		float currentFrame = static_cast<float>(glfwGetTime()); 

		// 현재 프레임 경과시간 - 마지막 프레임 경과시간 = 두 프레임 사이의 시간 간격
		deltaTime = currentFrame - lastFrame; 

		// 마지막 프레임 경과시간을 현재 프레임 경과시간으로 업데이트!
		lastFrame = currentFrame; 


		/* 윈도우 창 및 키 입력 감지 밎 이벤트 처리 */
		processInput(window, planetShader); 


		/* 버퍼 초기화 */

		// 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼 및 깊이 버퍼 초기화
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		/* 공통 변환행렬(projection, view) 계산 */

		// 투영행렬 계산 (fov 는 45도로 고정 -> zoom 값도 고정)
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

		// 카메라 클래스로부터 뷰 행렬(= LookAt 행렬) 가져오기
		glm::mat4 view = camera.GetViewMatrix();


		/* planet 그리기 */

		// Assimp 로 업로드한 3D 모델에 적용할 쉐이더 프로그램 객체 바인딩
		planetShader.use();

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 투영 행렬 전송
		planetShader.setMat4("projection", projection);

		// 현재 바인딩된 쉐이더 프로그램의 uniform 변수에 mat4 뷰 행렬 전송
		planetShader.setMat4("view", view); 

		// planet 에 적용할 모델행렬 계산

		// 모델 행렬을 단위행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f); 
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));

		// 최종 계산된 모델 행렬을 바인딩된 쉐이더 프로그램의 유니폼 변수로 전송
		planetShader.setMat4("model", model);

		// Model 클래스의 Draw 멤버함수 호출 > 해당 Model 에 포함된 모든 Mesh 인스턴스의 Draw 멤버함수를 호출함
		planet.Draw(planetShader);


		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	glfwTerminate(); // while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제

	return 0;
}

/* 전방선언된 콜백함수 정의 */

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
	mat4 타입으로 선언된 attribute 변수는
	다른 타입의 attribute 변수와 처리 방식이 다름.

	버텍스 쉐이더에 선언된 attribute 변수에
	데이터를 저장할 수 있는 최대치가 vec4 타입의 크기와 같으므로,

	mat4 타입의 attribute 변수는
	vec4 타입의 attribute 변수를 4개 할당한 것으로 보면 됨.

	따라서, mat4 타입의 attribute 변수에 전송되는 데이터의 해석 방식을 설정할 때에도,
	vec4 타입의 데이터를 4번씩 나눠서 해석 방식을 지정해줘야 함.

	why? 위에서 말했던 것처럼, mat4 타입의 attribute 변수는
	vec4 타입의 attribute 변수 4개로 쪼개져서 저장되어 있는 형태니까!

	그래서, glVertexAttribPointer() 로 데이터 해석 방식을 정의할 때,
	만약 mat4 attribute 변수가 3번 location 에 지정되어 있다면,

	3번부터 시작해서 3, 4, 5, 6번 location 에 4개의 vec4 타입 크기만큼
	attribute 변수가 쪼개져있는 형태로 저장되어 있다고 생각하고,
	
	4개의 location 변수들에 대해 각각 데이터 해석 방식을 나눠서 정의해줘야 함!

	단, 주의할 점이,
	glVertexAttribPointer() 의 5번째 매개변수인 stride (== 정점 데이터 간격) 의 경우,
	여전히 mat4 타입의 크기만큼이 한 묶음의 정점 데이터인 것으로 인식될 수 있도록,
	sizeof(glm::vec4) 가 아닌, 'sizeof(glm::mat4)' 로 지정해줘야 함!
*/

/*
	원래 assimp 로 업로드한 모델의 경우,
	VAO, VBO 객체들을 추상화된 Mesh 클래스에서 캡슐화하여 관리하지만,

	instanced array 에 전송할
	mat4 타입 모델행렬 데이터들의 VBO 정보 및 해석 방식을
	해당 모델의 VAO 객체에 설정해야 하기 때문에

	임시로 VAO 참조변수만 encapsulation 을 해제한 뒤,
	외부에서 접근할 수 있도록 허용한 것. -> 일종의 cheating

	사실 가장 좋은 것은 Mesh 클래스 내부에
	Instanced array 를 처리하는 코드까지 추상화 해주는 게 맞겠지
*/
