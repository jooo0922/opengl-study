#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리

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
#define STB_IMAGE_IMPLEMENTATION
#include "MyHeaders/stb_image.h"

#include "MyHeaders/shader_s.h"

#include <iostream>

// 콜백함수 전방선언
void framebuffer_size_callback(GLFWwindow* window, int width, int height); // GLFW 윈도우 크기 변경 감지 시, 호출할 콜백함수
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

	// Shader 클래스를 생성함으로써, 쉐이더 객체 / 프로그램 객체 생성 및 컴파일 / 링킹
	Shader ourShader("MyShaders/shader.vs", "MyShaders/shader.fs");

	// 정점 위치 데이터 배열 생성 (좌표 변환을 배우기 전이므로, 버텍스 쉐이더의 출력변수에 바로 할당할 수 있는 NDC좌표계([-1, 1] 사이)로 구성)
	// 여기에 정점 색상 데이터와 텍스쳐 좌표(uv) 데이터까지 추가함. 
	// 하나의 배열에 정점의 위치 데이터와 색상 데이터, uv 데이터가 섞여있기 때문에, 
	// glVertexAttribPointer() 를 통해서 정점 데이터 해석 방식을 잘 설정해줘야 함.
	float vertices[] = {
		// positions 데이터		// colors 데이터		  // 텍스쳐 좌표(uv) 데이터	
		 0.5f,  0.5f, 0.0f,     1.0f, 0.0f, 0.0f,	  1.0f, 1.0f,				// top right
		 0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,	  1.0f, 0.0f,				// bottom right
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,	  0.0f, 0.0f,				// bottom left
		-0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 0.0f,	  0.0f, 1.0f				// top left
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
	unsigned int VBO, VAO, EBO; // VBO, VAO, EBO 객체(object) 참조 id 를 저장할 변수
	glGenVertexArrays(1, &VAO); // VAO(Vertex Array Object) 객체 생성
	glGenBuffers(1, &VBO); // VBO(Vertex Buffer Object) 객체 생성 > VBO 객체를 만들어서 정점 데이터를 GPU 로 전송해야 한 번에 많은 양의 정점 데이터를 전송할 수 있음!
	glGenBuffers(1, &EBO); // EBO(Element Buffer Object) 객체 생성 > EBO 객체는 한 마디로, 정점의 인덱스 버퍼를 저장하는 객체로써, Indexed Drawing 으로 삼각형을 그릴 수 있게 해준다!

	glBindVertexArray(VAO); // VAO 객체 먼저 컨텍스트에 바인딩(연결)함. > 그래야 재사용할 여러 개의 VBO 객체들, EBO 객체들 및 설정 상태를 바인딩된 VAO 에 저장할 수 있음.

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // OpenGL 컨텍스트 중에서, VBO 객체는 GL_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함.
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 실제 정점 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 VBO 객체에 덮어씀.
	// 참고로, 각 인자는 (데이터를 복사해 집어넣을 버퍼 유형, 덮어쓸 데이터의 크기, 덮어쓸 실제 데이터, 그래픽 카드가 해당 데이터 관리 방식) 을 의미함
	// 데이터 관리 방식은, 삼각형 정점 위치 데이터는 변경되지 않을 데이터이므로, GL_STATIC_DRAW 로 지정함. 
	// 만약 변경이 잦은 데이터일 경우, GL_DYNAMIC_DRAW | GL_STREAM_DRAW 로 설정하면 그래픽 카드가 빠르게 데이터를 write 할 수 있는 메모리에 저장한다고 함.

	// EBO 객체를 OpenGL 컨텍스트에 바인딩하고, 바인딩된 버퍼에 인덱스 배열 데이터 복사
	// 참고로, VAO 객체 바인딩을 해제하기 전에 EBO 객체를 바인딩 및 버퍼를 복사하면,
	// EBO 객체 및 설정 상태 또한 VAO 객체에 저장해서 재사용할 수 있음!!
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // OpenGL 컨텍스트 중에서, EBO 객체는 GL_ELEMENT_ARRAY_BUFFER 타입의 버퍼 유형 상태에 바인딩되어야 함
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // 실제 정점 인덱스 데이터를 생성 및 OpenGL 컨텍스트에 바인딩된 EBO 객체에 덮어씀.

	// VBO 객체 설정
	// VBO 에 쓰여진 정점 데이터 해석 방식 설정
	// 각 파라미터는 1. 이 데이터를 가져다 쓸 버텍스 쉐이더의 attribute 변수의 location (0이 aPos 였으니 0으로 지정)
	// 2. 정점 당 필요한 데이터 개수
	// 3. 데이터 타입
	// 4. 데이터를 -1 ~ 1 사이로 정규화할 지 여부. 이미 vertices 배열은 NDC 좌표 기준으로 구성해놨으니 필요없겠군!
	// 5. stride. 정점 데이터 세트 사이의 메모리 간격. 각 정점은 현재 3개의 위치 데이터 + 3개의 색상 데이터 + 2개의 uv 데이터(총합 8개)를 가져다 쓰므로, <실수형 리터럴의 메모리 크기 * 8> 이 정점 데이터 세트 간 메모리 간격
	// 6. 정점 버퍼에서 데이터 시작 위치 offset > vertices 의 시작 지점부터 위치 데이터이므로, 그냥 0으로 하면 될건데, 마지막 파라미터 타입이 void* 타입으로만 받아서 형변환을 해준 것.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); // 원래 버텍스 쉐이더의 모든 location 의 attribute 변수들은 사용 못하도록 디폴트 설정이 되어있음. > 그 중에서 0번 location 변수만 사용하도록 활성화한 것!

	// vertex attribute 가 추가될 때마다, (색상 데이터가 추가되었지?)
	// 각 정점 attribute 로 들어올 정점 데이터의 해석 방식을 별도로 설정해줘야 함.
	// color attribute 의 location 은 1이였으니 1로 지정하고,
	// 정점 버퍼에서 색상 데이터의 시작 위치(offset)는 x, y, z 3개의 위치 데이터 다음부터 시작하므로, 3 * sizeof(float) = 12 bytes 만큼의 메모리 공간을 띄운 지점부터 시작! 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1); // color attribute 변수의 location 인 1번 location 변수도 사용할 수 있게 활성화함.

	// vertex attribute 로 텍스쳐 좌표(uv) 데이터를 가져다 쓸 aTexCoord 가 추가됨.
	// 그에 따라, 각 정점 attribute 로 들어올 uv 데이터 해석 방식을 추가로 설정함
	// aTexCoord attribute 의 location 은 2로 설정했고,
	// 정점 버퍼에서 uv 데이터의 시작 위치(offset)는 x, y, z 3개의 위치 데이터와 r, g, b 3개의 색상 데이터 다음부터 시작하므로 6 * sizeof(float) = 24 bytes 만큼의 메모리 공간을 띄운 지점부터 시작!
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); // uv attribute 변수의 location 인 2번 location 변수도 사용할 수 있게 활성화함.

	glBindBuffer(GL_ARRAY_BUFFER, 0); // VBO 객체 설정을 끝마쳤다면, OpenGL 컨텍스트로부터 바인딩(연결)을 해제함.

	/*
		EBO 객체는 왜 unbind 하지 않을까?

		삼각형을 인덱싱으로 draw 하고 싶다면,
		렌더링 루프에서 glDrawElements() 로 인덱싱 그리기 명령을 호출하기 전,
		glBindBuffer() 로 EBO 객체를 매번 바인딩해줘야 하는 번거로움이 있음.

		그러나, VAO 객체가 바인딩 되어있는 동안, EBO 객체도 바인딩되어 있다면,
		VAO 객체에 바인딩된 EBO 객체도 저장해둘 수 있기 때문에,
		EBO 객체를 매번 바인딩하지 않고,
		VAO 객체만 그리기 명령 전에 바인딩 해주면
		EBO 도 알아서 자동으로 바인딩 됨!

		그러나, 만약 VAO 객체가 바인딩 되어있는 동안,
		EBO 객체를 unbind 해버리면, 그 unbind 이력이 VAO 객체에 저장되어 버려서
		그리기 명령 전에 VAO 객체를 바인딩한다고 하더라도,
		EBO 객체까지 자동으로 바인딩되지 않음!

		따라서, EBO 로 인덱싱 그리기를 하고 싶다면,
		VAO 가 바인딩 되어있는 동안에는 절대로 EBO 를 unbind 하면 안됨!
	*/
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0); // 마찬가지로, VAO 객체에 저장해둘 VBO, EBO 객체 및 설정도 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 

	// 와이어프레임 모드로 그리기 (정확히는 각 polygon 의 rasterization mode 를 설정하는 것!)
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 각 파라미터는 (설정한 polygon mode 앞면/뒷면 적용, polygon rasterzing 방식 (선으로 그리기 or 면으로 채우기))

	// 텍스쳐 이미지 로드 및 텍스쳐 객체 생성 
	// (텍스쳐 객체 생성 및 바인딩, 데이터 덮어쓰기 등의 과정은 VBO, VAO, EBO 객체 생성 과정과 매우 유사함!)
	unsigned int texture1, texture2; // 텍스쳐 객체(object) 참조 id 를 저장할 변수들 선언

	// 첫 번째 텍스쳐 처리
	glGenTextures(1, &texture1); // 텍스쳐 객체 생성
	glBindTexture(GL_TEXTURE_2D, texture1); // OpenGL 컨텍스트 중에서, 2D 텍스쳐로 사용할 객체는 GL_TEXTURE_2D 타입의 상태에 바인딩됨 > 이제 GL_TEXTURE_2D 관련 텍스쳐 명령은 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체 설정에 적용됨.

	// 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체 설정 명령
	// Texture Wrapping 모드 설정 ([(0, 0), (1, 1)] 범위를 벗어나는 텍스쳐 좌표에 대한 처리 모드)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Texture Wrapping 을 반복 모드로 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 텍스쳐 축소/확대 및 Mipmap 교체 시 Texture Filtering (텍셀 필터링(보간)) 모드 설정
	/*
		텍셀(텍스쳐 픽셀) 필터링 방식을 지정해줘야 하는 이유

		텍스쳐가 확대되거나 축소될 때,
		보간된 어떤 텍스쳐 좌표가 예를 들어, (0.1234567..., 0.1234567...) 이라고 쳐보자.

		그런데, 텍스쳐 이미지의 텍셀(텍스쳐 해상도 상에서 각 텍스쳐 픽셀 하나하나의 단위)에서
		정확히 (0.1234567..., 0.1234567...) 에 대응되는 텍셀이 존재한다고 볼 수 있나?

		절대 아님!
		예를 들어, 텍스쳐 이미지 해상도가 256 * 256 이라고 한다면,
		(n/256, n/256 (단, n 은 0 ~ 256 사이)) 텍스쳐 좌표에 대응하는 텍셀은 찾을 수 있지만,
		정확히 (0.1234567..., 0.1234567...) 좌표에 대응하는 텍셀은 알 수가 없겠지?

		이런 식으로, 소수점이 너무 긴 텍스쳐 좌표의 경우,
		어떤 텍셀이 대응되는지 정확히 알 수 없으므로,
		저러한 텍스쳐 좌표를 받았을 때, 어떤 텍스쳐 색상값을 반환해줘야 하는지
		그 방식을 정의하는 게 Texture Filtering 이라고 볼 수 있음.

		GL_NEAREST 은 그냥 현재 텍스쳐 좌표와 각 텍셀의 중점 사이의 거리를 비교해서
		가장 가까운 텍셀의 색상값을 넘겨주는 것이고, (> 그래서 약간 blocky 하고 각져보이는 느낌이 듦.)

		GL_LINEAR 은 현재 텍스쳐 좌표와 가까이에 있는 텍셀들을 적당하게 섞어서
		주변 텍셀들 사이의 근사치를 색상값으로 계산하여 넘겨주는 방식
		> 즉, 주변 텍셀 색상들을 섞어서 반환해 줌! (> 그래서 약간 antialiasing 이 적용된 느낌이 듦.)
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Mipmap 은 텍스쳐 축소 시에만 사용되므로, 텍스쳐 확대 시에는 Mipmap 필터링 옵션을 적용할 필요 없음 > 심지어 적용하면 에러가 발생함.

	// 텍스쳐 객체에 사용할 이미지 로드하기 (stb_image.h 라이브러리 사용)
	int width, height, nrChannels; // 로드한 이미지의 width, height, 색상 채널 개수를 저장할 변수 선언

	// stb_image.h 이미지를 로드할 때, 이미지 데이터의 y축을 뒤집도록 함.
	// why? 보통 이미지 데이터는 y축 상단이 0.0 으로 정의되어 있지만, OpenGL 은 y축 하단을 0.0 으로 인식해서 이미지를 읽으므로, 
	// 그냥 로드하면 이미지가 뒤집어지는 이슈 발생.
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load("images/container.jpg", &width, &height, &nrChannels, 0); // 이미지 데이터 가져와서 char 타입의 bytes 데이터로 저장. (이미지의 width, height, 색상 채널 수도 저장)
	if (data)
	{
		// 이미지 데이터 로드 성공 시 처리
		// 로드한 이미지 데이터를 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체에 덮어쓰기 (VBO 에 버퍼 데이터 덮어쓰는 원리랑 유사!)
		// 각 매개변수에 대한 설명은 https://learnopengl.com/Getting-started/Textures 참고
		// 참고로 두 번째 파라미터 0은 우리가 생성할 텍스쳐 Mipmap 의 레벨을 지정하는 것이지만, 일단 기본 레벨인 0단계만 전달함.
		// 각 단계마다 밉맵을 생성하려면 0, 1, 2, 3, 4,... 이런식으로 두 번째 파라미터에 이어서 전달해줘야 하지만,
		// glGenerateMipmap() 을 사용하면 모든 단계의 밉맵을 알아서 자동으로 생성해준다고 함.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체에 필요한 모든 단계의 Mipmap 을 자동 생성함. 
	}
	else
	{
		// 이미지 데이터 로드 실패 시 처리
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data); // 텍스쳐 객체에 이미지 데이터를 전달하고, 밉맵까지 생성 완료했다면, 로드한 이미지 데이터는 항상 메모리 해제할 것!

	// 두 번째 텍스쳐 처리
	glGenTextures(1, &texture2); // 두 번째 텍스쳐 객체 생성
	glBindTexture(GL_TEXTURE_2D, texture2); // 두 번째 텍스쳐 객체를 GL_TEXTURE_2D 타입의 상태에 바인딩 > 이제 텍스쳐 객체 설정 명령이 바인딩된 두 번째 텍스쳐 객체에 적용될 것임.

	// 현재 바인딩된 두 번째 텍스쳐 객체 설정 명령
	// Texture Wrapping 모드 설정 ([(0, 0), (1, 1)] 범위를 벗어나는 텍스쳐 좌표에 대한 처리 모드)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Texture Wrapping 을 반복 모드로 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 텍스쳐 축소/확대 및 Mipmap 교체 시 Texture Filtering (텍셀 필터링(보간)) 모드 설정
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Mipmap 은 텍스쳐 축소 시에만 사용되므로, 텍스쳐 확대 시에는 Mipmap 필터

	data = stbi_load("images/awesomeface.png", &width, &height, &nrChannels, 0); // 두 번째 텍스쳐 객체에 사용할 이미지 데이터 로드
	if (data)
	{
		// 이미지 데이터 로드 성공 시 처리
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); // .png 는 알파 채널을 포함하므로, GL_RGBA 로 텍스쳐 포맷 및 원본 이미지 데이터 포맷 설정
		glGenerateMipmap(GL_TEXTURE_2D); // 현재 GL_TEXTURE_2D 상태에 바인딩된 텍스쳐 객체에 필요한 모든 단계의 Mipmap 을 자동 생성함. 
	}
	else
	{
		// 이미지 데이터 로드 실패 시 처리
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data); // 텍스쳐 객체에 이미지 데이터를 전달하고, 밉맵까지 생성 완료했다면, 로드한 이미지 데이터는 항상 메모리 해제할 것!

	// 여러 개의 텍스쳐 객체를 생성해서 쉐이더 프로그램의 sampler 타입 uniform 변수로 전송하고 싶다면,
	// 각 sampler 변수가 몇 번 텍스쳐 위치(보통 Texture Unit 이라고도 함.)에 바인딩된 텍스쳐 객체를 가져다 쓸 것인지 알려줘야 함.
	// 이 작업은 매 프레임마다 텍스쳐 이미지를 바꿔줄 게 아니라면 렌더링 루프 이전에 수행해줘도 괜찮음.
	// 이는 정수형 데이터를 유니폼 변수로 전송하는 glUniform1i() 함수를 사용하면 됨!
	ourShader.use(); // 항상 uniform 변수에 값을 전송할 때에는, 해당 변수가 선언된 Shader Program 을 바인딩해줌.
	ourShader.setInt("texture1", 0); // texture1 이라는 이름의 sampler 유니폼 변수에는 텍스쳐 유닛(텍스쳐 위치)을 0으로 전달함. > 0번 위치에 바인딩된 텍스쳐 객체를 사용하겠군. (참고로, Shader.setInt() 는 내부적으로 glUniform1i() 로 구현되어 있음!)
	ourShader.setInt("texture2", 1); // texture2 이라는 이름의 sampler 유니폰 변수에는 텍스쳐 유닛을 1로 전달함. > 1번 위치에 바인딩된 텍스쳐 객체를 사용하겠군.

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

		// 삼각형 색상값 계산 후, uniform 변수에 전송하기
		//float timeValue = glfwGetTime(); // glfwSetTime() 이 호출되지 않았다면, glfwInit() 이 호출된 이후의 경과시간(Elapsed Time) 을 반환하는 함수
		//float greenValue = sin(timeValue) / 2.0f + 0.5f; // 경과시간으로 계산된 -1 ~ 1 사이의 sin값을 0 ~ 1 사이의 값으로 맵핑

		// uniform 변수도 attribute 와 마찬가지로 선언과 동시에 location 값을 갖음. 
		// 따라서, 해당 쉐이더 프로그램 객체와, 거기에 linking 된 쉐이더에 선언된 uniform 변수명을 전달해서 해당 uniform 변수의 location 을 반환받음.
		// 이 location 을 알아야 glUniform~() 함수로 실제 값을 전송할 수 있음.
		// 또 참고로, glGetUniformLocation() 함수까지는 location 을 읽어오기만 하면 되는 함수이므로, 
		// 쉐이더 프로그램 객체를 glUseProgram() 으로 바인딩하지는 않아도 됨!! 
		//int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");  

		// Texture Unit(바인딩할 텍스쳐 위치) 활성화 및 텍스쳐 바인딩
		// GL_TEXTURE_2D 상태에 texture1 텍스쳐 객체를 바인딩할 때, 0번 텍스쳐 유닛 위치에 바인딩하도록 0번 위치 활성화
		// -> 참고로, 0번 위치는 항상 기본으로 활성화되어 있는 텍스쳐 유닛이므로, 하나의 텍스쳐 객체만 바인딩해서 사용할 경우 glActiveTexture() 를 생략할 수 있었음!
		glActiveTexture(GL_TEXTURE0); // 앞전에 texture1 이라는 sampler uniform 변수는 0번 텍스쳐 유닛을 할당받았으니, 0번 위치에 바인딩된 텍스쳐 객체를 가져다 쓰겠군!
		glBindTexture(GL_TEXTURE_2D, texture1); // 0번 위치에 바인딩할 텍스쳐 객체 바인딩 > 텍스쳐 바인딩 정보는 VAO 객체에는 저장할 수 없어 매번 따로 바인딩해줘야 함.

		// GL_TEXTURE_2D 상태에 texture2 텍스쳐 객체를 바인딩할 때, 1번 텍스쳐 유닛 위치에 바인딩하도록 1번 위치 활성화
		glActiveTexture(GL_TEXTURE1); // 앞전에 texture2 이라는 sampler uniform 변수는 1번 텍스쳐 유닛을 할당받았으니, 1번 위치에 바인딩된 텍스쳐 객체를 가져다 쓰겠군!
		glBindTexture(GL_TEXTURE_2D, texture2); // 1번 위치에 바인딩할 텍스쳐 객체 바인딩


		// 삼각형 그리기 명령 실행
		ourShader.use(); // Shader 클래스 내에서 생성된 쉐이더 프로그램 객체를 바인딩하는 메서드 호출

		// 바인딩된 쉐이더 프로그램 객체안에 특정 location 이 할당된 uniform 변수에 실제 값을 세팅하기
		// 여기서부터는 실제로 쉐이더 프로그램 안의 uniform 변수의 값을 '설정'해줘야 하므로, 값을 설정할 쉐이더 프로그램을 먼저 바인딩해줘야 함! 
		/*
			OpenGL 타입 오버로딩 미지원

			OpenGL 은 핵심 라이브러리가 C 로 구현되어 있음.
			C 는 타입 오버로딩을 지원하지 않음.

			참고로, 타입 오버로딩이란,
			전달받은 인자의 타입에 따라서 서로 다른 동작을 수행하도록
			동일한 이름의 함수를 매개변수 타입에 따라 여러 버전으로
			중복선언하는 기능이라고 함.

			예를 들어,
			void doSomething(float data);
			void doSomething(int data);
			void doSomething(double data);

			요런 식으로, 전달받는 매개변수 타입에 따라
			다양한 버전으로 동일한 이름의 함수를 중복 선언함으로써,
			인자 타입에 따라 다른 동작으로 대응할 수 있도록 하는 개념.

			근데 C 에서는 동일한 이름의 함수 중복 선언이 불가하기 때문에,
			타입 오버로딩이 안됨.

			그래서, C 로 구현된 OpenGL 은
			어떤 함수를 매개변수에 타입에 따라 다양하게 동작을 수행하도록
			여러 버전으로 구현해놓기 위해서, 타입 오버로딩 대신,
			'-4f', '-3f', '-f', '-i', '-ui' 등의 접미어를 함수명 뒤에 붙여서
			인자 타입에 따라서 여러 동작으로 대응할 수 있도록 함.

			그 대표적인 사례가 glUniform~() 함수!

			여기서 우리는 uniform vec4 ourColor 유니폼 변수에
			데이터를 전송하고 싶기 때문에, -4f(float 타입의 요소 4개) 를 전달할 수 있는
			glUniform4f() 함수를 사용함!
		*/
		//glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);

		glBindVertexArray(VAO); // 미리 생성한 VAO 객체를 바인딩하여, 해당 객체에 저장된 VBO 객체와 설정대로 그리도록 명령

		//glDrawArrays(GL_TRIANGLES, 0, 3); // 실제 primitive 그리기 명령을 수행하는 함수 

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