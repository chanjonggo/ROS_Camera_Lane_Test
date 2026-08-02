# GIS를 활용한 ROS 동작방식 테스트 프로젝트

# ROS 2 실전 프로젝트 기획서: GIS 기반 경로 계획 및 실시간 텔레메트리 파이프라인 시스템

## 1. 프로젝트 개요 (Overview)

* **목적:** ROS 2의 핵심 통신 매커니즘(Action, Components, Intra-process Communication)을 활용하여, 실제 내비게이션/GIS 환경에서 요구되는 경로 계획 및 실시간 데이터 수집 백엔드 시스템을 직접 설계하고 구현합니다.
* **주요 기술 스택:** C++, ROS 2 (Humble/Iron/Jazzy 등), `rclcpp_action`, `rclcpp_components`, CMake, Python (Launch 시스템)

---

## 2. 시스템 아키텍처 (System Architecture)

```text
[ Client Node ] 
      │ (Goal: Start/Target Coordinates)
      ▼
[ Path Planner (Action Server) ] ──(Zero-Copy / Intra-process)──> [ Telemetry / Logger Node ]
      │ (Feedback: Current Step, Distance)                               │ (Saves to DB/File)
      ▼                                                                  ▼
[ Final Result / Status ]                                     [ Local Database (SQLite) ]

```

* **Path Planner (Action Server Component):**
* 클라이언트로부터 출발지/목적지 좌표를 받아 단계별 경로를 연산합니다.
* 연산 중 실시간 진행 상황을 퍼블리시하고, 취소(Cancel) 요청을 처리합니다.


* **Telemetry & Logger Node (Subscriber Component):**
* 경로 탐색 과정에서 발생하는 위치 및 상태 데이터를 구독하여 로컬 DB에 비동기 저장합니다.


* **Composition Container:**
* 위 두 노드를 단일 프로세스 컨테이너에 묶고, 프로세스 내 제로카피(`use_intra_process_comms:=true`)를 활성화하여 최적화합니다.



---

## 3. 인터페이스 설계 (`.action`)

시스템의 뼈대가 되는 커스텀 액션 인터페이스 명세 (`path_planner.action`)입니다.

```text
# ==========================================
# Goal (목표 요청)
# ==========================================
float64[] start_pos      # 출발지 좌표 [latitude, longitude] 또는 [x, y]
float64[] target_pos     # 목적지 좌표 [latitude, longitude] 또는 [x, y]
---
# ==========================================
# Result (최종 결과)
# ==========================================
bool success             # 경로 계획 성공 여부
float32 total_time       # 소요 총 시간 (초)
geometry_msgs/Point[] final_path  # 최종 생성된 전체 경로 좌표 리스트
---
# ==========================================
# Feedback (실시간 중간 보고)
# ==========================================
int32 current_step       # 현재 통과 중인 웨이포인트 인덱스
float32 distance_remaining # 목적지까지 남은 잔여 거리
geometry_msgs/Point current_pose # 현재 연산 중인 위치

```

---

## 4. 단계별 구현 로드맵 (Implementation Roadmap)

### Step 1: 인터페이스 패키지 구축

* `custom_action_interfaces` 패키지 생성
* `path_planner.action` 작성 및 CMakeLists.txt / package.xml 빌드 설정 구성
* `colcon build`를 통한 C++ 헤더 파일 자동 생성 검증

### Step 2: 코어 비즈니스 로직 구현 (Action Server)

* `rclcpp_node` 상속 구조 및 `NodeOptions` 적용
* `execute` 함수 내 경로 탐색 시뮬레이션 루프 구현
* `is_canceling()`을 통한 예외 처리 및 취소 로직 구현
* `goal_handle->publish_feedback()` 및 `succeed()` / `canceled()` 처리

### Step 3: 컴포넌트화 및 성능 최적화 (Components & Launch)

* `main` 함수 제거 및 `RCLCPP_COMPONENTS_REGISTER_NODE` 매크로 적용
* CMakeLists.txt를 `add_library` (SHARED) 형태로 전환
* 파이썬 런치 파일 작성
* `ComposableNodeContainer` 활용
* `use_intra_process_comms:=true` 옵션 설정



### Step 4: 예외 처리 및 통합 테스트

* 액션 클라이언트(`action client`) 구현 또는 `ros2 action send_goal` 명령어를 통한 통신 테스트
* 연산 중 의도적인 작업 취소(`Cancel`) 요청 테스트
* 서버 비정상 종료 방지 및 에러 코드 반환 검증