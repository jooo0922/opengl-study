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
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};
#endif