# Shader Lab

C++ / OpenGL 셰이더 포트폴리오를 위한 작은 편집기입니다. 왼쪽에서 실험을 선택하고, 중앙 3D 뷰포트와 오른쪽 Inspector에서 결과를 확인합니다. `SimpleGame.sln`에서 ShaderLab을 빌드·실행할 수 있습니다. 기존 수업 소스는 보존되어 있으며, 이전 프로젝트 설정은 `SimpleGame/SimpleGame.Legacy.vcxproj`에 보관했습니다.

## 실행

**`Run-ShaderLab.cmd`를 더블 클릭**합니다. 첫 실행은 라이브러리를 다운로드하고 빌드하므로 인터넷 연결이 필요합니다. 이후에는 `build/Release/ShaderLab.exe`를 직접 실행할 수 있습니다.

준비물: Windows, Visual Studio 2022의 **C++를 사용한 데스크톱 개발** 및 **Windows용 C++ CMake 도구**, Git, OpenGL 3.3 지원 그래픽 드라이버.

Visual Studio에서는 **`SimpleGame.sln`을 열고 `Debug | x64`로 빌드한 뒤 F5**를 누릅니다. 솔루션을 이미 열고 있었다면 프로젝트 변경을 다시 로드하거나 솔루션을 닫았다가 다시 여세요. 프로젝트 이름은 `ShaderLab`이며, 기존 프로젝트의 ID를 유지해 시작 프로젝트 연결을 보존합니다. 실행 대상은 `build/Debug/ShaderLab.exe`입니다.

솔루션의 빌드·다시 빌드·정리는 공통 CMake 빌드 스크립트로 연결됩니다. Debug/Release와 x64/x86 선택이 반영되고, x86은 `build-win32`에 따로 생성됩니다. GLFW/GLEW를 해당 아키텍처로 함께 빌드하므로 Windows SDK 경로의 오래된 x86 freeglut 라이브러리에 의존하지 않습니다. 보관한 Legacy 프로젝트는 기존 실습용 설정 그대로이며, 현재 솔루션에서는 빌드하지 않습니다.

저장소 폴더를 열고 `windows` CMake preset을 선택하는 방식도 사용할 수 있습니다.

```powershell
./Run-ShaderLab.ps1 -BuildOnly  # 빌드만
./Run-ShaderLab.ps1 -Test       # 빌드 및 GPU 스모크 테스트
./Run-ShaderLab.ps1 -Test -Configuration Debug # Visual Studio Debug 구성 검증
```

## 구현된 기능

오른쪽 **Research** 탭에 동적 복셀 간접광과 meshlet LOD/컬링 실험을 추가했습니다. `Load GI study`, `Load 64-object LOD study`로 비교 장면을 불러옵니다. **UE Lumen/Nanite 전체 구현은 아닙니다.** 사용법, 코드 흐름, 제한 사항과 공식 자료·사용 소스는 [렌더링 연구 문서](RENDERING_RESEARCH.md)를 참고하세요. 재질의 색·발광·반사와 조명 설정은 **Light** 탭에, 위치·크기와 Dissolve/Wave 설정은 **Object** 탭에 있습니다.

| 실험 | 내용 | 전용 조절값 |
|---|---|---|
| Studio lighting | Phong / Blinn–Phong 선택, 직접광 on/off, 주변광, 톤 매핑 | Lighting model, Specular width |
| Toon shading | 단계형 명암과 하이라이트 | 조명 세기·방향 |
| Fresnel rim | 시선 방향에 따른 가장자리 발광 | Rim power |
| Dissolve | 3D 노이즈 기반 discard, 발광 경계 | Dissolve |
| Wave surface | 버텍스 변위 및 변위에 맞춘 법선 보정 | Wave height |

- 구, 큐브, 세분화 평면 전환. 물결 실험은 평면에서 확인하기 좋습니다.
- 기본색과 Light 탭의 광원 종류·색·세기·위치·방향·범위, 일정한 주변광 조절.
- 뷰포트 드래그로 카메라 회전, 휠로 확대·축소, Home으로 카메라 초기화.
- 와이어프레임, 기준 바닥, 재생·일시정지(Space), 장면 초기화.
- FPS와 프레임 시간, 선택 메시의 삼각형 수 표시. FPS는 전체 앱 프레임 통계이며 GPU 타이머 측정치가 아닙니다.
- `Reload shaders` 또는 R로 GLSL 다시 컴파일. 실패 시 마지막 정상 프로그램을 유지하고 오류를 Inspector에 표시합니다.
- `Save preview`로 실행 파일 옆에 `preview.png` 저장(기존 파일 덮어쓰기). UI를 제외한 뷰포트 이미지입니다. 저장소에 있던 LodePNG 코드를 재사용합니다.

UI 기본 글꼴은 Windows Segoe UI이며, 항목은 영문입니다.

## 셰이더 수정과 확장

| 파일 | 책임 |
|---|---|
| `main.cpp` | Application 생성 및 실행 |
| `Application.h/.cpp` | 창·OpenGL 컨텍스트 생성, 프레임 루프, 종료 순서 |
| `EditorUI.h/.cpp` | ImGui 수명 관리, 패널, 입력, 뷰포트 표시 |
| `SceneSettings.h/.cpp` | 편집 상태와 실험 프리셋 |
| `Camera.h/.cpp` | OrbitCamera 회전·줌·프레이밍, 뷰/투영 행렬 |
| `ShaderLibrary.h/.cpp` | 셰이더 읽기·컴파일·링크, 프로그램 소유권과 재로딩 |
| `Renderer.h/.cpp` | 메시·프레임버퍼 관리, uniform 전달, 장면 그리기 |
| `ImageCapture.h/.cpp` | 프레임버퍼 PNG 저장 |
| `SmokeTests.h/.cpp` | 렌더링 검증 및 UI 자동 입력 테스트 |

모든 파일은 `ShaderLab` 폴더에 있습니다.

- `ShaderLab/Shaders/Lighting.vs` + `Lighting.fs`: 기본 Blinn–Phong 조명.
- `ShaderLab/Shaders/Toon.vs` + `Toon.fs`: 단계형 명암과 하이라이트.
- `ShaderLab/Shaders/Fresnel.vs` + `Fresnel.fs`: 시선 방향에 따른 가장자리 발광.
- `ShaderLab/Shaders/Dissolve.vs` + `Dissolve.fs`: 노이즈 기반 디졸브와 발광 경계.
- `ShaderLab/Shaders/Wave.vs` + `Wave.fs`: 물결 정점 변위·법선 보정과 표면 조명.

각 기능은 독립적인 버텍스/프래그먼트 셰이더 쌍으로 컴파일됩니다. 목록 선택 시 해당 GPU 프로그램으로 전환하며, 셰이더 안의 `uEffect` 분기는 제거했습니다. 학습하기 쉽도록 공통 조명 코드도 각 파일에 포함했습니다. 물결 이외의 버텍스 셰이더는 현재 같은 좌표 변환을 수행하지만 독립적으로 수정할 수 있습니다. `uFloor`는 같은 프로그램으로 그리는 기준 바닥에 효과를 적용하지 않기 위한 구분입니다.

Inspector의 SHADER STATUS에 현재 파일 이름이 표시됩니다. Reload는 여섯 쌍을 모두 컴파일하고, 하나라도 실패하면 모든 기존 프로그램을 유지하며 실패한 기능과 셰이더 단계를 표시합니다.

빌드하면 원본 셰이더를 실행 파일 옆 `Shaders` 폴더로 복사합니다. **실행 중 Reload는 실행 파일 옆 복사본을 읽습니다.** 원본을 수정했다면 다시 빌드한 뒤 Reload하거나, 빠른 실험은 복사본에서 진행한 뒤 원본에 반영하세요. 복사본만 수정하면 다음 빌드 때 덮어씁니다.

단일 패스 재질을 추가할 때는 새 `.vs`/`.fs` 쌍과 실험 이름/설명/프리셋을 추가하고, ShaderLibrary의 `Name`, 프로그램 배열 및 UI/테스트의 효과 개수도 함께 확장합니다. SSAO·블룸·Shadow Map처럼 여러 패스가 필요한 기능은 Renderer에 별도 프레임버퍼와 패스를 추가해야 합니다. 현재 에셋 임포터, 장면 저장, 노드 편집기, PBR/IBL은 없습니다. 복셀 간접광과 복셀 직접광 그림자는 Research 탭에 구현되어 있습니다. Light 탭의 일정한 주변광 슬라이더는 GI와 별개입니다.

## 루멘·나나이트로 확장한다면

현재는 복셀 간접광과 객체 단위 LOD·CPU meshlet 프러스텀 컬링이 구현되어 있습니다. UE Lumen이나 Nanite 전체 시스템을 구현한 것은 아닙니다. 아래 순서는 추가 학습 방향이며, 현재 구현과 제한은 [렌더링 연구 문서](RENDERING_RESEARCH.md)를 기준으로 확인하세요.

1. 렌더링 기반: PBR → Shadow Map → G-buffer와 디버그 뷰.
2. 조명 연구: SSAO/SSR → 간단한 SDF 또는 복셀 기반 간접광 → 시간 누적. Lumen과 동일한 구현으로 소개하지 않습니다.
3. 지오메트리 연구: 메시 LOD → 프러스텀/가림 컬링 → 클러스터 단위 LOD와 스트리밍. 단순 LOD를 Nanite라고 소개하지 않습니다.
4. 각 실험에 on/off 비교, GPU 시간, 장면 복잡도, 시각적 오류 및 한계를 기록합니다.

참고: [Epic Lumen 기술 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine), [Epic Nanite 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine).

## 의존성과 검증

CMake FetchContent로 GLFW 3.4, GLEW 2.2.0(glew-cmake), Dear ImGui 1.91.8을 고정 버전으로 가져옵니다. 각 라이브러리의 라이선스는 `build/_deps/*-src`에 함께 내려받습니다. 배포 시 해당 저작권·라이선스 고지를 포함해야 합니다. 기존 수업 코드의 저작권 고지는 변경하지 않았습니다.

`--smoke-test`는 숨겨진 실제 OpenGL 컨텍스트에서 6개 셰이더 쌍이 서로 다른 프레임을 생성하는지, 3개 메시 및 와이어프레임 렌더링, 뷰포트 리사이즈, 성공/실패 시 셰이더 재로딩을 확인합니다. GPU/디스플레이 드라이버가 없는 환경에서는 실행할 수 없습니다.

`--ui-smoke-test`는 실제 UI에 마우스 이벤트를 전달해 효과 선택, 일시정지, 창 크기 변경, 객체 추가/삭제, 연구 장면과 Stress 탭의 1,000개 생성을 검증합니다. `--research-test`는 GI 갱신과 LOD·컬링을 검증합니다. 세 테스트는 `-Test`로 함께 실행됩니다.

## 현재 프레임 파이프라인

```text
main → Application::Run
  ├─ GLFW 창 / OpenGL 초기화
  ├─ Renderer 초기화 → ShaderLibrary가 6개 .vs/.fs 쌍 컴파일
  └─ 매 프레임
      ├─ 이벤트 처리 / 애니메이션 시간 갱신
      ├─ EditorUI: 입력과 Inspector로 Settings 변경
      ├─ Renderer: 뷰포트용 FBO 준비
      │   ├─ OrbitCamera의 뷰·투영 행렬 계산
      │   ├─ ShaderLibrary에서 효과별 프로그램 선택
      │   ├─ uniform 전달 → 오브젝트 → 기준 바닥
      │   └─ 컬러 텍스처를 EditorUI에 제공
      ├─ EditorUI: 뷰포트 이미지와 UI를 기본 프레임버퍼에 출력
      └─ 화면 버퍼 교환
```

Application은 종료 시 ImGui → 렌더링 자원 → 창 → GLFW 순서로 정리합니다. OpenGL 자원을 삭제할 때 컨텍스트가 살아 있도록 보장합니다. UI에서 예외가 발생하거나 셰이더 초기화에 실패한 경우도 같은 종료 경로를 사용합니다. GPU 프로그램과 ImGui 소유 클래스는 복사를 금지합니다.

Settings는 재질·조명·카메라 상태를 담는 데이터 구조체이고, ImageCapture는 상태 없는 유틸리티입니다. 메시 생성과 FBO 관리는 현재 Renderer에 남겨두었으며, 향후 모델 임포트나 다중 렌더 패스 추가 시 분리할 수 있습니다.

## 여러 객체 사용하기

왼쪽 **SCENE OBJECTS**에서 Add count(1~16)를 정하고 **+ Sphere / + Cube / + Plane**을 클릭하면 선택한 개수만큼 추가됩니다. 장면은 최대 **4,096개** 객체를 지원하며 추가 시 빈 격자 위치에 배치하고 전체가 보이도록 카메라를 조절합니다.

대량 생성은 오른쪽 **Stress** 탭을 사용합니다. 개수(1~4,096), 종류(구/큐브/평면/구+큐브), 열 수, 간격과 **3D grid**를 설정한 뒤 **Generate objects**를 누르세요. 100/1000/4096 버튼은 생성 개수를 빠르게 설정합니다. 생성은 기존 장면을 교체하고 선택된 객체의 재질을 복사하며, 크기는 구·큐브 1 / 평면 0.25로 설정합니다. 객체 목록은 화면에 보이는 항목만 처리합니다.

**Disable GI for geometry test**는 기본으로 켜져 있어 생성 시 복셀 GI·그림자를 끕니다. 대규모 격자는 고정 GI 범위를 벗어나고 CPU 복셀화 비용이 크기 때문입니다. LOD·컬링 설정은 유지되며 Stress 탭에서 켜고 끄면서 삼각형 수, draw 호출, GPU 시간과 FPS를 비교할 수 있습니다. **Keep selected only**는 선택 객체만 남기고, **Frame all objects**는 전체가 보이게 합니다. 대규모 장면에서도 휠 확대·축소 속도는 거리에 비례합니다.

목록에서 객체를 선택한 뒤 오른쪽 Inspector의 Position X/Y/Z와 Scale로 이동·균일 크기를 조절합니다. 왼쪽 실험 목록의 셰이더와 오른쪽 재질 값은 선택한 객체에만 적용됩니다. 효과를 바꾸어도 메시 종류는 유지됩니다. 물결을 평면에서 보려면 + Plane으로 추가하거나 Mesh를 Subdivided plane으로 선택하세요.

Duplicate는 선택된 객체의 재질·크기를 복제해 새 위치에 추가하고, Delete selected는 선택 객체를 삭제합니다. 마지막 객체 하나는 유지됩니다. Home 또는 Frame scene은 전체 객체를 보도록 카메라를 맞춥니다. 조명·와이어프레임·재생 상태는 장면 전체 설정이며, Reset scene은 초기 구 하나로 돌아갑니다. 장면 저장과 물리 충돌은 없습니다. 객체 간 그림자는 Research의 복셀 그림자를 켜면 제한된 범위에서 확인할 수 있습니다.

SceneObject가 객체별 상태를 가지며 Renderer는 같은 메시 버퍼를 재사용해 객체마다 그리기 명령을 보냅니다. 각 버텍스 셰이더는 `uScale`과 `uPosition`으로 로컬 위치를 월드 위치로 변환합니다. 정점 버퍼를 객체 수만큼 복제하거나 GPU instancing을 사용한 구현은 아닙니다. 디졸브의 노이즈는 로컬 좌표를 사용해 객체를 이동해도 무늬가 따라갑니다.

## Phong / Blinn–Phong 조명 비교

왼쪽 `Studio lighting`을 선택한 뒤 오른쪽 `Light → SELECTED OBJECT`에서 선택합니다.

- `Blinn-Phong (half vector)`: 기존 `Lighting.vs` / `Lighting.fs`. `H = normalize(L + V)`를 구하고 `dot(N, H)`로 하이라이트를 계산합니다.
- `Phong (reflection)`: 새 `Phong.vs` / `Phong.fs`. `R = 2.0 * N * dot(N, L) - L`을 구하고 `dot(R, V)`로 하이라이트를 계산합니다. 반사 벡터에는 clamp 이전 내적을 사용하고 빛을 받지 않는 면의 하이라이트는 별도로 제거합니다.

선택은 현재 객체에 적용되며 위치·색·카메라·하이라이트 조절값을 유지합니다. 두 모델은 같은 지수 범위를 사용하므로 같은 설정값에서도 하이라이트 폭이 다릅니다. 다른 효과(Toon/Fresnel/Dissolve/Wave)는 기존 계산을 유지합니다. 셰이더 여섯 쌍은 독립 컴파일되며, 재로딩 중 하나라도 실패하면 기존 프로그램 전체를 보존합니다.

Light 탭의 `Direct light enabled`는 장면 전체 직접광의 on/off입니다. 꺼도 Intensity 값은 보존되며, 주변광과 자체 발광은 남습니다. GI가 켜져 있으면 복셀에 주입되는 직접광도 갱신하고, 발광체에서 오는 간접광은 유지합니다. 완전히 어두운 비교를 하려면 Ambient fill과 Emission을 0으로 두고 GI를 끄세요. Research의 색상 디버그 뷰는 조명을 무시하므로 비교 시 `Shaded`를 선택합니다.

동일 장면 비교 캡처는 테스트 실행 후 `build/lighting-blinn.png`, `build/lighting-phong.png`에 저장됩니다. 테스트는 모델별 화면 차이, 원래 모델 복원, 직접광 off와 세기 0의 일치, GI 캐시 갱신, 실제 UI 모델 선택 및 조명 토글을 확인합니다.
## Light 탭과 XYZ 광원 이동

광원 설정은 **Object가 아닌 Light 탭**에 있습니다. 현재 장면 광원 하나를 편집합니다. Phong/Blinn–Phong, Base color, Emission, Specular width, Rim power도 Light 탭에서 조절합니다. 위쪽 SELECTED OBJECT는 선택 객체에만, 아래 SCENE LIGHT는 장면 전체에 적용됩니다.

1. Light 탭에서 `Directional / Point / Spot`을 선택합니다.
2. `Light color`, `Intensity`, `Position X/Y/Z`, `Direction X/Y/Z`, `Range`를 조절합니다. 숫자 필드는 드래그하거나 Ctrl+클릭으로 직접 입력할 수 있습니다.
3. `Frame light + selected`를 누르면 광원과 선택 객체를 함께 보여 줍니다.
4. 뷰포트의 **빨강 X / 초록 Y / 파랑 Z 핸들**을 마우스로 끌면 월드 좌표의 해당 축으로만 이동합니다. 카메라 회전은 동시에 발생하지 않습니다. 마우스를 놓으면 확정하고, Esc는 드래그 시작 위치로 되돌립니다. 빈 공간 드래그는 기존 카메라 회전입니다.
5. `Aim at selected object`로 Directional/Spot 방향을 선택 객체 쪽으로 설정합니다. 방향은 빛이 진행하는 방향이며, 0 벡터는 아래쪽 `(0,-1,0)`으로 처리합니다.
6. `Show range / cone`은 Point의 범위 구 또는 Spot의 원뿔 가이드를 표시합니다. 핸들과 가이드는 Light 탭에서만 보이는 편집기 오버레이이며 PNG 장면 캡처에는 포함되지 않습니다. 시선과 거의 나란한 축은 불안정한 드래그를 피하려고 숨깁니다. 이 경우 카메라를 돌리거나 숫자로 입력하세요.

| 광원 | 위치 | 방향 | 범위 |
|---|---|---|---|
| Directional | 핸들 위치만 이동하며 조명 결과는 동일 | 모든 표면을 일정한 방향으로 비춤 | 무한, Range 비활성화 |
| Point | 빛이 시작하는 위치 | 사용하지 않음 | 범위 밖은 0, 내부에서 거리 감쇠 |
| Spot | 빛이 시작하는 위치 | 원뿔이 향하는 방향 | 거리 감쇠 + 원뿔 각도 감쇠 |

Spot의 Inner/Outer 값은 원뿔의 **반각(도)**입니다. 내부는 밝게, 안쪽과 바깥쪽 경계 사이는 부드럽게 감쇠합니다. 바깥 반각은 안쪽보다 항상 큽니다. Point/Spot 감쇠는 학습용 `max(0,1-distance/range)^2 / (1+0.09*distance+0.032*distance^2)`이며 물리 단위 기반 광도는 아닙니다. 이전의 무감쇠 광원보다 기본 장면이 어둡게 보일 수 있습니다.

새 구조: `Light.h/.cpp`는 광원 상태와 CPU 복셀 주입용 계산, `LightEditor.h/.cpp`는 Light 패널과 XYZ 핸들, `Shaders/Light.glsl`은 모든 재질의 광원 방향/색/감쇠 계산을 담당합니다. Renderer가 uniform을 전달하고 ShaderLibrary가 Light.glsl과 Research.glsl을 확장합니다. 광원 변경은 복셀 GI 캐시에도 반영됩니다. 복셀 그림자·GI의 고정 범위 및 해상도 제한은 그대로입니다.

검증에는 세 광원과 여섯 재질 렌더링, Directional 위치 불변성, Point 위치·색·범위 변화, Spot 원뿔 밖 차광, GI 캐시 갱신, 실제 XYZ 드래그와 카메라 회전 방지·Esc 취소가 포함됩니다. `build/light-editor-ui.png`는 편집기 캡처입니다.
Light 탭의 아래쪽에는 Ambient fill과 펼쳐서 사용하는 Indirect light / shadows(GI on/off, 복셀 그림자, Bounce strength)가 있습니다. 패널을 아래로 스크롤하면 표시됩니다. Research 탭은 연구 장면 로딩·디버그 뷰·성능 통계를 담당합니다. 모델을 바꾸려면 왼쪽 Studio lighting을 선택한 다음 Light 탭 위쪽에서 Phong/Blinn–Phong을 선택하세요.
