#version 330 core

out vec4 FragColor;

// vertex shader 단계에서 전달받는 입력 interface block 선언
in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
} fs_in;

// OpenGL 에서 전송해 줄 uniform 변수들 선언
uniform sampler2D diffuseTexture; // 바닥 평면 텍스쳐 (0번 texture unit 에 바인딩된 텍스쳐 객체 샘플링)
uniform samplerCube shadowMap; // omnidirectional shadow map 텍스쳐 (1번 texture unit 에 바인딩된 큐브맵 텍스쳐 객체 샘플링)

uniform vec3 lightPos; // 광원 위치 > 조명벡터 계산에서 사용
uniform vec3 viewPos; // 카메라 위치 > 뷰 벡터 계산에서 사용

uniform float far_plane; // [0, 1] 사이로 정규화된 '광원 ~ 각 프래그먼트 사이의 월드공간 거리값'을 [0, far_plane] 사이의 거리값으로 복구할 때 사용
uniform bool shadows; // point shadow 활성화 여부 상태값

// omnidirectional shadow map 큐브맵으로부터 샘플링할 현재의 방향벡터(광원 ~ 현재 프래그먼트)에 적용할 offset 벡터들을 정적 배열에 초기화
vec3 gridSamplingDisk[20] = vec3[](vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1), vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1));

// 현재 프래그먼트가 그림자 안에 있는지 여부를 반환해주는 함수
float ShadowCalculation(vec3 fragPos) {
  // '광원 ~ 현재 프래그먼트 사이'의 월드공간 벡터 계산
  vec3 fragToLight = fragPos - lightPos;

  // 현재 프래그먼트의 깊이값은 '광원 ~ 현재 프래그먼트 사이'의 거리값으로 계산
  float currentDepth = length(fragToLight);

  /* PCF 알고리즘 적용 (자세한 설명 하단 참고) */

  // 누산할 shadow 값 초기화
  float shadow = 0.0;

  /*
    omnidirectional shadow mapping 에서 사용되는 깊이값(== 거리값, currentDepth)의 범위는 
    [near_plane, far_plane] 으로 더 커졌기 때문에,
  
    shadow bias 로 적용할 값도 더 크게 잡아줄 것.
  */
  float bias = 0.15;

  // PCF 알고리즘을 위해 '광원 ~ 현재 프래그먼트' 방향벡터의 주변을 샘플링할 횟수 -> '광원 ~ 현재 프래그먼트' 방향벡터에 적용할 offset 벡터(== vec3 gridSamplingDisk[20]) 개수와 일치
  int samples = 20;

  // 카메라에서 현재 프래그먼트와의 거리 계산
  float viewDistance = length(viewPos - fragPos);

  // 현재 방향벡터에 더해줄 offset 벡터의 길이 계산 (-> 카메라와의 거리가 멀수록 offset 벡터가 더 길어지도록 계산)
  float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

  // 샘플링 횟수만큼 반복문 순회
  for(int i = 0; i < samples; i++) {
    // 현재 방향벡터에 offset 벡터를 더한 주변 방향벡터들로 큐브맵에 저장된 깊이값 샘플링
    /*
      큐브맵은 방향벡터 만으로도 샘플링이 가능하므로,
      굳이 방향벡터의 길이를 1로 정규화하지 않아도 된다고 했었지?
      https://github.com/jooo0922/opengl-study/blob/main/AdvancedOpenGL/Cubemaps/MyShaders/skybox.fs 참고

      또한, offset 벡터의 길이를 결정하는 diskRadius 값이
      카메라에서 멀어질수록 커지도록 계산되기 때문에,

      현재 카메라에서 멀리 떨어진 프래그먼트일 수록,
      현재 방향벡터에서 더 멀리 떨어진 방향벡터를 사용해서 큐브맵을 샘플링하게 됨.

      -> 이렇게 함으로써, 카메라에서 멀리 떨어진 프래그먼트일 수록,
      현재 프래그먼트 방향에서 더 멀리 떨어진 방향에 존재하는 큐브맵 깊이값을 fetch 해와서
      shadow test 를 진행한 결과를 누산하게 될 것이고,

      shadow map 상에서 더 멀리 떨어진 깊이값과의 shadow test 결과를
      누산하여 평균을 내게 되면, 그림자 영역이 더 뭉게지고 부드러워질 것임.

      반면, 카메라에서 더 가까이 존재하는 프래그먼트일 수록,
      현재 방향벡터에서 아주 가까운 방향벡터를 사용해서 큐브맵을 샘플링하게 됨.

      -> 이는 카메라에서 멀리 떨어진 경우와는 반대로,
      오히려 현재 방향벡터와 유사한 방향벡터들로 샘플링 깊이값으로 shadow test 를 하게 될 것이고,
      그 결과를 누산하여 평균을 내면, 사실상 현재 방향벡터로 샘플링한 깊이값을 shadow test 한 결과와
      거의 동일한 값을 도출하게 될 것임.

      즉, 카메라에서 더 가까운 프래그먼트일 수록,
      PCF 알고리즘을 적용하지 않은 것처럼, 즉, 더 sharp 하고 각진 그림자로 렌더링될 것임.
    */
    /*
      참고로, 방향벡터에 다른 offset 벡터를 더한다고 하면,
      방향벡터는 '방향'만이 의미있는 벡터이니 '방향'은 그대로이고, '길이'만 바뀌는 것뿐 아닐까라고
      언뜻 보기에는 착각할 수 있음.

      그러나, 이는
      원본 방향벡터에 '스칼라 실수를 곱할 때' 성립되는 개념이고,
      
      원본 방향벡터에 '또다른 offset 벡터를 더할 떄'에는
      원본 방향벡터의 각 컴포넌트들이 일정한 비율로 uniform scaling 되는 것이 아니기 때문에,
      '아예 다른 방향을 갖는 벡터'가 도출된다는 것을 알 수 있음!
    */
    float closestDepth = texture(shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;

    // 큐브맵에서 가져온 깊이값은 [0, 1] 사이로 정규화된 '광원 ~ 가장 가까운 프래그먼트 사이의 거리값' 으로 저장되었기 때문에, 이를 다시 [0, far_plane] 범위의 값으로 복구시킴
    closestDepth *= far_plane;

    // 큐브맵에서 샘플링한 깊이값(closestDepth)과 현재 프래그먼트의 깊이값(currentDepth)을 비교하여,
    // 현재 프래그먼트가 그림자 영역 내에 존재할 때, shadow 값을 누산함
    if(currentDepth - bias > closestDepth) {
      shadow += 1.0;
    }
  }

  // 누산된 shadow 결과값을 샘플링 횟수로 나눠서 평균을 구함 -> '그림자 영역에 얼마만큼 포함되는지(Percentage-Closer)'의 비율값을 구할 수 있음!
  // (이때, 나눗셈 연산을 위해 samples 정수형 값을 float 타입으로 형변환)
  shadow /= float(samples);

  return shadow;
}

void main() {
  // 바닥 평면 텍스쳐를 샘플링하여 diffuse color 값 저장
  vec3 color = texture2D(diffuseTexture, fs_in.TexCoords).rgb;

  // 프래그먼트에 수직인 노멀벡터 정규화
  vec3 normal = normalize(fs_in.Normal);

  // 조명색상을 어두운 회색으로 초기화
  vec3 lightColor = vec3(0.3);

  /* blinn-phong 반사 모델 기준으로 조명 계산 */

  /* ambient 성분 계산 */
  float ambientStrength = 0.3; // ambient 강도
  vec3 ambient = ambientStrength * lightColor; // ambient 강도에 조명 색상 곱해서 최종 ambient 성분값 계산

  /* diffuse 성분 계산 */
  // 조명계산에 사용되는 모든 방향벡터들은 항상 정규화를 해줄 것! -> 그래야 내적계산 시 정확한 cos 값만 얻을 수 있음!
  vec3 lightDir = normalize(lightPos - fs_in.FragPos); // 조명벡터 (프래그먼트 위치 ~ 광원 위치)
  float diff = max(dot(lightDir, normal), 0.0); // 노멀벡터와 조명벡터 내적 > diffuse 성분의 세기(조도) 계산 (참고로, 음수인 diffuse 값은 조명값 계산을 부정확하게 만들기 때문에, 0.0 으로 clamping 시킴)
  vec3 diffuse = diff * lightColor; // diffuse 조도에 조명 색상 곱해서 최종 diffuse 성분값 계산

  /* specular 성분 계산 */
  vec3 viewDir = normalize(viewPos - fs_in.FragPos); // 뷰 벡터 (카메라 위치 ~ 프래그먼트 위치)
  float spec = 0.0; // specular 조도 선언 및 초기화

  // 뷰 벡터와 조명벡터 사이의 halfway 벡터 계산 (두 벡터의 합 -> 두 벡터 사이를 가로지르는 하프 벡터 (<셰이더 코딩 입문> p.222 참고))
  vec3 halfwayDir = normalize(lightDir + viewDir);

  // specular 성분의 조도 계산
  // 프래그먼트 지점의 normal 벡터와 halfway 벡터를 clamping 내적함
  // 내적값을 32제곱 > 32는 shininess 값으로써, 값이 클수록 highlight 영역이 정반사되고, 값이 작을수록 난반사됨.
  // Blinn-Phong 모델에서는 동일한 조건 하에 기본 Phong 모델에 비해 내적값이 더 작게 계산되기 때문에,
  // 일반적인 관례상 기본 Phong 모델보다 2~4배 큰 shininess(광택값)을 사용한다.
  spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);

  vec3 specular = spec * lightColor; // specular 조도에 조명 색상을 곱해 specular 성분값 계산

  // 현재 프래그먼트의 월드공간 좌표계를 매개변수로 전달하여 그림자 영역 내에 존재하는지 여부를 판단
  // (shadows(point shadow 활성화 상태값)값에 따라 해당 월드공간 프래그먼트가 그림자 영역 내 존재하는지 여부를 계산할 지 결정!)
  float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;

  // 3가지 성분을 모두 더한 뒤, 바닥 평면 텍스쳐 색상값(diffuse color)를 곱하여 최종 색상 계산
  /*
    조명 성분 중, diffuse 및 specular 성분에만
    shadow 값(그림자 영역 내에 얼만큼 속하는지)을 적용함

    why? 그림자 영역도 마찬가지로
    환경광(ambient)의 영향을 받기 때문에,
    완전히 검정색인 그림자는 존재하지 않음.

    따라서, ambient 성분값은 그대로 보존하고,
    diffuse 과 specular 성분에만 shadow 값을 곱해준 것!
  */
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

  FragColor = vec4(lighting, 1.0);
}

/*
  shadow bias


  shadow bias 란, 
  shadow acne 라고 불리는 artifact 를 해결하기 위해
  계산하는 보정값이라고 보면 됨.


  shadow map 은 일정한 해상도가 존재하는 텍스쳐에 불과하기 때문에,
  모든 프래그먼트의 좌표값(projCoords.xy)에 1:1 로 맵핑되는 texel 들이 존재하는 게 아님.

  즉, 여러 개의 프래그먼트 좌표값이
  shadow map 으로부터 동일한 texel 을 샘플링하게 될 확률이 높다는 뜻이지!


  만약에 이렇다면 어떤 상황이 발생하게 될까?
  서로 다른 프래그먼트들이 shadow map 으로부터 동일한 깊이값(closestDepth)을 샘플링하여,
  자신의 깊이값(currentDepth)과 비교하게 될 것임.

  이것은 일반적인 상황에서는 큰 문제가 되지는 않겠지만,
  만약 조명벡터와 각 프래그먼트들의 표면 사이가 '기울어져' 있다면,
  LearnOpenGL 본문에 삽입된 일러스트처럼,

  샘플링하려는 shadow map 의 texel 이 각 프래그먼트의 표면과
  지그재그로 기울어진 상태로 샘플링되는 꼴이 되어버림!
  (혹은, 각 프래그먼트의 표면을 기울여진다고 상상해보면, 
  그것도 마찬가지로 shadow map 의 texel 과 지그재그로 교차되는 꼴이 될 것임!)


  이렇게 되면, 해당 일러스트에서 보는 것과 같이,
  각 프래그먼트의 깊이값이 다르더라도, 
  해상도의 한계로 인해 shadow map 으로부터 동일한 깊이값을 샘플링해와서
  비교할 수 밖에 없다보니, 
  
  서로 가까이에 있는 프래그먼트들이라 하더라도,
  어떤 부분은 그림자 영역 내에 있는 것으로 판정되고(일러스트의 검은색 부분),
  어떤 부분은 그림자 영역 밖에 있는 것으로 판정되는(일러스트의 노란색 부분) 것임


  이로 인해 약간의 Moiré 같은 패턴이 생기게 되는데,
  이를 'shadow acne' 라고 함.


  이를 해결하기 위한 방법은 아주 간단한데,
  각 프래그먼트들의 깊이값(currentDepth)들을 전반적으로
  광원의 위치에 더 가깝게 당겨줌으로써, 

  동일한 shadow map 의 깊이값(closestDepth)을 지그재그 형태로 샘플링하더라도,

  두 깊이값 사이의 차이가 확연하게 나지 않을 정도면,
  항상 현재 프래그먼트의 깊이값이(currentDepth) 더 광원에 가까운 것으로 판정하는,
  즉, 그림자 영역 밖에 있는 것으로 판정되도록 깊이값을 보정하는 것이지.


  이럴 떄 사용하는 보정값이 
  'shadow bias' 라고 보면 됨.


  그런데, shadow bias 값을 계산할 때에는
  몇 가지 주의할 사항들이 존재하는데,

  첫째로, 각 프래그먼트 표면과 조명벡터 사이의 각도가 
  전부 다르기 때문에, 각 프래그먼트와 shadow map 의 texel 이
  어느 정도로 지그재그 되는지 그 각도마저도 모두 다름.

  따라서, 그 각도에 대한 내적값을 이용해서,
  각도에 따라 shadow bias 값을 프래그먼트마다 다르게 적용할 수 있도로 함.


  둘째로, shadow bias 값이 너무 커지면,
  확연히 그림자 영역 내에 존재해야 할 프래그먼트들 조차
  광원에 더 가까워지다보니,

  그림자 영역 밖에 있는 것으로 판정되는 이슈가 발생함.

  이로 인해, 마치 그림자가 '떼어진 것처럼 보이는' 현상이 발생하는데
  이를 Peter panning 이라고 함.

  이 현상이 발생하지 않으려면,
  shadow bias 를 계산할 때, 
  일정 범위 내에서 clamping 되도록 범위를 지정해줘야 함. 
*/

/*
	PCF 알고리즘


	PCF 알고리즘(Percentage-Closer Filtering)은 
	shadow mapping 으로 생성한 그림자의 외곽선 부분을 
	더 부드럽게 렌더링해주는 기법 중 하나임.

	기본적으로 shadow mapping 으로 생성한 그림자는
	마치 aliasing 현상처럼 그림자의 외곽선 부분에
	blocky 한 계단현상이 나타나게 됨.

	
	이를 좀 더 부드럽게 렌더링하기 위해,
	shadow map 으로부터 현재 프래그먼트의 uv 좌표값(projCoords.xy)을 중심으로
	주변 texel 들의 깊이값을 n 번 샘플링하여 누산하고,
	그 누산값을 n 만큼 나눠서 '평균값'을 계산하게 됨.


	이렇게 되면,
	기존의 shadow 값이 0.0 또는 1.0 으로 나왔던 것들이,
	'평균값'으로 계산되어 0.xxxx... 로 나오게 되겠지!

	즉, 기존의 shadow 값으로 그림자 영역 안에 '있다 / 없다' 만 알 수 있었다면,
	평균값으로 누산된 shadow 값은 그림자 영역 안에 '얼마나 있는지'를 알 수 있게 되는 것임!


	즉, 해당 프래그먼트 지점이
	광원으로부터 '얼마나 가까운 지'를 '비율로써' 나타낼 수 있으므로,
	외곽선 영역으로 갈수록, 주변 texel 의 깊이값을 샘플링하여 누산된 평균값을 구한다면,
  `shadow 영역이 더 부드럽게 렌더링될 수 있을 것임.

	이처럼, 광원에 얼마나 '가까운 지'를 '비율로써' 나타낸다는 의미에서
	Percentage-Closer Filtering 기법이라고 부르는 것임.


	만약, offset 으로 사용하는 단위 texel 의 크기인 texelSize 를 조절하고,
	주변 texel 샘플링 횟수를 증가시킨다면, 훨씬 더 부드러운 그림자를 렌더링 할 수 있을 것임!
*/