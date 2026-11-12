/**
 * Presentación interactiva de Middleware.
 * Este archivo únicamente controla la navegación y las simulaciones visuales.
 * No establece conexiones TCP ni ejecuta los programas Java y C.
 */

// 1. Navegación responsive y animaciones al entrar en pantalla.
const sections = [...document.querySelectorAll('main > section')];
const links = [...document.querySelectorAll('nav a')];
const navigation = document.getElementById('navigation');
const toggle = document.getElementById('mobileToggle');
toggle.addEventListener('click', () => {
  const open = navigation.classList.toggle('open');
  toggle.setAttribute('aria-expanded', String(open));
});
links.forEach(a => a.addEventListener('click', () => {
  navigation.classList.remove('open'); toggle.setAttribute('aria-expanded','false');
}));
const revealObserver = new IntersectionObserver(entries => {
  entries.forEach(entry => { if (entry.isIntersecting) {entry.target.classList.add('visible');revealObserver.unobserve(entry.target);} });
}, {threshold:.1});
document.querySelectorAll('.reveal').forEach(el => revealObserver.observe(el));
const sectionObserver = new IntersectionObserver(entries => {
  entries.forEach(entry => {
    if (entry.isIntersecting) links.forEach(a => a.classList.toggle('active', a.hash === '#' + entry.target.id));
  });
}, {rootMargin:'-35% 0px -55% 0px'});
sections.forEach(el => sectionObserver.observe(el));
// 2. Modo presentación: las flechas avanzan entre secciones; Esc lo desactiva.
let presentation = false;
const presentationBtn = document.getElementById('presentationBtn');
const presentHelp = document.getElementById('presentHelp');
function setPresentation(value){
  presentation = value;
  document.body.classList.toggle('presentation-mode', value);
  presentHelp.classList.toggle('visible', value);
  presentationBtn.innerHTML = value ? '✕ <span>Salir</span>' : '◧ <span>Presentar</span>';
}
presentationBtn.addEventListener('click',()=>setPresentation(!presentation));
document.addEventListener('keydown',e=>{
  if(e.key==='Escape' && presentation){setPresentation(false);return;}
  if(!presentation || ['INPUT','TEXTAREA','BUTTON'].includes(document.activeElement.tagName)) return;
  if(!['ArrowRight','ArrowLeft','PageDown','PageUp'].includes(e.key))return;
  e.preventDefault();
  let current = 0;
  sections.forEach((s,i)=>{if(s.getBoundingClientRect().top <= window.innerHeight*.45)current=i;});
  const next = Math.max(0,Math.min(sections.length-1,current+(['ArrowRight','PageDown'].includes(e.key)?1:-1)));
  sections[next].scrollIntoView({behavior:'smooth'});
});
// 3. Addition: exactamente el ejemplo 2x2 mostrado en la presentación.
// Los arreglos representan matrices 2 × 2 en orden por filas.
const A=[1,2,3,4],B=[5,6,7,8];
let additionStep=0;
// Dibuja cada celda y resalta la operación actual.
function renderMatrix(id,values,step,result=false){
  document.getElementById(id).innerHTML=values.map((v,i)=>{
    const active=additionStep>0 && i===additionStep-1;
    const done=result && i<additionStep;
    const visible=!result || i<additionStep;
    return `<div class="matrix-cell ${active?'active':''} ${done?'done':''} ${!visible?'empty':''}">${visible?v:'·'}</div>`;
  }).join('');
}
function renderAddition(){
  renderMatrix('matrixA',A);renderMatrix('matrixB',B);renderMatrix('matrixR',A.map((v,i)=>v+B[i]),additionStep,true);
  document.getElementById('additionStepLabel').textContent=`PASO ${additionStep} / 4`;
  const t=document.getElementById('additionTitle'),p=document.getElementById('additionExplain'),f=document.getElementById('additionFormula');
  if(!additionStep){t.textContent='Listos para comenzar';p.textContent='El servidor recibe dos matrices del mismo tamaño y calcula cada elemento de la matriz resultado.';f.textContent='R[i][j] = A[i][j] + B[i][j]';}
  else {const i=additionStep-1,row=Math.floor(i/2),col=i%2;t.textContent=`Posición [${row}][${col}]`;p.textContent=`Se suman los valores que ocupan la misma posición en ambas matrices.`;f.textContent=`${A[i]} + ${B[i]} = ${A[i]+B[i]}`;}
  document.getElementById('addNext').disabled=additionStep===4;
}
document.getElementById('addNext').addEventListener('click',()=>{additionStep=Math.min(4,additionStep+1);renderAddition();});
document.getElementById('addReset').addEventListener('click',()=>{additionStep=0;renderAddition();});
renderAddition();
// 4. Greatest: el mayor se actualiza al recorrer el vector.
const vector=[10,25,8,40,15];let greatestStep=0;
// En cada paso se calcula el máximo del prefijo ya recorrido.
function renderGreatest(){
  document.getElementById('vector').innerHTML=vector.map((v,i)=>`<div class="vector-cell ${i===greatestStep?'current':i<greatestStep?'visited':''}"><small>v[${i}]</small><strong>${v}</strong></div>`).join('');
  const max=Math.max(...vector.slice(0,greatestStep+1));
  document.getElementById('maxValue').textContent=max;
  document.getElementById('greatestStepLabel').textContent=`PASO ${greatestStep+1} / ${vector.length}`;
  const prev=greatestStep?Math.max(...vector.slice(0,greatestStep)):null;
  document.getElementById('compareText').textContent=prev===null?'Comenzamos con el primer elemento.':`${vector[greatestStep]} ${vector[greatestStep]>prev?'>':'≤'} ${prev}: ${vector[greatestStep]>prev?'se actualiza':'se conserva'} el mayor.`;
  document.getElementById('maxNext').disabled=greatestStep===vector.length-1;
}
document.getElementById('maxNext').addEventListener('click',()=>{greatestStep=Math.min(vector.length-1,greatestStep+1);renderGreatest();});
document.getElementById('maxReset').addEventListener('click',()=>{greatestStep=0;renderGreatest();});
renderGreatest();
