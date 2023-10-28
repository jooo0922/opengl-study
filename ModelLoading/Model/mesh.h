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

// 텍스쳐 구조체 선언
struct Texture
{
    unsigned int id; // glGenTextures() 로 생성한 텍스쳐 객체의 참조 ID 를 저장할 멤버
    string type; // 텍스쳐 타입을 문자열로 저장할 멤버 (e.g., "texture_diffuse", "texture_specular", ...)
    string path; // 텍스쳐 이미지 경로를 문자열로 저장할 멤버
};

// Mesh 클래스 선언
class Mesh
{
public:
    // public 멤버변수 선언
    vector<Vertex> vertices; // Mesh 의 정점 데이터를 동적 배열 멤버로 선언
    vector<unsigned int> indices; // Mesh 의 정점 인덱스를 동적 배열 멤버로 선언
    vector<Texture> textures; // Mesh 에서 사용할 텍스쳐들을 동적 배열 멤버로 선언

    // 생성자 함수 선언 및 구현
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        // 클래스로부터 파생된 인스턴스 객체 포인터(this)를 통해, 동적 배열 멤버변수들을 초기화함.
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 생성자 매개변수로부터 필요한 정점 데이터들을 전달받아 초기화했으므로,
        // 정점 데이터 관련 버퍼 객체 생성 및 데이터 해석 방식을 설정해 줌.
        setupMesh();
    };

private:

    // VBO, VAO, EBO 등 정점 데이터 관련 버퍼 객체 생성 및 데이터 해석 방식 설정하는 멤버 함수
    void setupMesh()
    {

    }
};

#endif // !MESH_H

// 원래 헤더파일에는 prototype 만 선언해서 정리해두는 게 정석이지만,
// 위의 Mesh 클래스처럼 코드 정리가 번거로울 경우, 그냥 헤더파일에 구현부까지 다 때려넣는 경우가 더 많음.
