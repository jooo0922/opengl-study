#ifndef MESH_H
#define MESH_H
/*
    mesh.h 헤더파일에 대한
    헤더가드 처리를 위해 전처리기 선언
*/

// 운영체제별 OpenGL 함수 포인터 포함
#include <glad/glad.h> 

// glm 라이브러리 사용을 위한 헤더파일 포함
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Mesh 렌더링 시 사용할 texture unit location 전송을 위해 Shader 클래스 포함
#include "MyHeaders/shader_s.h"

// Shader 클래스를 통해 전송할 sampler uniform 변수명을 파싱하기 위한 std::string 라이브러리 포함
#include <string>

// Mesh 를 구성하는 정점 데이터를 동적 배열로 관리하기 위해 std::vector 라이브러리 포함
#include <vector>

using namespace std;

// SkinnedMesh 를 고려하여 Mesh 클래스를 설계하고 있기 때문에,
// 각 정점마다 가중치(SkinWeight)를 부여해서 영향을 줄 수 있는 Bone 의 최대 갯수를 매크로 전처리기로 정의해 둠.
#define MAX_BONE_INFLUENCE 4

#endif // !MESH_H
