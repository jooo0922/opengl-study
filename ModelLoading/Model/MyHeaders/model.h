#ifndef MODEL_H
#define MODEL_H

// 운영체제별 OpenGL 함수 포인터 포함
#include <glad/glad.h> 

// glm 라이브러리 사용을 위한 헤더파일 포함
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 텍스쳐 이미지 로드를 위한 stb_image 라이브러리 포함
#include "stb_image.h"

// 3D 모델 포맷 로딩을 위한 Assimp 라이브러리 포함
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Model 을 구성하는 Mesh 클래스 인스턴스 생성을 위해 포함
#include "mesh.h"

// Draw 그리기 명령 호출 시 전달할 Shader 클래스 포함
#include "shader_s.h"

// std::string 클래스 포함
#include <string>

// std::vector 동적 배열 클래스 포함
#include <vector>

// 입출력 스트림 클래스 포함
#include <iostream>

using namespace std;

// 텍스쳐 객체 생성 및 참조 ID 반환 함수 전방선언
unsigned int TextureFromFile(const char* path, const string& directory);

class Model
{
public:
	// model data 관련 public 멤버 선언
	vector<Texture> textures_loaded; // 텍스쳐 객체 중복 생성 방지를 위해 이미 로드된 텍스쳐 구조체를 동적 배열에 저장해두는 멤버
	vector<Mesh> meshes; // Model 클래스에 사용되는 Mesh 클래스 인스턴스들을 동적 배열에 저장하는 멤버
	string directory; // 3D 모델 파일이 위치하는 디렉토리 경로를 저장하는 멤버

	// 생성자 함수 선언 및 구현
	Model(const string& path)
	{
		// 생성자에서 Assimp 로 모델 로드하는 함수 곧바로 호출
		loadModel(path);
	}

	// Model 클래스 내에 저장된 모든 Mesh 클래스 인스턴스의 Draw() 명령 호출 멤버 함수
	void Draw(Shader& shader)
	{
		for (unsigned int i = 0; i < meshes.size(); i++)
		{
			meshes[i].Draw(shader);
		}
	}

private:
	void loadModel(const string& path)
	{
		// Assimp 로 Scene 노드 불러오기 (Assimp 모델 구조 참고)
		Assimp::Importer importer;

		// 비트플래그 연산을 통해, 3D 모델을 Scene 구조로 불러올 때의 여러 가지 옵션들을 지정함
		// 비트플래그 및 비트마스킹 연산 관련 https://github.com/jooo0922/cpp-study/blob/main/TBCppStudy/Chapter3_9/Chapter3_9.cpp 참고
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			/*
				Scene 구조가 다음 케이스 중 하나로 인해 불완전하게 로드되었을 때 에러를 출력함.

				1. Scene 구조의 포인터가 NULL 일 때
				2. AI_SCENE_FLAGS_INCOMPLETE 와의 비트마스킹 연산을 통해, 모델이 불완전하게 불러온 것이 확인되었을 때
				3. Scene 구조의 RootNode 가 존재하지 않을 때 
			*/
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}

		// 3D 모델 파일이 존재하는 디렉토리 경로를 멤버변수에 저장
		// 참고로, std::string.find_last_of('/')는 string 으로 저장된 문자열 상에서 마지막 '/' 문자가 저장된 위치를 반환함.
		// std::string.substr() 는 string 에서 지정된 시작 위치와 마지막 위치 사이의 부분 문자열을 반환함.
		directory = path.substr(0, path.find_last_of('/'));
		
		// Assimp Scene 구조를 따라 재귀적으로 RootNode 를 처리함
		processNode(scene->mRootNode, scene);
	}

	void processNode(aiNode* node, const aiScene* scene)
	{

	}
};

// 텍스쳐 객체 생성 및 참조 ID 반환 함수 구현
// 각 입력 매개변수는 1. 파일 이름, 2. 공통 디렉토리 경로(3D 모델 파일과 동일한 디렉토리)
unsigned int TextureFromFile(const char* path, const string& directory)
{
	// 각 문자열을 조합하여 텍스쳐 이미지 경로 생성 
	// 이 경로는 3D 모델 파일과 텍스쳐 파일이 동일한 디렉토리에 있다는 전제하에 유효한 경로임! 
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID; // 텍스쳐 객체(object) 참조 id 를 저장할 변수 선언
	glGenTextures(1, &textureID); // 텍스쳐 객체 생성

	// 로드한 이미지의 width, height, 색상 채널 개수를 저장할 변수 선언
	int width, height, nrComponents;

	// 이미지 데이터 가져와서 char 타입의 bytes 데이터로 저장. 
	// 이미지 width, height, 색상 채널 변수의 주소값도 넘겨줌으로써, 해당 함수 내부에서 값을 변경. -> 출력변수 역할
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

#endif // !MODEL_H

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