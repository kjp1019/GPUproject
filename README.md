# 3D 그래픽 렌더링

## 프로젝트 소개
이 프로젝트는 C++ / OpenGL / Assimp 기반의 3D 그래픽 렌더링 프로그램으로, 플레이어가 큐브 캐릭터를 조작하여 차량과 상호작용하는 시뮬레이션입니다.
스포츠카 A와 충돌 시 산산조각 폭발(Geometry Shader) 과 파티클(연기·파편) 효과가 발생하며, 다른 스포츠카 B는 탑승하여 직접 운전할 수 있습니다.

## 기술 스택
- 언어 & 그래픽 API : C++17, OpenGL 3.3 Core
- 외부 라이브러리
  - GLFW
  - GLAD
  - GLM
  - Assimp
  - stb_image

## 주요 기능
- 플레이어 캐릭터 (Cube)
  - 기본 이동 (WASD)
  - 점프 (Space)
  - 대시 이동 (Shift)
  - 3인칭 시점 카메라 마우스 회전 지원
- 스포츠카 A (폭발형 차량)
  - 큐브 또는 탑승 차량이 일정 거리 내 접근 → 폭발
  - Geometry Shader 기반 산산조각 연출
  - CPU 기반 파티클 시스템으로 연기/파편 효과 추가
- 스포츠카 B (탑승 가능 차량)
  - 플레이어가 접근 후 E 키로 탑승 / 하차
  - 전후진 및 좌우 조향 가능
  - 차량 충돌 시 주변 Car A가 폭발
- Particle System
  - 수천 개 파티클을 GPU에 전송 후 화면에 점으로 렌더링
  - 중력 및 속도 적용 → 파편이 자연스럽게 퍼지는 효과
  - 랜덤 분포 기반으로 매번 다른 폭발 연출
- 리셋 기능
  - R 키 입력 시 모든 상태(큐브 위치, 차량, 폭발, 카메라, 파티클)가 초기화

### 조작 방법
| 키             | 기능         |
| ------------- | ---------- |
| W / A / S / D | 큐브 이동      |
| Space         | 점프         |
| Shift         | 대시         |
| E             | 차량 탑승 / 하차 |
| R             | 전체 리셋      |
| 마우스 이동        | 카메라 회전     |

## 프로젝트 구조

```
├─ main.cpp                # 메인 로직
├─ shader/
│   ├─ basic.vs / basic.fs
│   ├─ 9.2.geometry_shader.vs / fs / gs
│   ├─ particle.vs / particle.fs
├─ resources/
│   ├─ objects/
│   │   ├─ cube/cube.obj
│   │   ├─ sportscar/sportsCar.obj
│   ├─ textures/metal.png

```

## 실행 방벙
- mkdir build && cd build
- cmake ..
- make
- ./project
