#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리

#include <iostream>

// 콜백함수 선언
/*
	C++ 에서는 자바스크립트와 달리 함수 호이스팅이 불가능하므로,
	함수를 main() 에서 호출하고 싶다면, 
	main() 윗부분에 해당 함수를 선언해놔야 함.

	사실, 이것보다 더 좋은 방법은, 
	함수 선언부를 헤더파일로 분리해서 include 하는 게 더 적절함.
*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수 (변경된 GLFW 윈도우 사이즈에 맞게 glViewport() 사이즈 변경) 
void processInput(GLFWwindow* window); // GLFW 윈도우 및 키 입력 감지 및 이에 대한 반응 처리 함수 선언

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

	// GLFW 윈도우 생성 (윈도우 높이, 너비, 윈도우창 제목, 풀스크린 모드/창모드, 컨텍스트 리소스를 공유할 또 다른 윈도우)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		// GLFW 윈도우 생성 실패(null 체크) 시 예외 처리
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); // 현재 남아있는 glfw 윈도우 제거, 리소스 메모리 해제, 라이브러리 초기화 이전 상태로 되돌림.(-> GLFW 함수 사용 불가. glfwInit() 과 반대 역할!) -> 게임루프 탈출 후, 애플리케이션 종료 전 사용 권장
		return -1;
	}

	/*
		glfwMakeContextCurrent(실행중인 스레드의 현재 컨텍스트 설정)

		OpenGL 컨텍스트는 실행되는 단일 스레드 단위마다 하나씩 가질 수 있음.
		이때, 현재 실행중인 스레드의 현재 OpenGL 컨텍스트를 변경하고자 할 때,
		glfwMakeContextCurrent() 를 사용함.

		보통, 새로운 GLFWwindow 윈도우 객체를 만들면, 
		해당 윈도우의 OpenGL 컨텍스트를 
		현재 실행중인 스레드의 현재 컨텍스트로 지정하기 위해
		주로 사용함.
	*/
	glfwMakeContextCurrent(window); 

	/*
		콜백함수 등록은
		GLFWwindow 생성 후 ~ 게임루프 시작 전
		사이에 등록하도록 되어있음.

		Three.js 할때도 비슷한 시점에 등록했던 것 같음!
	*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // GLFWwindow 창 크기 변경(resize) 감지 시, 발생시킬 리사이징 콜백함수 등록

	/*
		OpenGL 은 운영체제 및 그래픽 드라이버마다 필요한 함수들을
		가져오는 방식이 다르기 때문에, 컴파일 시점이 아닌 런타임 시점에
		함수 포인터를 가져와달라고 요청해야 함.

		그래서 런타임에서 프로그램이 실행되는 운영체제에 걸맞는
		OpenGL 함수 포인터를 GLAD 로 로드해서 초기화해야 한댔지?

		그 작업을 해주는 게 gladLoadGLLoader()

		이때, glfwGetProcAddress() 함수는
		현재 운영체제에 맞는 OpenGL 함수들의 주소값을 반환해줌.

		그러나, 이는 GLFW 라이브러리의 함수이기 때문에,
		glfwGetProcAddress() 가 반환해 준 주소값을
		GLAD 라이브러리의 함수인 gladLoadGLLoader() 에서 사용하려면
		gladLoadGLLoader() 가 원하는 함수 포인터 주소값 타입인
		GLADloadproc 로 캐스팅(형변환)을 해주고 나서 전달해줘야 함!

		이렇게 포인터 주소값을 형변환해줘야,
		gladLoadGLLoader() 함수가 받아들일 수 있게 되는 것임!
	*/
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		// GLAD 로 런타임에 각 운영체제에 걸맞는 함수 포인터 초기화 실패 시, 예외 처리.
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 렌더링 루프 구현
	// C++ 에서는 while 문으로 구현한다고 함.
	/*
		glfwWindowShouldClose(GLFWwindow* window)

		얘는 현재 루프를 시작하기 전,
		GLFWwindow 를 종료하라는 명령이 있었는지 체크함.

		명령이 없다면 false 를 반환하므로 while 문이 계속될 것이고,
		명령이 있었다면 true 를 반환하여 while 문이 종료됨.

		while 문을 종료한 이후에는
		애플리케이션을 종료해도 괜찮겠지.
	*/
	while (!glfwWindowShouldClose(window))
	{
		processInput(window); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. -> 이게 OpenGL 컨텍스트의 상태('색상 버퍼를 초기화할 색상'이라는 상태값)를 설정하는 함수!! (state-setting)
		glClear(GL_COLOR_BUFFER_BIT); // glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼를 초기화함. -> 이게 OpenGL 컨텍스트의 상태('색상 버퍼를 초기화할 색상')를 검색 및 가져와서 사용하는 함수!! (state-using)

		/*
			참고로, glClear() 는
			C++ 의 '비트 플래그' 기법을 사용하는
			대표적인 사례!

			glClear() 는
			현재 프레임 버퍼의 정보들 중에서
			색상 버퍼, 깊이 버퍼, 스텐실 버퍼 등
			여러 개의 버퍼를 각각 초기화해줄 수 있는 함수!

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			위와 같이, bitwise operator(비트단위 연산자) 를 사용해서
			각 버퍼를 초기화할 지 정의하는 플래그를 
			비트 단위로 저장 및 관리할 수 있도록 함.
		*/


		/*
			glfwSwapBuffers(GLFWwindow* window)

			얘는 뭐냐면, 최종적으로 렌더링할 이미지 버퍼를 교체하는 역할을 함.

			이걸 이해하려면 "Double Buffer" 가 뭔지 알아야 하는데,
			애플리케이션이 Single Buffer(단일 버퍼)로 이미지를 그리려고 하면,
			약간의 깜빡임 현상이 발생할 수 있음.

			그 이유는, 현재 프레임에서 다음 프레임으로 한방에 교체되는 게 아니고,
			프레임의 좌상단 픽셀에서 우하단 픽셀로 
			한 픽셀 한 픽셀씩 색상이 교체되면서
			이미지가 변경되기 때문!

			그래서, 단일 버퍼 상의 모든 픽셀들이 한땀한땀 교체되기까지 걸리는 텀이
			사람의 눈에 인식되기에 깜빡이는 것으로 보이게 된다는 것이지.

			그래서, Front 버퍼와 Back 버퍼 두 개의 Double 버퍼라는 이중 구조로 만들어놓고,
			Back 버퍼에서 렌더링 명령에 따라서 픽셀들을 한땀한땀 교체한 다음,
			픽셀들의 교체가 완료되면 그 순간 Back 버퍼와 Front 버퍼를 '교체(Swap)' 해버림.
			그러면 실제로 사람들은 픽셀 교체가 완료된 Front 버퍼만 보게 되기 때문에,
			이미지가 바로바로 교체되는 것처럼 보여서 깜빡거림 증세가 사라진다는 것이지.

			이 '교체(Swap)'을 처리해주는 함수가 바로 glfwSwapBuffers() 라는 것!
		*/
		glfwSwapBuffers(window);

		/*
			glfwPollEvents()

			얘는 키보드 입력, 마우스 입력 등의 이벤트 발생을 검사하고,
			해당 이벤트에 등록되어 있는 콜백함수를 호출하는 역할을 함.

			또한, 이벤트 발생에 따른 GLFWwindow 상태를 업데이트하는 역할도 함.
		*/
		glfwPollEvents();
	}

	/*
		while 문, 즉 렌더링 루프가 종료되어 while 블록을 탈출하면,
		렌더링이 종료되었다는 의미임.

		따라서, 더 이상 GLFWwindow 와 리소스 메모리를 유지할 필요가 없으므로,
		윈도우를 종료하고 리소스 메모리를 해제한다.

		아까 GLFWwindow 생성 예외 처리에서도 사용했던 함수지?
	*/
	glfwTerminate();

	return 0;
}

// 맨 위에 선언된 함수 정의
/*
	선언된 함수를 정의하는 위치는 마음대로 지정해도 되지만,
	
	가장 좋은 건, 이러한 util 함수들을 정의하는 .cpp 파일을
	별도로 만들어서 include 해서 사용하는 게 가장 좋음.
*/
// GLFWwindow 윈도우 창 리사이징 감지 시, 호출할 콜백 함수 정의
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	/*
		glViewport() 
		
		GLFWwindow 상에 렌더링될 뷰포트 영역을 정의함.
		첫 번째와 두 번째 인자는 뷰포트 영역에 좌상단 좌표,
		세 번째와 네 번째 인자는 뷰포트 영역의 너비와 높이를 지정함.
		
		이렇게 지정된 뷰포트 정보를 기반으로, OpenGL 이 내부에서
		-1 ~ 1 사이의 NDC 좌표계 > 스크린좌표계로 변환할 때 사용함.

		참고로, GLFWwindow 와 뷰포트 영역을 다르게 설정할수도 있으나,
		가급적 뷰포트 영역과 맞추는 게 정상적이고 일반적인 방식이겠지?
	*/
	glViewport(0, 0, width, height);
}

// GLFWwindow 윈도우 입력 및 키 입력 감지 후 이벤트 처리 함수 (렌더링 루프에서 반복 감지)
void processInput(GLFWwindow* window)
{
	/*
		glfwGetKey(GLFWwindow, 키 enum)

		얘는, 특정 GLFWwindow 에 대하여(활성화 시,)
		특정 키가 입력되었는지 여부를 감지함.

		입력 상태에서는 GLFW_PRESS 이넘을 반환하고,
		미입력 상태에서는 GLFW_RELEASE 이넘을 반환함.

		렌더링 루프에서 매 프레임마다 각각의 키 입력을 반복적으로 감지함.

		따라서, 감지하고자 하는 키 입력을 
		이 함수 안에서 쭉 나열해서 추가해주면 되겠지!
	*/
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		/*
			현재 렌더링 루프에서 ESC 키 입력 상태 시,
			인자로 넘겨준 GLFWwindow 의 WindowShouldClose 플래그(상태값)을
			true 또는 false 로 생성할 수 있음.

			여기서 WindowShouldClose 플래그가 재설정되면,
			main() 함수의 while 조건문에서 해당 플래그를 검사하면서
			렌더링 루프를 종료할 지를 결정함.
		*/
		glfwSetWindowShouldClose(window, true);
	}
}