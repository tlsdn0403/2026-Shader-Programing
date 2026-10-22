# LOD / 컬링 자동 측정
2026-10-21, AMD Ryzen 7 6800H / AMD Radeon(TM) Graphics.
OpenGL driver: 3.3.0 Core Profile Context 22.20.24.10.240429.
현재 렌더러 소스를 별도 숨김 창 측정 프로그램으로 MSVC x64 /O2 /MD 빌드했습니다.
960x640, 구, 32열, 간격 3, 평면 격자. 객체 수마다 GenerateStressScene 후 camera.Frame({0,0,0},18)로 덮어써 카메라를 고정합니다.
카메라에 보이는 영역이 고정되므로 개수가 커질수록 화면 밖 객체도 늘어나는 실험입니다. 모든 객체가 보이는 상황으로 일반화할 수 없습니다.
LOD error=2, Blinn–Phong, time=0, floor/GI/voxel shadows/animation OFF.
모드 0=둘다OFF, 1=LOD, 2=컬링, 3=둘다ON. 매 반복 순서를 한 칸씩 순환합니다.
조건별 예열 20프레임 이후 60프레임씩 3회 측정, 총180개의 중앙값을 사용했습니다.
GPU 시간은 Render 전후 GL_TIMESTAMP 차이입니다. 동기화 렌더 시간은 CPU에서 Render 전부터 glFinish 완료까지의 벽시계 시간입니다. 순수 CPU 시간이나 편집기 FPS가 아닙니다.
UI, 화면 교환, 초기화 및 리소스 생성 시간은 제외합니다. 각 프레임에 glFinish를 사용하므로 일반 프레임 파이프라이닝과 다릅니다.
모두OFF는 객체별 draw 경로, 나머지 설정은 meshlet별 draw 경로입니다. 인덱스 버퍼 사용 등 경로 차이도 있어 단일 요인만의 비용으로 해석하지 않습니다.
원시 데이터: benchmark-raw.csv. gpu_ms, sync_wall_ms는 밀리초이며 submitted triangles와 draw calls는 객체 렌더링 통계입니다.
결과는 이 기기와 특정 장면에 한정됩니다. 전원/백그라운드 상태는 고정하지 않았고 GPU 수치 분산이 존재합니다.
측정 보조 프로그램은 AI가 구성·실행했으며, 사용자 본인이 수행한 프로파일링 경험으로 표기하지 않습니다.

## 통계 패널 캡처
capture-metrics.csv와 capture-metrics-raw.csv는 최신 캡처의 측정값입니다. 이전 benchmark-raw.csv와 측정 영역·방식이 다르므로 수치를 혼용하지 않습니다.
장면900x720, 창1280x720. 카메라 중심원점; 다중객체 pitch=.65 yaw=.55, 단일구 pitch=.28 yaw=.55. distance는 실제 카메라와 원점 사이 월드 거리입니다.
30회 예열 후 연속180프레임 표본의 중앙값, 조건별1회 실행. 전원 및 백그라운드 통제 없음. GL_TIMESTAMP로 장면 GPU 측정, CPU는 Render 호출 경과시간, 프레임은 UI 및 swap, glFinish 완료를 포함한 반복 벽시계시간입니다. FPS는1000/프레임중앙값이며 실시간 편집기 FPS가 아닙니다. CPU와GPU 시간은 겹칠 수 있어 단순 합산하지 않습니다.
캡처 보조창은 실제 Renderer와 Dear ImGui를 실행합니다. 측정완료 후 중앙값을 창 안에 표시해 기본 프레임버퍼를 캡처했습니다. UI는 원본 에디터가 아닌 AI 작성 측정 보조창입니다.
와이어프레임 확대 보기는 같은 렌더텍스처의 중앙영역을 확대해 추가 렌더링한 UI 이미지입니다. LOD를 강제하거나 원래 메시를 바꾸지 않았습니다.

가독성 수정: 기존 capture-metrics.csv의 중앙값을 그대로 사용하고, 동일 장면을 재렌더링해 큰 글씨 패널로 캡처했습니다. 숫자는 현재 재캡처 프레임의 순간값이 아닌 기존 기록입니다.
