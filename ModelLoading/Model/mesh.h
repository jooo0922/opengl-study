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
    glm::vec3 Bitangent; // 정점의 비탄젠트 벡터 > 노말맵 적용 시, TBN 행렬 계산에 사용됨.
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
    // VBO, VAO, EBO 등 정점 버퍼 객체의 참조 ID 를 저장할 멤버 선언
    // 정점 버퍼 데이터는 외부에 노출되어선 안되므로, encapsulation 처리
    unsigned int VAO, VBO, EBO;

    // VBO, VAO, EBO 등 정점 데이터 관련 버퍼 객체 생성 및 데이터 해석 방식 설정하는 멤버 함수
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO); // VAO(Vertex Array Object) 객체 생성
        glGenBuffers(1, &VBO); // VBO(Vertex Buffer Object) 객체 생성
        glGenBuffers(1, &EBO); // EBO(Element Buffer Object) 객체 생성

        glBindVertexArray(VAO); // VAO 객체 컨텍스트에 바인딩 > 재사용할 여러 개의 VBO, EBO 객체들 및 설정 상태 저장

        glBindBuffer(GL_ARRAY_BUFFER, VBO); // VBO 객체를 GL_ARRAY_BUFFER 버퍼 타입에 바인딩

        // Struct(구조체) 로 정점 데이터를 표현하는 장점 관련 하단 필기 참고
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW); // 정점 데이터를 VBO 객체에 덮어쓰기
    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // 이번에는 EBO 객체를 GL_ELEMENT_ARRAY_BUFFER 버퍼 타입에 바인딩
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW); // 인덱스 데이터를 EBO 객체에 덮어쓰기


        /* 각 정점 데이터 타입별 해석 방식 설정 */

        glEnableVertexAttribArray(0); // 0번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // position 데이터 해석 방식 설정
        
        // 정점 데이터 해석 방식 정의 시, Struct(구조체)의 또 다른 이점 하단 필기 참고
        glEnableVertexAttribArray(1); // 1번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)); // normal 데이터 해석 방식 설정

        glEnableVertexAttribArray(2); // 2번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords)); // uv 데이터 해석 방식 설정

        glEnableVertexAttribArray(3); // 3번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent)); // tangent 데이터 해석 방식 설정

        glEnableVertexAttribArray(4); // 4번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent)); // bitangent 데이터 해석 방식 설정

        glEnableVertexAttribArray(5); // 5번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs)); // Bone 인덱스 데이터 해석 방식 설정

        glEnableVertexAttribArray(6); // 6번 로케이션 attribute 변수 활성화
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights)); // Bone 가중치 데이터 해석 방식 설정

        glBindVertexArray(0); // VAO 객체에 저장해둘 설정을 끝마쳤으므로, OpenGL 컨텍스트로부터 바인딩 해제 
    }
};

#endif // !MESH_H

// 원래 헤더파일에는 prototype 만 선언해서 정리해두는 게 정석이지만,
// 위의 Mesh 클래스처럼 코드 정리가 번거로울 경우, 그냥 헤더파일에 구현부까지 다 때려넣는 경우가 더 많음.

/*
    정점 데이터를 버퍼에 덮어쓸 때, Struct(구조체)의 장점

    Struct(구조체)는 메모리 구조 상에 저장될 때,
    마치 배열처럼 요소들이 '연속적으로(sequential)' 저장됨!

    이게 무슨 말이냐면,
    Vertex 구조체의 멤버로 선언된 Position, Normal, TexCoords, ... 등의 데이터들이
    glm::vec3/2 타입으로 이루어져 있잖아?

    이럴 경우, 얘내들은 메모리 상에
    각 vec3/2 타입 멤버들의 컴포넌트(x, y, z)가
    마치 배열처럼 순차적으로 나란히 저장되어 있는 상태라는 말임!

    예를 들어, 아래와 같이 각 멤버들의 컴포넌트들이 나란히 저장되어 있다는 소리!

    Vertex vertex;
    vertex.Position  = glm::vec3(0.2f, 0.4f, 0.6f);
    vertex.Normal    = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(1.0f, 0.0f);
    // = [0.2f, 0.4f, 0.6f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f];

    이런 형태의 float 정적 배열 어디서 많이 보지 않았나?

    이전 예제까지 우리가 보통 정점 데이터 배열을 이렇게 생성했었잖아!
    float vertices[] = { ... }; 이런 식으로!

    이렇게 하드코딩된 raw 데이터와
    Struct 가 메모리 상에 저장되는 구조는 어차피 동일하기 때문에,
    정점 데이터 배열을 아주 쉽고 간결하게 표현할 수 있는
    좋은 자료구조라는 뜻이지!

    그래서, vector<Vertex> 타입의 동적 배열 vertices 의
    첫 번째 요소 주소값인 vertices[0] 만 넘겨주면,
    그 뒤에 나란히 저장되어 있는 정점 데이터 배열의 나머지 요소까지
    한방에 glBufferData() 함수로 전달할 수 있음.

    원래 배열 또한 내부적으로는 배열의 첫 번째 요소에 대한
    주소값을 갖는 포인터 변수와 동일한 것이니까!
*/

/*
    정점 데이터 해석 방식 정의 시, Struct(구조체)의 또 다른 장점

    glVertexAttribPointer() 함수의 마지막 인자인
    '정점 버퍼에서 데이터 시작 위치 offset' 을 아주 쉽게 계산할 수 있음!

    offsetof() 라는 함수처럼 보이지만 매크로 전처리기인 이 문법을 사용하면 됨!

    offsetof(s, m) 에서 첫번째 인자 s 는 구조체를, 
    두 번째 인자 m 은 그 구조체 안에 있는 멤버변수명을 전달함.

    이때, 계산된 값은 구조체 s 의 메모리 레이아웃 상의 첫 번째 위치에서
    멤버변수 m 이 얼만큼 떨어진 지점부터 저장되기 시작하는지, 
    즉, 메모리 레이아웃 상의 'offset' 값이라고 보면 됨!

    Vertex 구조체에서는 이 값 자체가 바로
    '정점 버퍼에서 데이터 시작 위치 offset' 인 셈!
*/