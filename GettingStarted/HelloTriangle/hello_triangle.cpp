#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리

#include <iostream>

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
void processInput(GLFWwindow* window); // GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언

// 윈도우 창 생성 옵션
// 너비와 높이는 음수가 없으므로, 부호가 없는 정수형 타입으로 심볼릭 상수 지정 (가급적 전역변수 사용 자제...)
const unsigned int SCR_WIDTH = 800; // 윈도우 창 너비
const unsigned int SCR_HEIGHT = 600; // 윈도우 창 높이

// 버텍스 쉐이더 소스코드를 문자열로 저장한 포인터 변수 (나중에 Shader 클래스를 만들어서 .vs, .fs 파일로 사용할 수 있도록 할 것임.)
const char* vertexShaderSource = "#version 330 core\n" // 쉐이더 버전 명시
"layout (location = 0) in vec3 aPos;\n" // 입력할 vertex attribute (위치값 데이터)의 location 을 0으로 받음.
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n" // 다음 파이프라인으로 넘길 버텍스 쉐이더의 출력값은 gl_Position 에 할당
"}\n\0"; // 참고로, '\0' 는 NULL 종료 문자를 나타내며, char 타입 문자열 배열 끝부분에 ASCII 코드 상에서 'NULL' 을 의미하는 '\0' 을 저장함으로써, 문자열의 끝부분임을 알림.

// 프래그먼트 쉐이더 소스코드를 문자열로 저장한 포인터 변수
const char* fragmentShaderSource = "#version 330 core\n" // 쉐이더 버전 명시
"out vec4 FragColor;\n" // out 키워드로 vec4 타입의 프래그먼트 쉐이더 출력 변수 선언
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" // 프래그먼트 쉐이더의 최종 출력값을 vec4 타입의 주황색 값으로 할당
"}\n\0"; // NULL 종료 문자 '\0' 로 문자열의 끝부분임을 표시

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

	// GLAD 로 런타임에 각 운영체제에 걸맞는 OpenGL 함수 포인터 초기화 및 실패 시 예외 처리.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// while 문으로 렌더링 루프 구현
	// glfwWindowShouldClose(GLFWwindow* window) 로 현재 루프 시작 전, GLFWwindow 를 종료하라는 명령이 있었는지 검사.
	while (!glfwWindowShouldClose(window))
	{
		processInput(window); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClear(GL_COLOR_BUFFER_BIT); // glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼를 초기화함. (state-using)

		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	glfwTerminate(); // while 렌더링 루프 탈출 시, GLFWwindow 종료 및 리소스 메모리 해제

	return 0;
}

// 전방선언된 콜백함수 정의
// GLFWwindow 윈도우 창 리사이징 감지 시, 호출할 콜백 함수 정의
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height); // GLFWwindow 상에 렌더링될 뷰포트 영역을 정의. (뷰포트 영역의 좌상단 좌표, 뷰포트 영역의 너비와 높이)
}

// GLFWwindow 윈도우 입력 및 키 입력 감지 후 이벤트 처리 함수 (렌더링 루프에서 반복 감지)
void processInput(GLFWwindow* window)
{
	// 현재 GLFWwindow 에 대하여(활성화 시,) 특정 키(esc 키)가 입력되었는지 여부를 감지
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, true); // GLFWwindow 의 WindowShouldClose 플래그(상태값)을 true 로 설정 -> main() 함수의 while 조건문에서 렌더링 루프 탈출 > 렌더링 종료!
	}
}