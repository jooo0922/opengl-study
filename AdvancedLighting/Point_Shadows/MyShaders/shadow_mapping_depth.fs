#version 330 core

void main() {
  /*
    이전 파이프라인 (== primitive assembly) 단계에서 수행된 
    원근분할의 결과인 NDC 좌표의 z 값이 깊이 버퍼에 자동으로 기록되기 때문에,

    아래 코드를 굳이 작성해 줄 필요는 없음.

    그러나, 현재 프래그먼트 쉐이더 단계에서
    아래와 같은 원리로 깊이 버퍼가 기록된다는 것을 명시하기 위해
    주석처리를 남겨둔 것임. 
  */
  // gl_FragDepth = gl_FragCoord.z;
}