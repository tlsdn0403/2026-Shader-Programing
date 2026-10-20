# 동적 간접광과 클러스터 LOD 실험

이 프로젝트는 **UE Lumen / Nanite의 이식 또는 동일 구현이 아닙니다.** 현재 OpenGL 3.3 편집기에서 직접 읽고 수정할 수 있도록, 간접광과 가시 기하 감소라는 목적을 작은 연구 구현으로 추가했습니다. UE 소스는 복사하지 않았습니다.

## 사용 방법

1. `Run-ShaderLab.cmd` 또는 Visual Studio F5로 실행합니다.
2. 오른쪽 `Research` 탭에서 `Load GI study`를 누릅니다. 이 버튼은 현재 장면을 교체합니다.
3. `Voxel cone tracing GI`를 켜고 끄며 빨강·파랑 발광 큐브가 구와 바닥에 전달하는 색을 비교합니다. `Voxel direct shadows`는 직접광 그림자만 별도로 켜고 끕니다.
4. `Light` 탭에서 선택 객체의 색·`Emission`을 바꾸거나 `Object` 탭에서 위치를 바꾸면 복셀 데이터가 갱신됩니다. 카메라만 움직이면 기존 데이터를 재사용합니다.
5. `Load 64-object LOD study`는 64개의 구를 배치합니다. `Automatic mesh LOD`, `Cluster frustum culling`을 끄고 켜며 제출 삼각형 수를 비교합니다. 휠/드래그로 거리와 시점을 바꿀 수 있습니다.
6. Research 패널 아래로 스크롤하면 `Shaded`, `Indirect light only`, `Cluster colors`, `LOD colors`와 통계가 있습니다. LOD 색은 0=초록, 1=파랑, 2=노랑, 3=빨강입니다.

`LOD error`를 높이면 더 적은 삼각형으로 그리는 LOD를 선택하기 쉬워집니다. 이는 단순화 오차를 화면 크기로 환산한 선택 기준이며 실제 이미지 오차를 보장하지 않습니다. 클러스터 색은 각 메시 내부 번호 기준이라 다른 객체에서 반복됩니다. `Indirect light only`는 GI 체크 상태와 관계없이 간접광만 표시합니다.

## 코드와 파이프라인

| 파일 | 역할 |
|---|---|
| `ShaderLab/Light.h`, `.cpp`, `Shaders/Light.glsl` | Directional/Point/Spot 방향·색·거리 및 원뿔 감쇠 |
| `ShaderLab/VoxelGI.h`, `.cpp` | CPU 도형 복셀화, 방사휘도/점유율 생성, 변경 감지, 3D 텍스처 업로드 |
| `ShaderLab/Shaders/Research.glsl` | mip 기반 cone tracing 간접광, 복셀 그림자 추적 |
| `ShaderLab/ClusterMesh.h`, `.cpp` | meshoptimizer를 이용한 4개 LOD 및 meshlet 생성, LOD 선택, 프러스텀 컬링 |
| `ShaderLab/Renderer.cpp` | 객체별 draw, GI 데이터 바인딩, 통계와 비동기 GPU 시간 쿼리 |
| `ShaderLab/SceneSettings.cpp` | 발광체 비교 장면과 64개 객체 비교 장면 |
| `ShaderLab/EditorUI.cpp` | Research/Object 탭 및 조절 UI |
| `ShaderLab/ShaderLibrary.cpp` | 각 `.fs`의 공통 `Research.glsl` include 확장과 재컴파일 |

각 효과는 기존 `.vs` / `.fs` 파일을 유지합니다. 공통 함수만 `.glsl`에 둡니다. GLSL 자체 include 기능이 아니라 C++ 로더가 지정된 include 문장을 확장합니다. `Reload shaders`로 공통 파일 변경도 반영됩니다.

간접광 경로: 장면 변경 → 64³ 복셀의 RGB 방사휘도/A 점유율 생성 → 3D 텍스처 mip 생성 → 프래그먼트마다 반구 방향 5개 cone 추적 → 기존 직접광에 간접광을 더함 → 톤 매핑.

기하 경로: 초기화 시 메시의 중복 정점 정리 → meshoptimizer 단순화로 LOD 4단계 생성 → 각 LOD를 최대 64정점/124삼각형 meshlet으로 분할 → 프레임마다 객체 전체의 LOD 선택 → 각 meshlet의 경계 구와 프러스텀 6개 평면 검사 → 보이는 meshlet만 인덱스 draw.

## 현재 구현의 한계

- GI는 Voxel Cone Tracing 계열의 간소화입니다. Lumen의 Surface Cache, 화면 추적, 거리장/하드웨어 레이 트레이싱, 반사 및 시간적 누적은 구현하지 않았습니다.
- 고정 복셀 범위는 X/Z -8~8, Y -6~10, 셀 크기 0.25입니다. 외부에서는 GI가 없고, 얇은 물체 누광·거친 그림자·자기 가림 오차가 생길 수 있습니다.
- 구/큐브/평면의 해석적 도형을 복셀화합니다. 임의 모델 로딩이나 GPU 삼각형 복셀화는 없습니다. 겹치는 셀은 목록의 첫 도형이 차지합니다.
- 주입되는 직접 방사휘도는 법선과 광원으로 계산한 **비차폐 직접광**입니다. 주입 단계의 직접광 그림자, 반복적인 다중 바운스, 물리 단위 기반 에너지 보존은 없습니다. `Voxel direct shadows`는 화면의 직접광에 적용됩니다. 발광색과 단일 바운스 근사 비교에 적합합니다.
- Dissolve/Wave는 표면 모양과 복셀 표현이 달라지는 오류를 피하려고 GI 광원·차폐체에서 제외합니다. 화면에서는 GI를 받습니다. 발광을 조절해도 이 두 효과는 주변으로 발광을 전달하지 않습니다.
- 기하 기능은 **객체 단위 LOD + CPU meshlet 프러스텀 컬링**입니다. Nanite의 계층 DAG, 클러스터별 혼합 LOD, GPU 가림 컬링, 소프트웨어 래스터라이저, 가상 기하 스트리밍은 없습니다. 생성한 모든 LOD는 메모리에 상주합니다.
- Wave는 변위 오차가 단순화 오차에 포함되지 않아 LOD 0을 유지하며, 컬링 경계는 최대 변위만큼 확장합니다. LOD 전환은 즉시 이루어져 팝핑이 가능합니다.
- meshlet마다 draw하므로 삼각형 감소가 FPS 향상을 보장하지 않습니다. 작은 메시에서는 오히려 draw 호출 비용이 커질 수 있습니다.
- 통계는 기준 바닥을 제외한 **GPU 제출 삼각형** 수입니다. 최종 보이는 픽셀이나 고유 가시 삼각형 수가 아닙니다. GPU 시간은 이전 쿼리 결과이며 UI 및 CPU 복셀 계산은 제외합니다. 복셀 시간은 CPU 계산과 업로드 호출 시간으로, GPU 업로드 완료 시간은 아닙니다.

## 참고한 공식 자료와 사용 소스

- [Epic: Lumen Technical Details](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine): Lumen 구성과 본 실험에서 빠진 시스템을 구분하는 기준.
- [Epic: Nanite SIGGRAPH 2021 발표](https://advances.realtimerendering.com/s2021/Karis_Nanite_SIGGRAPH_Advances_2021_final.pdf): 클러스터, 계층화, 컬링, 가상 기하의 설계 배경.
- [Crassin 외: Interactive Indirect Illumination Using Voxel Cone Tracing, 2011](https://research.nvidia.com/publication/2011-09_interactive-indirect-illumination-using-voxel-cone-tracing): 본 GI 실험의 실제 알고리즘 계열. 논문의 전체 구현이나 sparse voxel octree를 재현한 것은 아닙니다.
- [meshoptimizer v0.22 소스](https://github.com/zeux/meshoptimizer/tree/v0.22): 실제 링크하는 라이브러리. `meshopt_simplify`, `meshopt_buildMeshlets`, `meshopt_computeMeshletBounds` 등을 사용합니다. CMake가 고정 버전 태그를 다운로드합니다. MIT 라이선스 사본을 실행 파일 옆 `Licenses/meshoptimizer.txt`로 복사합니다.
- [Unreal Engine GitHub 소스 접근 안내](https://www.unrealengine.com/ue-on-github/): UE 실제 소스는 Epic/GitHub 계정 연결과 해당 라이선스 조건에 따라 별도로 접근해야 합니다. 이 저장소에는 UE 코드를 포함하지 않습니다.

## 검증

`./Run-ShaderLab.ps1 -Test` 또는 `-Test -Configuration Debug`로 GPU 테스트 세 가지를 실행합니다. 기존 셰이더/UI 테스트 외에 `--research-test`가 GI on/off 차이, 정적 캐시 재사용, 이동 후 갱신, 제외 효과, 다섯 재질의 GL 오류, LOD 삼각형 감소, 프러스텀 컬링 전후 보이는 픽셀 일치와 제출량 감소, Wave의 LOD 0 유지를 검사합니다. UI 테스트는 두 연구 장면 버튼도 실제 입력으로 누릅니다.

결과 이미지는 `build/research-gi-off.png`, `build/research-gi-on.png`, `build/research-lod.png`, `build/research-ui.png`에 생성됩니다. 측정 수치는 뷰포트 크기·카메라·설정에 따라 달라집니다.
