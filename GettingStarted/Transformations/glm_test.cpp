#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
/*
	glm

	GLM은 OpenGL Mathematics 의 약자이고, Header Only 라이브러리. 
	링킹과 컴파일이 필요없고, 헤더파일의 루트 디렉터리(glm/)만 libs/include 폴더에 추가하면 됨.
	
	glm 으로 행렬을 다룰 시, 
	주의할 점이 하나 있음.

	일반적으로 three.js 같은 데에서는 new THREE.Matrix4() 로 생성하면
	기본적으로 단위행렬로 초기화되지만, 

	glm 0.9.9 이후 버전부터는 glm::mat4 로 생성할 시, 
	모든 요소가 0인 null 행렬로 초기화됨.
	(원래는 0.9.9 이하 버전에서도 단위행렬로 초기화 됬었다고 함.)

	따라서, 단위행렬로 glm::mat4 를 초기화하고 싶다면,
	glm::mat4(1.0f) 이런 식으로 초기화해줘야 함.
*/

#include <iostream>

using namespace std;

int test()
{
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f); // 위치 벡터 (1, 0, 0) 을 glm::vec4 타입 변수로 선언 및 초기화
	//glm::vec4 vec = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // 위치 벡터를 이렇게 초기화한 것과 동일함. 초기화 문법이 다를 뿐임.
	
	glm::mat4 trans = glm::mat4(1.0f); // 이동 변환행렬로 만들 glm::mat4 타입 변수 선언 및 단위행렬로 초기화

	// glm::translate() 함수를 사용하여 trans 행렬을 이동 변환행렬로 만듦.
	// 이때, 함수의 첫 번째 파라미터에는 이동 변환행렬로 만들 mat4 행렬을 전달하고,
	// 두 번째 파라미터에는 얼만큼 이동할 것인지 정의하는 이동 벡터를 glm::vec3 타입으로 생성하여 전달함.
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));

	// 위치 벡터를 정의한 glm::vec4 타입 변수 vec 에
	// glm::mat4 타입의 이동 변환행렬을 행렬 곱하여 vec 을 이동 변환시킴 
	// 따라서, 정상적으로 변환이 적용되었다면 vec 변수 벡터는 (2, 1, 0) 으로 변환되어 있어야 할 것임.
	vec = trans * vec;

	cout << vec.x << vec.y << vec.z << endl; // 이동 변환이 정상적으로 수행되었다면, 210 이 출력됨.

	return 0;
}