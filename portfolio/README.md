# 박신우 클라이언트 포트폴리오

## 열기
- `index.html`을 더블클릭하면 인터넷 연결이나 서버 없이 열립니다.
- `portfolio-standalone.html`은 이미지·스타일·스크립트를 모두 포함한 단일 파일입니다. 이 파일 하나만 전달해도 됩니다.
- 폴더 버전을 전달할 때에는 assets, styles.css, links.js, main.js를 함께 전달하세요.

## 링크 수정
`links.js`에서 graduation / shader / network / flickdom의 github, youtube, download 주소를 입력하세요.
빈 문자열은 클릭되지 않는 "준비 중"으로 표시합니다.
FlickDom의 기존 게임 주소는 Game Play로 표시하며 원본 PDF에 있던 GitHub/영상/게임 링크를 보존했습니다.
졸업작품 및 Shader Lab의 실제 배포 주소는 제공되지 않아 비워 두었습니다.
수정 후 페이지를 새로 고치세요. 단일 파일 버전은 HTML 내 `window.PORTFOLIO_LINKS` 부분을 직접 수정해야 합니다.

## 구성
- 원본 졸업작품 소개 5페이지 다음에 Shader Lab 6페이지를 추가했습니다.
- 원본 프로젝트 페이지는 디자인을 보존하기 위해 고해상도 이미지로 포함했습니다. 이 부분의 글은 HTML 텍스트가 아닙니다.
- 목차와 새 Shader Lab 페이지는 수정 가능한 HTML/CSS입니다. index.html에서 내용을, styles.css에서 디자인을 수정하세요.
- 원본 목차에만 있던 버블로그는 목차 하단과 원본 목차 이미지로 유지했습니다. 제공되지 않은 상세 내용은 추가하지 않았습니다.
- 새 페이지의 이미지는 실제 Shader Lab 실행 캡처입니다.
- Voxel GI 및 CPU LOD/컬링을 설명하며 Unreal Lumen/Nanite 전체 구현으로 표현하지 않았습니다.
- AI 협업 기반 구현 및 학습 프로젝트로 표기했습니다.
- 기존 원본 페이지는 작은 화면에서 축소되므로 확대해서 읽을 수 있습니다. 신규 HTML 페이지는 모바일에서 세로 배치됩니다.

## 원본
사용자가 제공한 '박신우 클라이언트 포트폴리오.pdf'를 기반으로 제작했습니다.
외부 폰트, 추적 스크립트, CDN 의존성은 없습니다. 웹 공개는 수행하지 않았습니다.
