#ifndef SHADER_H
#define SHADER_H

/*
	#ifndef ~ #endif 전처리기는
	헤더파일의 헤더가드를 처리하여 중복 include 방지해 줌!
*/

#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 > shader 관련 OpenGL 함수가 필요하니까!
#include <glm/glm.hpp> // glm 으로 생성한 벡터 및 행렬 데이터를 쉐이더 프로그램의 유니폼 변수로 전송하기 위해 include

#include <string> // std::string 을 사용할 시, 이 라이브러리를 include 해줘야 함.
#include <fstream> // 파일 입출력(파일 열기, 읽기, 쓰기 등...) 관련 라이브러리 (.vs, .fs 등의 shader 파일을 다룰 때 필요)
#include <sstream> // 문자열 스트림 관련 라이브러리 (문자열 파싱, 문자열을 다른 데이터 타입을 변환 등...)
#include <iostream> // cout, cin, endl 등 콘솔 입출력 관련 라이브러리

/*
	Shader 클래스

	쉐이더 파일 파싱, 컴파일, 컴파일 에러 예외처리,
	쉐이더 프로그램 생성 및 관리, uniform 변수에 데이터 전송 등

	쉐이더와 관련된 모든 작업들을 관리하는 클래스!

	즉, 기존 쉐이더 관련 코드들을 별도의 클래스로 추출하는
	리팩토링을 했다고 보면 됨!
*/
class Shader
{
public:
	unsigned int ID; // 생성된 ShaderProgram 의 참조 id

	// Shader 클래스 생성자 (shader 파일 읽기 및 compile, linking 등의 작업 담당)
	// 생성자 인자로 쉐이더 파일 경로 문자열에 대한 참조 (포인터)를 전달받음.
	// (+ geometry shader 파일 경로를 선택적으로 입력받을 수 있도록 매개변수 추가 
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr)
	{
		// 쉐이더 코드를 std::string(문자열) 타입으로 파싱하여 저장할 변수 선언 
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;

		// std::ifstream 은 파일에서 데이터를 읽어오는 작업을 수행하는 입력 파일 스트림(input file stream) 클래스
		// <fstream> 에 정의된 C++ 표준 라이브러리 클래스
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;

		// 입력 파일 스트림 클래스에서 파일 읽기 작업 도중 failbit 및 badbit 관련 에러가 발생하면,
		// C++ 예외 처리 기능을 통해 프로그램 흐름을 제어하도록 함. > 파일 읽기 작업의 오류 대응 및 안정성 확보
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// 이제 try...catch 문으로 파일 읽기 작업에 대한 예외처리가 가능해 짐!
		try
		{
			// 파일 열기 (파일을 읽으려면 열어야겠지?)
			// 여기서 '연다'는 것은 해당 파일로부터 데이터를 읽어오기 위한 준비를 한다는 뜻.
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);

			/*
				stringstream(문자열 스트림)

				문자열 스트림이란,
				문자열은 아니지만, 문자열을 마치 데이터 스트림(stream) 처럼
				다룰 수 있도록 해주는 클래스를 뜻함.

				여기서 스트림이란,
				cout << 데이터 << endl;
				이런 식으로 콘솔 출력할 때,
				스트림 연산자 << 등을 통해 입출력 장치(파일, 키보드, 화면 등)으로
				흘려보내는 데이터를 의미함.

				그래서 std::ifstream 클래스는
				.rdbuf() 라는 함수를 사용함으로써,

				입출력 장치인 파일의 내용을 스트림 버퍼에서 읽어와서
				해당 내용을 데이터로 흘려보낸다.

				결국, .rdbuf() 는 스트림 버퍼에서 읽어온 내용을
				'문자열 스트림' 이라는 데이터 타입으로 반환하다 보니,

				먼저 std::stringstream 타입의 변수에 반환된 문자열 스트림을 저장하고,
				해당 문자열 스트림을 실제 '문자열'로 파싱해서 사용하도록
				코드를 짜게 된 것!
			*/
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf(); // cout << 하는 것과 동일하게 스트림 데이터를 흘려보내서 변수에 저장
			fShaderStream << fShaderFile.rdbuf();

			// 파일의 스트림 버퍼에서 내용을 읽어왔으니,
			// 다 읽은 파일은 닫아줘야 메모리 누수 방지할 수 있음.
			vShaderFile.close();
			fShaderFile.close();

			// 저장해둔 문자열 스트림을 실제 문자열로 파싱
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();

			// 만약, geometry shader 파일 경로도 입력받았다면, 
			// geometry shader 에 대해서도 동일하게 처리해 준다!
			if (geometryPath != nullptr)
			{
				// 파일 열기
				gShaderFile.open(geometryPath);

				// 스트림 버퍼에서 읽어온 데이터를 문자열 스트림 변수에 흘려보내서 저장
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();

				// 파일 닫기
				gShaderFile.close();

				//저장해둔 문자열 스트림을 실제 std::string 타입 문자열로 파싱
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure e)
		{
			// 파일 읽기 작업 도중 에러가 발생하면 예외처리 및 에러 출력
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}

		/*
			std::string.c_str()

			std::string 은 c++ 표준 라이브러리에만 존재하는 문자열 타입으로,
			문자열을 쉽게 다룰 수 있는 편리하고 좋은 클래스임.

			하지만, OpenGL 는 핵심 라이브러리가 C 로 구현됐다고 했었지?

			그래서, glShaderSource() 같이 문자열을 받는 함수들은
			C 스타일의 문자열, 즉 null 종료문자(\o) 를 갖고 있는
			char 타입의 배열 형태로 문자열을 받아서 처리하도록 되어있음.

			그래서 std::string 을 C 스타일 문자열로 변환하기 위해
			필요한 함수가 .c_str() 이라고 보면 됨.
		*/
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		/* 아래부터는 기존 main 함수에 있던 쉐이더 / 쉐이더 프로그램 객체 생성 및 컴파일 / 링킹 작업을 그대로 옮겨온 것 */

		// 쉐이더 객체 생성 및 컴파일
		unsigned int vertex, fragment; // OpenGL 쉐이더 객체(object)의 참조 id를 저장할 변수

		// 버텍스 쉐이더 생성 및 컴파일
		vertex = glCreateShader(GL_VERTEX_SHADER); // OpenGL 쉐이더 객체(object) 생성
		glShaderSource(vertex, 1, &vShaderCode, NULL); // 생성된 버텍스 쉐이더 객체(object)에 쉐이더 소스코드 문자열 붙임
		glCompileShader(vertex); // 쉐이더 소스코드 문자열이 연결된 버텍스 쉐이더 객체(object)를 런타임에 동적으로 컴파일
		checkCompileErrors(vertex, "VERTEX"); // 쉐이더 컴파일 에러 대응

		// 프래그먼트 쉐이더 생성 및 컴파일
		fragment = glCreateShader(GL_FRAGMENT_SHADER); // OpenGL 쉐이더 객체(object) 생성
		glShaderSource(fragment, 1, &fShaderCode, NULL); // 생성된 프래그먼트 쉐이더 객체(object)에 쉐이더 소스코드 문자열 붙임
		glCompileShader(fragment); // 쉐이더 소스코드 문자열이 연결된 프래그먼트 쉐이더 객체(object)를 런타임에 동적으로 컴파일
		checkCompileErrors(fragment, "FRAGMENT"); // 쉐이더 컴파일 에러 대응

		// 지오메트리 쉐이더 파일 경로를 입력 받았다면, 동일한 작업을 처리해 줌!
		unsigned int geometry; // 지오메트리 쉐이더 객체(object)의 참조 id 를 저장할 변수
		if (geometryPath != nullptr)
		{
			// 지오메트리 쉐이더 문자열을 C 스타일 문자열로 변환
			const char* gShaderCode = geometryCode.c_str();

			// 지오메트리 쉐이더 생성 및 컴파일
			geometry = glCreateShader(GL_GEOMETRY_SHADER); // OpenGL 쉐이더 객체(object) 생성
			glShaderSource(geometry, 1, &gShaderCode, NULL); // 생성된 지오메트리 쉐이더 객체(object)에 쉐이더 소스코드 문자열 붙임
			glCompileShader(geometry); // 쉐이더 소스코드 문자열이 연결된 지오메트리 쉐이더 객체(object)를 런타임에 동적으로 컴파일
			checkCompileErrors(geometry, "GEOMETRY"); // 쉐이더 컴파일 에러 대응
		}

		// 쉐이더 프로그램 객체 생성 및 쉐이더 객체 linking
		ID = glCreateProgram(); // OpenGL 쉐이더 프로그램 객체(object)의 참조 id를 멤버변수에 저장
		glAttachShader(ID, vertex); // 그래픽 파이프라인의 입출력 순서에 따라 쉐이더를 프로그램 객체에 붙여줘야 함. (즉, 버텍스 쉐이더 -> 프래그먼트 쉐이더 순!)
		glAttachShader(ID, fragment);

		// 지오메트리 쉐이더 파일 경로를 입력 받았다면, 동일한 작업을 처리해 줌!
		if (geometryPath != nullptr)
		{
			glAttachShader(ID, geometry);
		}

		glLinkProgram(ID); // 쉐이더 프로그램에 붙여진 쉐이더 객체를 연결 > 이때 쉐이더 간 입출력 변수들끼리 연결됨!
		checkCompileErrors(ID, "PROGRAM"); // 쉐이더 프로그램 linking 에러 대응

		// 이미 쉐이더 프로그램 객체에 연결한 쉐이더 객체들은 더 이상 불필요하므로 제거!
		glDeleteShader(vertex);
		glDeleteShader(fragment);

		// 지오메트리 쉐이더 파일 경로를 입력 받았다면, 동일한 작업을 처리해 줌!
		if (geometryPath != nullptr)
		{
			glDeleteShader(geometry);
		}
	}

	// ShaderProgram 객체 활성화(바인딩)
	void use()
	{
		glUseProgram(ID); // 미리 생성 및 linking 해둔 쉐이더 프로그램 객체를 사용하도록 바인딩 (state-setting)
	}

	// 해당 쉐이더 프로그램의 uniform 변수 관련 utils
	/*
		const std::string &name (매개변수를 참조로 선언)

		&name 은 이름 문자열이 담긴 메모리 공간에 대한 참조(주소값)을 뜻함.

		즉, 자동 메모리에 새로운 메모리 공간을 할당하는
		새로운 매개변수를 만드는 것(매개변수를 const std::string name 으로 선언했다면 이렇게 됬을 것임.)
		이 아니라,

		인자로 전달한 이름 문자열 변수가 가리키는
		동일한 메모리 공간에 저장된 이름 문자열 데이터에 접근하기 위해
		참조 주소값을 넘겨주도록 매개변수를 선언한 것!

		즉, 매개변수로 전달받는 문자열 데이터를
		복사해서 새로운 메모리 공간(만약 매개변수를 const std::string name 로 선언했다면 새로운 메모리 공간이 할당되므로...)
		에 할당하고 싶지 않은 것임!

		매개변수로 전달받은 문자열이 저장된 동일한 메모리 공간에서
		데이터를 참조해서 사용하기 위해 매개변수를 &(레퍼런스) 로 선언한 것!
	*/
	/*
		상수 멤버 함수 (const member function)

		함수 선언 맨 뒷쪽에 'const' 키워드가 붙으면,
		"이 함수는 해당 클래스의 멤버변수를 변경하지 않는
		'상수 멤버 함수' 입니다~" 라는 뜻임.

		이딴 게 왜 필요하냐?

		예를 들어, 지금 구현된 Shader 클래스를
		인스턴스화해서 어떤 객체를 만든다고 치자.

		호출부에서
		const Shader shaderObj;
		이런 식으로 Shader 클래스로 인스턴스화할 객체의 변수에
		const 키워드를 붙여서 '상수 객체' 로 만들어서 쓰는 경우가 있음.

		'상수 객체'는
		객체 내부의 데이터(즉, 멤버변수)에 대한 변경이 불가함.
		그래서, 동일한 클래스로 선언된 객체라고 하더라도,
		'상수 객체'인 경우, 멤버변수를 변경시키는 메서드(멤버 함수)는
		호출이 불가능함.

		대신, 이런 상수 객체에서도 호출이 가능한 메서드를 만들고자 한다면,
		멤버 함수 선언 끝에 'const' 를 붙임으로써,
		객체 내의 데이터를 변경하지 않음을 보장하는 '상수 멤버 함수'로
		만듦으로써, 상수 객체를 통해서도 해당 함수를 호출할 수 있음!

		이를 통해 객체 내의 데이터를 보호하고,
		객체를 불변 상태로 유지시킬 수 있겠지!
	*/
	void setBool(const std::string& name, bool value) const
	{
		// glGetUniformLocation() 함수로 멤버변수 ID 가 가리키는 쉐이더 프로그램 객체에 존재하는
		// 특정 유니폼 변수의 location 을 가져오고, 
		// 이 location 값을 통해 bool 값을 유니폼 변수에 전송함.
		// 이때, boolean 타입은 정수형 타입인 0 또는 1 로도 처리가 가능하여, 정수형으로 형변환해서 전달.
		// 그래서 glUniform1i(1 int) 로 유니폼 변수를 전송함.
		// 또한, OpenGL 함수들은 C 스타일 문자열만 받는다고 했으니까 유니폼 변수명 문자열을
		// std::string 타입에서 c 스타일 문자열로 변환하여 전달함.
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	// vec2 타입 데이터를 배열 형태로 전달하는 메서드
	// 이때, 배열 데이터를 복사해서 새롭게 선언된 매개변수의 새로운 메모리 공간에 복사해서 쓰고 싶지는 않으므로,
	// value 매개변수를 &(레퍼런스)로 받음으로써, 
	// 인자로 전달받은 배열이 이미 존재하는 메모리 공간에 저장된 배열 데이터를 
	// 참조해서 쓰고 싶은 것! (name 매개변수를 const std::string& name 로 선언한 것과 같은 이유!)
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		/*
			glUniform2fv()

			vec2 타입 유니폼 변수에 데이터를 배열 형태로 전송할 때 사용

			각 매개변수는 다음과 같다.
			1. name 에 해당하는 uniform 변수 location
			2. 몇 개의 vec2 데이터를 넣어줄 것인가
			3. vec2 타입의 요소들을 모두 직접 전송하는 것이 아니라, 첫 번째 요소의 메모리 주소값만 전달함.

			즉, glUniform2fv() 는 vec2 타입 데이터를 '배열' 형태로 미리 저장해놨다가
			포인터(배열의 첫 번째 요소의 주소값)로 전달하는 구조임.
		*/
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	// vec2 타입 데이터를 직접 전달하는 메서드
	void setVec2(const std::string& name, float x, float y) const
	{
		/*
			glUniform2f()

			vec2 타입 유니폼 변수에 float 데이터 2개를 직접 전달함.

			각 매개변수는 다음과 같다.
			1. name 에 해당하는 uniform 변수 location
			2. 직접 전달할 vec2 의 x값
			3. 직접 전달할 vec2 의 y값

			즉, glUniform2f() 는 vec2 의 요소 x, y 를
			실제 float 타입 데이터로 직접 전달하는 구조임.
		*/
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}

	// vec3 타입 데이터를 배열 형태로 전달하는 메서드
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	// vec3 타입 데이터를 직접 전달하는 메서드
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}

	// vec4 타입 데이터를 배열 형태로 전달하는 메서드
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

	// vec4 타입 데이터를 직접 전달하는 메서드
	void setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}

	// mat2 타입 데이터를 2차원 배열로 전달하는 메서드
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		/*
			glUniformMatrix2fv()

			mat2 타입 유니폼 변수에 데이터를 2차원 배열 형태로 전송할 때 사용
			(c++ 에서 행렬은 보통 2차원 배열로 표현.
			다차원 배열에 대해서는 https://github.com/jooo0922/cpp-study/blob/main/TBCppStudy/Chapter6_05/Chapter6_05.cpp 참고)

			각 매개변수는 다음과 같다.
			1. name 에 해당하는 uniform 변수 location
			2. 몇 개의 mat2 데이터를 넣어줄 것인가
			3. 행과 열을 바꿀 것인지 boolean 값 전달
			4. 2차원 배열 요소들을 모두 직접 전송하는 것이 아니라, 첫 번째 요소의 메모리 주소값만 전달함.

			참고로 3번은 뭐냐면, OpenGL 이 열 우선 행렬을 사용하고,
			GLM 또한 기본 행렬 레이아웃을 열 우선 행렬로 구현하고 있기 때문에,
			굳이 열과 행을 바꿀 필요가 없음.

			즉, glUniformMatrix2fv() 는 mat2 타입 데이터를 '다차원 배열' 형태로 미리 저장해놨다가
			포인터(배열의 첫 번째 요소의 주소값)로 전달하는 구조임.
		*/
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	// mat3 타입 데이터를 2차원 배열로 전달하는 메서드
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	// mat4 타입 데이터를 2차원 배열로 전달하는 메서드
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// Shader 객체 및 ShaderProgram 객체의 compile 및 linking 에러 대응
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success; // 컴파일 또는 링킹 성공여부 저장하는 정수형 변수 선언
		char infoLog[1024]; // 최대 1024개의 char 타입 문자를 저장할 수 있는 문자열 배열 변수 선언 > 일반적으로는 std::string 클래스로 문자열을 다루는 경우가 더 많음. > 암튼 여기에 컴파일 에러 메시지 저장

		if (type != "PROGRAM")
		{
			// 쉐이더 객체 컴파일 에러 대응
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // 생성된 쉐이더 객체에 대한 정보를 질의할 수 있는 쿼리 함수 > 컴파일 상태(GL_COMPILE_STATUS)를 물어보고 결과를 success 변수에 저장
			if (!success)
			{
				// 컴파일 실패 시, 에러 메시지 출력
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);  // 문자열 배열 변수 infoLog 에 쉐이더 객체(object)에 대한 InfoLog 를 최대 1024자까지 저장
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			// 쉐이더 프로그램 객체 링킹 에러 대응
			glGetProgramiv(shader, GL_LINK_STATUS, &success); // 생성된 프로그램 객체에 대한 정보를 질의하는 쿼리 함수
			if (!success)
			{
				// 쉐이더 프로그램 linking 실패 시, 에러 메시지 출력
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);  // 문자열 배열 변수 infoLog 에 쉐이더 프로그램 객체(object)에 대한 InfoLog 를 최대 1024자까지 저장
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};


#endif // !SHADER_H
