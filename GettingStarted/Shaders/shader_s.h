#ifndef SHADER_H
#define SHADER_H

/*
	#ifndef ~ #endif 전처리기는
	헤더파일의 헤더가드를 처리하여 중복 include 방지해 줌!
*/

#include <glad/glad.h> // 운영체제(플랫폼)별 OpenGL 함수를 함수 포인터에 저장 및 초기화 > shader 관련 OpenGL 함수가 필요하니까!

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
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
	{
		// 쉐이더 코드를 std::string(문자열) 타입으로 파싱하여 저장할 변수 선언 
		std::string vertexCode;
		std::string fragmentCode;

		// std::ifstream 은 파일에서 데이터를 읽어오는 작업을 수행하는 입력 파일 스트림(input file stream) 클래스
		// <fstream> 에 정의된 C++ 표준 라이브러리 클래스
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		// 입력 파일 스트림 클래스에서 파일 읽기 작업 도중 failbit 및 badbit 관련 에러가 발생하면,
		// C++ 예외 처리 기능을 통해 프로그램 흐름을 제어하도록 함. > 파일 읽기 작업의 오류 대응 및 안정성 확보
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

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
	}

	// ShaderProgram 객체 활성화(바인딩)
	void use()
	{

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
	void setBool(const std::string &name, bool value) const
	{

	}

	void setInt(const std::string& name, int value) const
	{

	}

	void setFloat(const std::string& name, float value) const
	{

	}

private:
	// Shader 객체 및 ShaderProgram 객체의 compile 및 linking 에러 대응
	void checkCompileErrors(unsigned int shader, std::string type)
	{

	}
};


#endif // !SHADER_H
