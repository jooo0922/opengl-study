#version 330 core

out vec4 FragColor;

// 버텍스 쉐이더로부터 보간되어 전달된 큐브맵 샘플 좌표값
// 큐브맵은 2D 텍스쳐와 달리, 3차원 방향벡터를 사용해서 샘플링함!
in vec3 TexCoords;

/* 큐브맵 텍스쳐가 바인딩된 texture unit 을 전달받는 uniform samplerCube 선언 */
uniform samplerCube skybox;

void main() {
  // 큐브맵 텍스쳐를 방향벡터로 샘플링
  // 참고로, 이때 사용되는 방향벡터 TexCoords 는 '버텍스의 오브젝트공간 좌표(= local position)'과 동일함!
  // 또한, textureCube() 함수는 방향벡터 TexCoords 의 길이를 1로 정규화하지 않더라도, 방향값만 갖고 있으면 알아서 그 방향에 맞는 텍셀값을 큐브맵으로부터 fetch 해옴!
  FragColor = texture(skybox, TexCoords);
}