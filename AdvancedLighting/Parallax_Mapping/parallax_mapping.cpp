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

// normal map 을 적용할 QuadMesh 를 렌더링하는 함수 선언
void renderQuad();


// 윈도우 창 생성 옵션
// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이

// parallax mapping 에서 샘플링된 전체 깊이값을 조절하는 scale 값 초기화
float heightScale = 0.1f;

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
	Shader shader("MyShaders/parallax_mapping.vs", "MyShaders/parallax_mapping.fs");


	/* 텍스쳐 객체 생성 및 쉐이더 프로그램 전송 */

	// 텍스쳐 객체 생성
	unsigned int diffuseMap = loadTexture("resources/textures/bricks2.jpg");
	unsigned int normalMap = loadTexture("resources/textures/bricks2_normal.jpg");
	unsigned int heightMap = loadTexture("resources/textures/bricks2_disp.jpg");

	//unsigned int diffuseMap = loadTexture("resources/textures/toy_box_diffuse.png");
	//unsigned int normalMap = loadTexture("resources/textures/toy_box_normal.png");
	//unsigned int heightMap = loadTexture("resources/textures/toy_box_disp.png");


	/*
		프래그먼트 쉐이더에 선언된 각 uniform sampler 변수에서
		diffuseMap 은 0번 texture unit 위치값을,
		normalMap 은 1번 texture unit 위치값을,
		heightMap (정확히는 displacementMap 으로 사용될 depthMap)은 2번 texture unit 위치값을 전송
	*/
	shader.use();
	shader.setInt("diffuseMap", 0);
	shader.setInt("normalMap", 1);
	shader.setInt("depthMap", 2);


	// 광원 위치값 초기화
	glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

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


		/* QuadMesh 를 회전시키는 모델행렬 계산 및 전송 */

		// 모델행렬을 단위 행렬로 초기화
		glm::mat4 model = glm::mat4(1.0f);

		// 경과시간 흐름에 따라 x축, z축으로 각각 회전시키는 회전행렬 적용
		// -> QuadMesh 의 방향벡터가 회전에 의해 매 프레임마다 변경되더라도, normal mapping 이 정상적으로 적용되는지 테스트하기 위함
		model = glm::rotate(model, glm::radians((float)glfwGetTime() * -10.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)));

		// 계산된 모델행렬을 쉐이더 프로그램에 전송
		shader.setMat4("model", model);


		/* 기타 uniform 변수들 쉐이더 객체에 전송 */

		// 카메라 위치값 쉐이더 프로그램에 전송
		shader.setVec3("viewPos", camera.Position);

		// 광원 위치값 쉐이더 프로그램에 전송
		shader.setVec3("lightPos", lightPos);

		// heightScale 값 쉐이더 프로그램에 전송
		shader.setFloat("heightScale", heightScale);

		// 현재 heightScale 값을 콘솔에 출력
		std::cout << heightScale << std::endl;


		/* 텍스쳐 객체를 활성화된 각 texture unit 위치에 바인딩 */

		// diffuseMap 텍스쳐 객체를 0번 texture unit 위치에 바인딩
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		// normalMap 텍스쳐 객체를 1번 texture unit 위치에 바인딩
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		// depthMap 텍스쳐 객체를 2번 texture unit 위치에 바인딩
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, heightMap);


		// QuadMesh 렌더링 함수 호출
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

/* normal map 을 적용할 QuadMesh 를 렌더링하는 함수 구현 */

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
		// QuadMesh 의 4개 정점의 위치값 선언 및 초기화
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);

		// QuadMesh 의 4개 정점의 uv 좌표값 선언 및 초기화
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);

		// QuadMesh 의 전체 방향벡터 선언 및 초기화
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// QuadMesh 의 두 삼각형 각각의 탄젠트 벡터와 비탄젠트 벡터 변수 선언
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;


		/* 첫 번째 삼각형이 local 인 탄젠트 공간의 탄젠트 벡터와 비탄젠트 벡터 계산 */

		// 첫 번째 삼각형의 Edge 벡터 E₁, E₂ 계산
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;

		// 첫 번째 삼각형의 uv 좌표 변화량 (ΔU₁, ΔV₁), (ΔU₂, ΔV₂) 계산
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		// 첫 번째 삼각형의 uv 변화량으로 구성된 행렬(LearnOpenGL 본문 참고)의 
		// 행렬식(determinant) 의 역수 (Fractional Part) 계산
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		// 첫 번째 삼각형의 탄젠트 공간의 탄젠트 벡터 계산
		// (LearnOpenGL 본문의 행렬 곱셈식 코드로 전개)
		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		// 첫 번째 삼각형의 탄젠트 공간의 비탄젠트 벡터 계산
		// (LearnOpenGL 본문의 행렬 곱셈식 코드로 전개)
		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


		/* 두 번째 삼각형이 local 인 탄젠트 공간의 탄젠트 벡터와 비탄젠트 벡터 계산 */

		// 두 번째 삼각형의 Edge 벡터 E₁, E₂ 계산
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;

		// 두 번째 삼각형의 uv 좌표 변화량 (ΔU₁, ΔV₁), (ΔU₂, ΔV₂) 계산
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		// 두 번째 삼각형의 uv 변화량으로 구성된 행렬(LearnOpenGL 본문 참고)의 
		// 행렬식(determinant) 의 역수 (Fractional Part) 계산
		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		// 두 번째 삼각형의 탄젠트 공간의 탄젠트 벡터 계산
		// (LearnOpenGL 본문의 행렬 곱셈식 코드로 전개)
		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		// 두 번째 삼각형의 탄젠트 공간의 비탄젠트 벡터 계산
		// (LearnOpenGL 본문의 행렬 곱셈식 코드로 전개)
		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


		/* QuadMesh 의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩 */

		// QuadMesh 의 정점 데이터 정적 배열 초기화 (위에서 계산한 tangent, bitangent 벡터 추가)
		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		// 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. 
		// -> 그 중에서 0번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(0);

		// 정점 위치 데이터(0번 location 입력변수 in vec3 aPos 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);

		// 1번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(1);

		// 정점 normal 데이터(1번 location 입력변수 in vec3 aNormal 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));

		// 2번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(2);

		// 정점 UV 데이터(2번 location 입력변수 in vec2 aTexCoords 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));

		// 3번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(3);

		// 정점 tangent 데이터(3번 location 입력변수 in vec3 aTangent 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));

		// 4번 location 변수를 사용하도록 활성화
		glEnableVertexAttribArray(4);

		// 정점 bitangent 데이터(4번 location 입력변수 in vec3 aBitangent 에 전달할 데이터) 해석 방식 정의
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

		// VBO 객체 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// 마찬가지로, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
		glBindVertexArray(0);
	}

	/* QuadMesh 그리기 */

	// QuadMesh 에 적용할 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
	glBindVertexArray(quadVAO);

	// QuadMesh 그리기 명령
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

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

	/*
		Q 키 입력 시, heightScale 값을 감소시키고,
		E 키 입력 시, heightScale 값을 증가시킴.
	*/
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		// heightScale 이 0 보다 작아지지 않도록 감소시킴
		if (heightScale > 0.0f)
		{
			heightScale -= 0.0005f;
		}
		else
		{
			heightScale = 0.0f;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		// heightScale 이 1 보다 커지지 않도록 증가시킴
		if (heightScale < 1.0f)
		{
			heightScale += 0.0005f;
		}
		else
		{
			heightScale = 1.0f;
		}

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
		/*
			.png 같은 RGBA 포맷일 경우, 반투명한 texel 이 존재함.

			텍스쳐 좌표가 보간(interpolation)되어
			정수값이 아닌 텍스쳐 좌표로 texel 을 샘플링하려고 하면,
			인접한 주변의 texl 값들이 섞여서 샘플링됨.

			이때, 텍스쳐의 경계 부분을 샘플링할 경우,
			GL_REPEAT 모드로 인해 next repeat 지점의 인접 texel 들을 가져와
			혼합을 시도하게 되고,

			인접한 texel 들 중에서 텍스쳐 경계 부분의 texel 과
			투명도가 아예 달라지는 값이 필시 존재할테니,
			그 투명도가 혼합되어 '반투명(semi-transparent) 경계'가 샘플링됨.

			이를 방지하기 위해,
			GL_CLAMP_TO_EDGE 모드로 WRAPPING 모드를 설정하여
			텍스쳐 경계 부분이 엉뚱한 texel 의 투명도와 섞이지 않도록 함!
		*/
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