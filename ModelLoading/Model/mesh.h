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

// 정점 구조체 선언
// 정점을 구조체로 선언하고, 구조체 멤버를 각각 glm::vec3 타입으로 선언함으로써, 
// 정점 데이터 해석하는 코드를 좀 더 쉽게 작성할 수 있는 이점이 있음! 
struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent; // 정점의 탄젠트 벡터 > 노말맵 적용 시, TBN 행렬 계산에 사용됨.
    glm::vec3 BiTangent; // 정점의 비탄젠트 벡터 > 노말맵 적용 시, TBN 행렬 계산에 사용됨.
    int m_BoneIDs[MAX_BONE_INFLUENCE]; // Bone 인덱스 정점 배열 (SkinnedMesh 를 고려한 정점 데이터)
    float m_Weights[MAX_BONE_INFLUENCE]; // Bone 의 가중치 정점 배열 (SkinnedMesh 를 고려한 정점 데이터)
};

#endif // !MESH_H
