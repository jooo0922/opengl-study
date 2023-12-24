#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리
#include <algorithm> // std::min(), std::max() 를 사용하기 위해 포함한 라이브러리

// 행렬 및 벡터 계산에서 사용할 Header Only 라이브러리 include
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MyHeaders/shader_s.h"

#include <iostream>

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수

// 윈도우 창 생성 옵션
// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이

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

	// GLAD 로 런타임에 각 운영체제에 걸맞는 OpenGL 함수 포인터 초기화 및 실패 시 예외 처리.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 각 프래그먼트에 대한 깊이 버퍼(z-buffer)를 새로운 프래그먼트의 깊이값과 비교해서
	// 프래그먼트 값을 덮어쓸 지 말 지를 결정하는 Depth Test(깊이 테스팅) 상태를 활성화함
	glEnable(GL_DEPTH_TEST);

	// Instancing 기법을 적용한 쉐이더 객체 생성
	Shader shader("MyShaders/instancing.vs", "MyShaders/instancing.fs");


	/* Instanced Array 로 전송할 각 Quad 의 offset position 계산하여 정적 배열에 저장 */
	
	// 각 Quad 인스턴스의 offset position 을 저장할 정적 배열
	glm::vec2 translations[100];

	// Quad 인스턴스 개수만큼 증가시켜가며 정적 배열의 인덱싱 값으로 사용할 변수
	int index = 0;

	// screen 의 좌상단 모서리로부터 띄워줄 간격
	float offset = 0.1f;

	// 이중 for-loop 안에서 각 Quad 의 offset position 계산
	for (int y = -10; y < 10; y += 2)
	{
		for (int x = -10; x < 10; x += 2)
		{
			// NDC 좌표계 기준, 각 Quad 의 offset position 의 x, y 좌표값을 각각 [-1, 1] 범위 사이의 값으로 계산.
			// 이때, 각 Quad 의 offset position 은 0.2 만큼의 거리를 둔 채 계산되겠군. 
			// (참고로, quadVertices 에서는 각 Quad 의 width, height 이 0.1 로 나오도록 NDC 좌표를 지정하므로, 
			// 각 Quad 사이의 간격은 0.1 (0.2 - 0.1 = 0.1) 로 계산될 것임.)
			// + 여기에, screen 의 좌상단 모서리로부터 띄워줄 간격인 offset 을 각 x, y 좌표값에 모두 더해줌.
			glm::vec2 translation;
			translation.x = (float)x / 10.0f + offset;
			translation.y = (float)y / 10.0f + offset;
			translations[index++] = translation;
		}
	}


	/* Instanced Array 로 전송할 데이터를 덮어쓸 VBO 객체 생성 */

	// VBO 객체 참조 id 를 저장할 변수
	unsigned int instanceVBO;

	// VBO(Vertex Buffer Object) 객체 생성
	glGenBuffers(1, &instanceVBO);

	// VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	// Instanced Array 에 전송할 데이터를 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
	// (참고로, 정적 배열을 전송할 때에는, 정적 배열의 첫 번째 요소의 주소값만 전달하면 됨! why? 이 자체가 정적 배열의 주소값과 같으니까!)
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);

	// VBO 객체 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Instancing 으로 렌더링할 Quad 정점 데이터의 정적 배열 초기화 (위치값은 NDC 좌표계 기준)
	float quadVertices[] = {
		// positions     // colors
		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		-0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

		-0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
		 0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
		 0.05f,  0.05f,  0.0f, 1.0f, 1.0f
	};


	/* Quad 의 VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩(하단 VAO 관련 필기 참고) */

	// VBO, VAO 객체(object) 참조 id 를 저장할 변수
	unsigned int quadVBO, quadVAO;

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


	/* 정점 단위로 업데이트할 attribute 데이터들의 해석 방식 설정 */

	// 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. 
	// -> 그 중에서 0번 location 변수(aPos)만 사용하도록 활성화한 것!
	glEnableVertexAttribArray(0);

	// 정점 위치 데이터(0번 location 입력변수 in vec2 aPos 에 전달할 데이터) 해석 방식 정의
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// 1번 location 변수(aColor)를 사용하도록 활성화
	glEnableVertexAttribArray(1);

	// 정점 색상 데이터(1번 location 입력변수 in vec3 aColor 에 전달할 데이터) 해석 방식 정의
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));


	/* Instance 단위로 업데이트할 attribute 데이터의 해석 방식 설정 */

	// 2번 location 변수(aOffset)를 사용하도록 활성화 (== instanced array 로 사용할 vertex attribute 변수)
	glEnableVertexAttribArray(2);

	// instanced array 에 전송할 데이터가 덮어쓰여진 VBO 객체 바인딩
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

	// 인스턴스 단위로 각 정점에 더해줄 offset 데이터(2번 location 입력변수 in vec2 aOffset 에 전달할 데이터) 해석 방식 정의
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	// VBO 객체에 쓰여진 데이터 해석 방식 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// instanced array 로 사용될 2번 location 변수의 업데이트 단위를 vertex -> instance 로 변경함 (관련 필기 하단 참고)
	glVertexAttribDivisor(2, 1);

	// 마찬가지로, VAO 객체도 OpenGL 컨텍스트로부터 바인딩 해제 
	glBindVertexArray(0);


	// while 문으로 렌더링 루프 구현
	while (!glfwWindowShouldClose(window))
	{
		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		// 색상 버퍼 및 깊이 버퍼 초기화 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...


		/* Quad 객체를 Instancing 하여 100 개 그리기 */


		// Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwSwapBuffers(window);

		// 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
		glfwPollEvents();
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	// while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제
	glfwTerminate();

	return 0;
}


/* 전방선언된 콜백함수 정의 */

// GLFWwindow 윈도우 창 리사이징 감지 시, 호출할 콜백 함수 정의
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height); // GLFWwindow 상에 렌더링될 뷰포트 영역을 정의. (뷰포트 영역의 좌상단 좌표, 뷰포트 영역의 너비와 높이)
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
	평범한 vertex attribute 변수를 instanced array 로 만드는 방법 
	(feat. glVertexAttribDivisor())


	LearnOpenGL 본문에서도 나왔듯이,
	일반적인 배열 형태의 uniform 변수( ex> uniform vec2 offsets[100]; )로 
	각 Instance 들의 offset position 을 관리할수도 있지만,

	uniform 변수에 한 번에 전송할 수 있는 데이터의 양에는 한계가 있으므로,
	더 많은 양의 데이터를 빠르게 GPU 로 전송하기 위해
	vertex attribute 변수를 사용하려는 것이 instanced array 의 기본 아이디어임!

	그러나, vertex attribute 는
	버텍스 쉐이더에서 vertex 단위로 (per vertex) iteration 이 발생할 때마다
	값이 업데이트되기 때문에,

	이를 instance 단위로 (per instance) 값이 업데이트 되도록
	설정을 바꿔줘야 함!

	그래야 instanced array 변수로써 사용할 수 있겠지!

	이를 위해 사용하는 OpenGL 함수가 glVertexAttribDivisor(GLuint index, GLuint divisor) 라고 보면 됨.

	첫 번째 매개변수인 index 는
	instanced array 로 사용할 vertex attribute 의 location 값에 해당하고,

	두 번째 매개변수인 divisor 는
	0 일 경우, per vertex update 로 설정되고,
	1 일 경우, per instance update 로 설정됨.
	또한, 2일 경우, every 2 instances update 로 설정됨.
*/