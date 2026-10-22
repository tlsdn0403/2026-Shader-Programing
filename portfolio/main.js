
// 링크가 아직 없는 항목은 클릭할 수 없는 자리표시자로 유지합니다.
document.querySelectorAll('[data-project]').forEach(link => {
  const url = window.PORTFOLIO_LINKS?.[link.dataset.project]?.[link.dataset.kind]?.trim();
  if (!url) return;
  try {
    const parsed = new URL(url, location.href);
    if (!['http:', 'https:', 'file:'].includes(parsed.protocol)) return;
    link.href = url;
    link.removeAttribute('aria-disabled');
    link.target = '_blank';
    link.rel = 'noopener noreferrer';
    link.querySelector('span').textContent = '열기 ↗';
    if (!link.classList.contains('pdf-link') && link.dataset.project === 'flickdom' && link.dataset.kind === 'download') {
      link.firstChild.textContent = 'Game Play ';
    }
  } catch (_) { /* 잘못된 주소는 준비 중 상태로 유지합니다. */ }
});
document.querySelectorAll('.slide').forEach((slide, index) => {
  const number = slide.querySelector('.page-number');
  if (number) number.textContent = String(index + 1).padStart(2, '0');
});

// 원본 해상도의 캡처와 측정 패널을 확대해 읽습니다.
const shotDialog=document.createElement('dialog');shotDialog.className='shot-dialog';shotDialog.setAttribute('aria-label','실행 화면 확대');shotDialog.innerHTML='<button type="button">닫기 ×</button><img alt="확대된 실행 화면">';document.body.append(shotDialog);shotDialog.querySelector('button').onclick=()=>shotDialog.close();document.querySelectorAll('.shot-open').forEach(b=>b.onclick=()=>{const img=b.querySelector('img');shotDialog.querySelector('img').src=img.src;shotDialog.querySelector('img').alt=img.alt;shotDialog.showModal();});
