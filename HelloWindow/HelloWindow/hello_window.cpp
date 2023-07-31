#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 (OpenGL 을 사용하는 다른 라이브러리(GLFW)보다 먼저 include 할 것.)
#include <GLFW/glfw3.h> // OpenGL 컨텍스트 생성, 윈도우 생성, 사용자 입력 처리 관련 OpenGL 라이브러리

#include <iostream>

int main()
{
	// GLFW 초기화
	glfwInit(); 

	// GLFW 윈도우(창) 설정 구성
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // GLFW 에게 OpenGL 3.3 버전 사용 명시
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // GLFW 에게 core-profile 버전 (Core-profile vs Immediate mode 비교글 참고)

	return 0;
}