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

	// 버텍스 쉐이더 객체 생성 및 컴파일
	unsigned int vertexShader; // OpenGL 쉐이더 객체(object)의 참조 id를 저장할 변수
	vertexShader = glCreateShader(GL_VERTEX_SHADER); // OpenGL 쉐이더 객체(object) 생성 > 버텍스 쉐이더 생성 시, GL_VERTEX_SHADER 파라미터 입력
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // 생성된 버텍스 쉐이더 객체(object)에 쉐이더 소스코드 문자열 붙임 (두 번째 파라미터 1은 소스코드가 1개의 문자열(char) 변수로 구성됨을 의미.
	glCompileShader(vertexShader); // 쉐이더 소스코드 문자열 변수의 주소값(&vertexShaderSource)이 연결된 버텍스 쉐이더 객체(object)를 런타임에 동적으로 컴파일

	int success; // 컴파일 성공여부 저장하는 정수형 변수 선언
	char infoLog[512]; // 최대 512개의 char 타입 문자를 저장할 수 있는 문자열 배열 변수 선언 > 일반적으로는 std::string 클래스로 문자열을 다루는 경우가 더 많음. > 암튼 여기에 컴파일 에러 메시지 저장
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // 생성된 쉐이더 객체에 대한 정보를 질의할 수 있는 쿼리 함수 > 컴파일 상태(	GL_COMPILE_STATUS)를 물어보고 결과를 success 변수에 저장

	if (!success)
	{
		// 컴파일 실패 시, 에러 메시지 출력
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); // 문자열 배열 변수 infoLog 에 버텍스 쉐이더 객체(object)에 대한 InfoLog 를 최대 512자까지 저장
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// 프래그먼트 쉐이더 객체 생성 및 컴파일
	unsigned int fragmentShader; // OpenGL 쉐이더 객체(object)의 참조 id를 저장할 변수
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // OpenGL 쉐이더 객체(object) 생성
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // 생성된 프래그먼트 쉐이더 객체(object)에 쉐이더 소스코드 문자열 붙임
	glCompileShader(fragmentShader); // 프래그먼트 쉐이더 객체(object)를 런타임에 동적으로 컴파일

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); // 생성된 쉐이더 객체에 대한 정보를 질의할 수 있는 쿼리 함수
	if (!success)
	{
		// 컴파일 실패 시, 에러 메시지 출력
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); // 문자열 배열 변수 infoLog 에 프래그먼트 쉐이더 객체(object)에 대한 InfoLog 를 최대 512자까지 저장
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// 생성된 여러 쉐이더 객체 연결 > 쉐이더 프로그램 객체 생성
	unsigned int shaderProgram = glCreateProgram(); // OpenGL 쉐이더 프로그램 객체(object)의 참조 id를 저장할 변수
	glAttachShader(shaderProgram, vertexShader); // 그래픽 파이프라인의 입출력 순서에 따라 쉐이더를 프로그램 객체에 붙여줘야 함. (즉, 버텍스 쉐이더 -> 프래그먼트 쉐이더 순!)
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram); // 쉐이더 프로그램에 붙여진 쉐이더 객체를 연결

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); // 생성된 프로그램 객체에 대한 정보를 질의하는 쿼리 함수
	if (!success)
	{
		// 쉐이더 프로그램 linking 실패 시, 에러 메시지 출력
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); // 문자열 배열 변수 infoLog 에 쉐이더 프로그램 객체(object)에 대한 InfoLog 를 최대 512자까지 저장
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// 이미 쉐이더 프로그램 객체에 연결한 쉐이더 객체들은 더 이상 불필요하므로 제거!
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// 정점 위치 데이터 배열 생성 (좌표 변환을 배우기 전이므로, 버텍스 쉐이더의 출력변수에 바로 할당할 수 있는 NDC좌표계([-1, 1] 사이)로 구성)
	// 두 개의 삼각형을 Indexed Drawing 으로 그려서 사각형을 그려내기 위해, 사각형 정점을 추가함.
	float vertices[] = {
		 0.5f,  0.5f, 0.0f, // 우측 상단
		 0.5f, -0.5f, 0.0f, // 우측 하단
		-0.5f, -0.5f, 0.0f, // 좌측 하단
		-0.5f,  0.5f, 0.0f  // 좌측 상단
	};

	// EBO(Element Buffer Object) 생성 시 사용할 정점 인덱스 배열 생성 > EBO 는 한마디로 인덱스 버퍼라고 보면 됨!
	unsigned int indices[] = {
		0, 1, 3, // 첫 번째 삼각형 인덱스
		1, 2, 3 // 두 번째 삼각형 인덱스 
	};

	// VAO(Vertex Array Object), VBO(Vertex Buffer Object) 생성 및 바인딩 + VBO 에 쓰여진 버텍스 데이터 해석 방식 정의
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
	unsigned int VBO, VAO, EBO; // VBO, VAO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &VAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &VBO); // VBO(Vertex Buffer Object) 객체 생성 > VBO 객체를 만들어서 정점 데이터를 GPU 로 전송해야 한 번에 많은 양의 정점 데이터를 전송할 수 있음!
	glGenBuffers(1, &EBO); // EBO(Element Buffer Object) 객체 생성 > EBO 객체는 한 마디로, 정점의 인덱스 버퍼를 저장하는 객체로써, Indexed Drawing 으로 삼각형을 그릴 수 있게 해준다!

	glBindVertexArray(VAO); // VAO 객체 먼저 컨텍스트에 바인딩(연결)함. > 그래야 재사용할 여러 개의 VBO 객체들 및 설정 상태를 VAO 에 바인딩된 VAO 에 저장할 수 있음.

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // OpenGL 컨텍스트 중에서, VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
	// 참고로, 각 인자는 (데이터를 복사해 집어넣을 버퍼 유형, 덮어쓸 데이터의 크기, 덮어쓸 실제 데이터, 그래픽 카드가 해당 데이터 관리 방식) 을 의미함
	// 데이터 관리 방식은, 삼각형 정점 위치 데이터는 변경되지 않을 데이터이므로, GL_STATIC_DRAW 로 지정함. 
	// 만약 변경이 잦은 데이터일 경우, GL_DYNAMIC_DRAW | GL_STREAM_DRAW 로 설정하면 그래픽 카드가 빠르게 데이터를 write 할 수 있는 메모리에 저장한다고 함.

	// EBO 객체를 OpenGL 컨텍스트에 바인딩하고, 바인딩된 버퍼에 인덱스 배열 데이터 복사
	// 참고로, VAO 객체 바인딩을 해제하기 전에 EBO 객체를 바인딩 및 버퍼를 복사하면,
	// EBO 객체 및 설정 상태 또한 VAO 객체에 저장해서 재사용할 수 있음!!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // OpenGL 컨텍스트 중에서, EBO 객체는 GL_ELEMENT_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 실제 정점 인덱스 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 EBO 객체에 덮어씀.

	// VBO 객체 설정
	// VBO 에 쓰여진 정점 데이터 해석 방식 설정
	// 각 파라미터는 1. 이 데이터를 가져다 쓸 버텍스 쉐이더의 attribute 변수의 location (0이 aPos 였으니 0으로 지정)
	// 2. 정점 당 필요한 데이터 개수
	// 3. 데이터 타입
	// 4. 데이터를 -1 ~ 1 사이로 정규화할 지 여부. 이미 vertices 배열은 NDC 좌표 기준으로 구성해놨으니 필요없겠군!
	// 5. stride. 정점 데이터 세트 사이의 메모리 간격. 각 정점은 현재 3개의 위치 데이터만 가져다 쓰므로, <실수형 리터럴의 메모리 크기 * 3> 이 정점 데이터 세트 간 메모리 간격
	// 6. 정점 버퍼에서 데이터 시작 위치 offset > vertices 의 시작 지점부터 위치 데이터이므로, 그냥 0으로 하면 될건데, 마지막 파라미터 타입이 void* 타입으로만 받아서 형변환을 해준 것.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. > 그 중에서 0번 location 변수만 사용하도록 활성화한 것!

	glBindBuffer(GL_ARRAY_BUFFER, 0); // VBO 객체 설정을 끝마쳤다면, OpenGL 컨텍스트로부터 바인딩(연결)을 해제함.

	glBindVertexArray(0); // 마찬가지로, VAO 객체에 저장해둘 VBO 객체 및 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 

	// 와이어프레임 모드로 그리기 (정확히는 각 polygon 의 rasterization mode 를 설정하는 것!)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 각 파라미터는 (설정한 polygon mode 앞면/뒷면 적용, polygon rasterzing 방식 (선으로 그리기 or 면으로 채우기))

	// while 문으로 렌더링 루프 구현
	// glfwWindowShouldClose(GLFWwindow* window) 로 현재 루프 시작 전, GLFWwindow 를 종료하라는 명령이 있었는지 검사.
	while (!glfwWindowShouldClose(window))
	{
		processInput(window); // 윈도우 창 및 키 입력 감지 밎 이벤트 처리

		// 현재까지 저장되어 있는 프레임 버퍼(그 중에서도 색상 버퍼) 초기화하기
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 어떤 색상으로 색상 버퍼를 초기화할 지 결정함. (state-setting)
		glClear(GL_COLOR_BUFFER_BIT); // glClearColor() 에서 설정한 상태값(색상)으로 색상 버퍼를 초기화함. (state-using)

		// 여기서부터 루프에서 실행시킬 모든 렌더링 명령(rendering commands)을 작성함.
		// ...
		// 삼각형 그리기 명령 실행
		glUseProgram(shaderProgram); // 미리 생성 및 linking 해둔 쉐이더 프로그램 객체를 사용하여 그리도록 명령
		glBindVertexArray(VAO); // 미리 생성한 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령
		//glDrawArrays(GL_TRIANGLES, 0, 3); // 실제 primitive 그리기 명령을 수행하는 함수 
		// glDrawArrays 의 각 파라미터는 다음과 같다. 
		// 1. 그려야 할 primitive 유형 
		// 2. 정점 배열의 시작 인덱스(활성화된 정점 버퍼 배열에서 시작 요소의 인덱스) 
		// 3. 현재 도형에 몇 개의 정점을 그릴지(삼각형이니 3개 겠지?))

		// 앞서 바인딩된 VAO 에 저장해둔 EBO 객체 (즉, 정점 인덱스 버퍼)로부터 Indexed Drawing 으로 그리려면, 
		// glDrawArrays() 대신 glDrawElements() 를 사용해줘야 함!
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 각 파라미터는 (삼각형 모드, 그리고 싶은 정점 개수, 인덱스들의 타입, EBO 버퍼 offset)

		glfwSwapBuffers(window); // Double Buffer 상에서 Back Buffer 에 픽셀들이 모두 그려지면, Front Buffer 와 교체(swap)해버림.
		glfwPollEvents(); // 키보드, 마우스 입력 이벤트 발생 검사 후 등록된 콜백함수 호출 + 이벤트 발생에 따른 GLFWwindow 상태 업데이트
	}

	// 렌더링 루프 종료 시, 생성해 둔 VAO, VBO, EBO, 쉐이더 프로그램 객체들은 더 이상 필요가 없으므로 메모리 해제한다!
	// 이들은 미리 생성 후 저장해두는 이유는, 렌더링이 진행중에 필요한 객체들을 다시 바인딩하여 교체하기 위한 목적이므로,
	// 렌더링이 끝났다면 더 이상 메모리에 남겨둘 이유가 없음!!
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO); 
	glDeleteProgram(shaderProgram);

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