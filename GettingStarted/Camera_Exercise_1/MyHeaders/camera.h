#ifndef CAMERA_H
#define CAMERA_H
/*
    camera.h 헤더파일에 대한 
    헤더가드 처리를 위해 전처리기 선언
*/

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// GLFW 키 입력 메서드와는 별개로 독립적인 카메라 이동에 관한 키 입력 상태를 관리하기 위해 선언한 ENUM
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// 카메라 이동, 각도, 줌 관련 초기값을 심볼릭 상수로 선언
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // 현재 카메라 위치 및 방향벡터 관련 멤버변수
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // 현재 카메라 방향벡터 계산에 사용되는 오일러 각 멤버변수
    float Yaw;
    float Pitch;
    // 현재 카메라 이동, 줌 관련 멤버변수
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // glm::vec3 타입으로 매개변수를 전달받는 생성자 오버로딩
    // 참고로, : 옆에는 매개변수와 무관하게 초기화할 수 있는 멤버변수 초기화 리스트 
    // (멤버변수 초기화 리스트 관련 https://github.com/jooo0922/raytracing-study/blob/main/InOneWeekend/InOneWeekend/ray.h 참고!)
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // float 타입으로 매개변수를 전달받는 생성자 오버로딩
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // 현재 카메라의 위치, 타겟, 카메라 업 벡터를 가지고 LookAt 행렬(= 뷰 행렬)을 계산하여 반환
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // 매 프레임마다 키 입력에 대한 현재 클래스에서만 사용되는 독립적인 Enum 을 입력받아 카메라 이동 처리 
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

        // 카메라가 xz축 평면 상에서만 이동하고 y축 수직이동은 못하게 막음.
        Position.y = 0.0f;
    }

    // 매 프레임마다 마우스 이동량(offset)을 입력받아 현재 카메라 오일러 각 재계산
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 카메라 공간 로컬 축(카메라 방향벡터, right 벡터, up 벡터) 업데이트
        updateCameraVectors();
    }

    // 매 프레임마다 마우스 수직방향 스크롤 이동량을 입력받아 현재 카메라 zoom 값 재계산
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // 현재 카메라 오일러 각을 기준으로 카메라 공간 로컬 축(카메라 방향벡터, right 벡터, up 벡터) 업데이트
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // 카메라 방향벡터 변경에 따른 카메라 right 벡터와 up 벡터 재계산
        // 참고로, 카메라 앞쪽 방향벡터와 월드공간 UP 벡터를 외적하면 '카메라 right 벡터' 가 나온댔지?
        // 또한, 카메라 right 벡터는 ProcessKeyboard() 에서 카메라 좌우 이동 시 방향벡터로 사용되므로, 길이를 1로 정규화해야 일정한 속도로 좌우 이동됨!
        Right = glm::normalize(glm::cross(Front, WorldUp)); 
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif

/*
    왜 yaw 초기각을 -90도로 설정했을까?

    yaw 초기각을 0도로 설정하면
    월드 공간에서 카메라 방향벡터가 양의 x축을 향하게 됨.

    우리는 월드 공간에서 카메라 방향벡터가 x축을 향하는 게 아닌,
    원점을 바라보도록 초기화하고 싶은 게 목적임!

    이를 달성하려면,
    카메라 월드공간 위치 z값이 양수라고 가정한다면,
    카메라 방향벡터는 '월드 공간에서 음의 z축'을 향하도록 초기화해야
    카메라가 월드 공간 상의 원점을 바라볼 수 있음.

    그러나, 여기서 말하는 '카메라 방향벡터' 란,
    카메라가 원점인 '뷰 좌표계 상의 카메라 방향벡터' 를 칭하므로,
    실제 월드 공간상의 카메라 방향벡터와 정반대 방향으로 계산되어 있음.

    그래서 LearnOpenGL 본문에서도
    "'direction vector' 라는 이름이 실제 월드공간에서 카메라가 바라봐야 할 방향의
    정반대 방향을 가리키다보니 좋은 이름은 아닐 수 있다"
    고 명시하고 있음.

    어쨋든, 카메라 월드공간 위치의 z값이 양수인 상태에서
    카메라가 월드 공간의 원점을 바라보려면,
    카메라가 '월드 공간의 음의 z축'을 바라봐야 하고,

    이는 월드 공간의 반대방향으로 변환되는 뷰 좌표계 상에서는
    '뷰 좌표계의 양의 z축'을 바라보는 것에 해당하기 때문에,
    결과적으로 '뷰 좌표계 상의 카메라 방향벡터' 는
    '월드 공간 상의 카메라 방향벡터'와 정반대 방향으로 뒤집어서 계산해줘야 함!

    이 전제 하에,
    yaw 축 각도가 -90도 회전하게 되면,
    '뷰 좌표계 상의 카메라 방향벡터'는 '양의 z축'을 바라보게 되고,
    이는 '월드 공간 상의 카메라 방향벡터'가 정반대 방향으로 뒤집어져서 '음의 z축'을 바라보는 것과 같지?

    따라서, 카메라 방향벡터의 yaw 각도를 -90도로 설정하면,

    월드 공간 상의 카메라 위치의 z값이 양수인 상태에서
    월드 공간 상의 카메라 방향벡터가 음의 z축을 바라보게 되는 것이고,

    이는 월드 공간 상의 카메라가 원점을 바라보도록 초기화한 것이라고 볼 수 있겠지!
*/

/*
    LookAt 행렬(glm::lookAt())로 뷰 행렬 만들기

    'LookAt 행렬'이라는 개념이 약간 생소할 수 있음.

    'LookAt' 이라는 개념 자체는 카메라가 어느 지점을 바라보도록 target 을
    설정하는 값 정도로 Three.js 에서 사용해본 적이 꽤 있었지.

    그러나, 사실 이 'LookAt' 이라는 개념은
    'LookAt 행렬'이라는 것에서 파생된 것인데,
    'LookAt 행렬'은 놀랍게도 '뷰 행렬'의 일종이라고 보면 됨.

    즉, 본질적으로 'LookAt 행렬'은
    카메라의 이동과 회전을 계산해주는,
    카메라가 움직이고자 하는 방향과 정반대 방향으로
    Scene 안의 모든 오브젝트들을 역변환시키는 '뷰 행렬' 이라는 것!

    일반적으로 뷰 행렬은
    카메라가 이동 및 회전하고자 하는
    이동행렬과 회전행렬의 역행렬을 곱해서 계산했었지? (게임수학 p.347 ~ p.350 참고)

    LookAt 행렬도 이와 다르지 않음.
    카메라의 이동행렬과 회전행렬의 역행렬을 계산하여 서로 곱하는 방식임.

    그런데, LookAt 행렬은 회전행렬을 계산할 때,
    glm::rotate() 를 사용하는 것이 아니라,
    1. 카메라 방향벡터(카메라 위치 - 카메라 타겟)
    2. 카메라 right 벡터 (월드 up 벡터와 카메라 방향벡터 외적)
    3. 카메라 up 벡터 (카메라 방향벡터와 카메라 right 벡터 외적)
    3개의 벡터를 계산하고, 이 3개의 벡터를 정규화한 다음,
    이 3개의 벡터를 회전이 적용된 기저축으로 삼는 회전행렬을 만듦.

    회전행렬은 회전이 적용된 로컬 축 벡터를
    열 벡터로 꽂아넣어 만든 것이라고 게임수학에서 배웠었지? (p.344)
    이것과 완전 동일한 원리인 것임.

    현재 카메라를 중심으로한 3개의 직교 벡터를 정의한 뒤,
    이를 회전이 적용된 로컬 축 벡터로 삼아 열 벡터로 꽂아넣으면
    현재 카메라의 회전행렬이 되는 것이지!

    다만, 이 회전행렬에는 카메라가 어느 방향을 바라보고 있는지,
    즉, target 에 대한 정보를 갖고있는 로컬 축(1. 카메라 방향벡터)을
    포함하고 있기에, 일반적인 회전행렬과 다르게 카메라의 'LookAt' 을
    구현할 수 있는 아주 특별한 회전행렬이라고 볼 수 있음!

    한 편, 이동행렬은 어떻게 구하냐?
    이건 아주 간단함. 그냥 카메라 위치 벡터를 카메라의 이동벡터 삼아서
    그 이동벡터를 가지고 이동행렬을 만들어버리면 됨.

    단, 카메라 뷰 행렬은 언제나 그렇듯, 카메라가 회전 및 이동하고자 하는 방향과
    정반대 방향의 변환을 적용해줘야 하기에, 회전행렬과 이동행렬 모두 '역행렬'로 계산해줘야 함.

    이때, 이동행렬은 이동벡터의 각 컴포넌트를 음수로 뒤집어주면(negate) 되고,
    회전행렬은 직교행렬이라면(즉, 회전행렬의 각 열벡터가 직교하는 행렬) 전치행렬과 역행렬이 같으므로,
    회전행렬의 대각성분을 기준으로 뒤집어주면(즉, 전치행렬을 구해주면), 그것이 곧 회전행렬의 역행렬이 되어버림.
    (이 내용도 게임수학 p.349 에 나와있음.)

    그렇게 역행렬로 계산된 회전행렬과 이동행렬을 곱한 것이
    https://learnopengl.com/Getting-started/Camera 에 나온 LookAt 행렬을 구하는 공식이 된 것임!

    그리고, glm::lookAt() 함수는
    카메라 현재 위치벡터, 카메라 target 벡터, 월드공간 up 벡터를 인자로 전달하면
    내부에서 자동으로 LookAt 행렬을 계산하여 반환해줌!
*/

/*
    카메라 방향벡터 계산 관련 유의사항

    LearnOpenGL 본문에서 삼각법으로 카메라 방향벡터 계산 시,
    direction.x, z 에 cos(pitch) 가 곱해졌던 이유는 직관적으로 보면 명확함.

    yaw 삼각형의 빗변의 길이가 cos(pitch) 와 일치하도록 그려져야 하기 때문!

    즉, yaw 삼각형의 빗변의 길이가 1임을 가정하고,
    cos(yaw) 와 sin(yaw) 를 방향벡터의 x, z 값으로 정할 수 있었던 것이지만,
    결국 최종적으로는 pitch 삼각형에서의 빗변의 길이가 1로써 최종 방향벡터와 길이가 같아야 함.

    pitch 삼각형의 빗변과 최종 방향벡터의 길이가 같아야 하고,
    pitch 삼각형의 빗변의 길이가 1이어야 한다면,
    yaw 삼각형의 빗변의 길이가 동시에 1일 수는 없다!

    따라서, yaw 삼각형의 빗변의 길이는 그에 맞춰서 cos(pitch) 만큼으로 줄어들 수밖에 없다!

    그렇기 때문에 yaw 삼각형의 빗변의 길이를 결정하는 밑변과 높이인 cos(yaw), sin(yaw)
    즉, direction.x, z 의 값에 cos(pitch) 만큼의 가중치가 곱해져서,
    최종적으로는 yaw 삼각형의 빗변의 길이가 cos(pitch) 로 줄어들게 된다!

    이것이 LearnOpenGL 본문에서 xz sides 가 cos(pitch) 의 영향을 받는다고 말한 것의 의미임!
*/

/*
    LookAt flip 현상 방지

    LearnOpenGL 본문에서 봐서 알겠지만,
    LookAt 행렬을 구하는 과정에서 카메라 right 벡터를 계산해야 하는데,

    카메라 right 벡터는 카메라 방향벡터와 월드공간 Up 벡터의 외적으로 계산했었지?

    문제는, 이때 카메라 방향벡터와 월드공간 Up 벡터가 서로 평행해진다면,
    평행한 두 벡터의 외적의 결과가 아무런 방향을 가지지 않는 '영 벡터'가 나와버림.

    즉, right 벡터가 영벡터가 되어버리면 LookAt 행렬을 정확하게 계산할 수 없음.

    따라서, 이러한 상황이 되는 것을 방지하려면,
    pitch 축 각도가 -90도보다 작아져서는 안되고, 90도보다 커져서도 안됨.
    만약에 그렇게 되는 순간 카메라 방향벡터가 월드공간 Up 벡터와 평행해짐.

    이러한 제한사항을 코드로 구현하기 위해,
    pitch 축 각도를 -89도 에서 89도 사이로 안전하게 clamping 한 것!
*/