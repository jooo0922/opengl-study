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

// fstream, sstream 은 딱히 사용하는 곳이 없어서 포함 안함...

// std::string 클래스 포함
#include <string>

// std::vector 동적 배열 클래스 포함
#include <vector>

// 입출력 스트림 클래스 포함
#include <iostream>

#endif // !MODEL_H
