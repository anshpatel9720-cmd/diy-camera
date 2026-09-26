#pragma once

static const char PAGE_INDEX[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>DIGICAM-8000</title>
<style>
:root{
  --shell:#b7b9bc; --shell-hi:#d4d6d8; --shell-lo:#8c8f93; --shell-ink:#3c4044;
  --bezel:#17191b; --lcd:#0c0f0d;
  --osd:#f1f3ee; --osd-dim:rgba(241,243,238,.55); --amber:#ffab19; --rec:#ff4436;
  --mono:ui-monospace,"DejaVu Sans Mono","Courier New",monospace;
  --sans:"Helvetica Neue",Arial,system-ui,sans-serif;
}
*{box-sizing:border-box}
html,body{margin:0;background:#26282a;color:var(--osd);font-family:var(--sans)}
body{display:flex;justify-content:center;padding:14px 10px 30px}

/* ---------- camera body ---------- */
.shell{width:100%;max-width:420px;background:linear-gradient(180deg,var(--shell-hi),var(--shell) 12%,var(--shell) 82%,var(--shell-lo));
  border-radius:14px 14px 18px 18px;padding:10px;box-shadow:0 18px 40px rgba(0,0,0,.55),inset 0 1px 0 #e6e8ea}
.topplate{display:flex;justify-content:space-between;align-items:baseline;padding:2px 6px 9px;color:var(--shell-ink)}
.brand{font:700 13px/1 var(--sans);letter-spacing:.22em}
.brand b{font-weight:400;opacity:.6}
.slug{font:400 9px/1 var(--sans);letter-spacing:.18em;opacity:.65;text-transform:uppercase}

.bezel{background:var(--bezel);border-radius:6px;padding:9px 9px 7px;box-shadow:inset 0 2px 6px rgba(0,0,0,.9)}
.lcd{position:relative;aspect-ratio:4/3;background:var(--lcd);overflow:hidden;border-radius:2px}
.feed{position:absolute;inset:0;width:100%;height:100%;object-fit:cover;display:block}
.feed:not([src]){display:none}
.lcdlip{height:3px;margin:6px 2px 0;border-radius:2px;background:linear-gradient(90deg,#2a2d2f,#0d0f10)}

/* ---------- on-screen display ---------- */
.osd{position:absolute;inset:0;pointer-events:none;transition:opacity .15s;font:600 10px/1.35 var(--mono);letter-spacing:.06em;text-transform:uppercase}
.osd>div{position:absolute;padding:7px 8px;display:flex;gap:9px;text-shadow:0 1px 2px rgba(0,0,0,.95)}
.tl{top:0;left:0}.tr{top:0;right:0}.bl{bottom:0;left:0;flex-wrap:wrap}.br{bottom:0;right:0;align-items:center}
.osd .k{color:var(--osd-dim)}
.osd .v{color:var(--amber)}
.dot{width:7px;height:7px;border-radius:50%;background:var(--rec);box-shadow:0 0 6px var(--rec)}
@media (prefers-reduced-motion:no-preference){.dot{animation:blink 2.4s steps(1,end) infinite}}
@keyframes blink{0%,60%{opacity:1}61%,100%{opacity:.15}}

.lcd.panelopen .osd{opacity:0}
.toast{position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);
  background:rgba(0,0,0,.72);border:1px solid rgba(255,171,25,.5);color:var(--amber);
  font:600 11px/1 var(--mono);letter-spacing:.1em;padding:8px 12px;opacity:0;transition:opacity .18s}
.toast.show{opacity:1}
.flash{position:absolute;inset:0;background:#fff;opacity:0;pointer-events:none}
.flash.go{animation:pop .32s ease-out}
@keyframes pop{0%{opacity:.9}100%{opacity:0}}

/* ---------- panels that live inside the screen ---------- */
.panel{position:absolute;inset:0;background:rgba(6,9,7,.975);display:none;flex-direction:column;
  font:600 11px/1.3 var(--mono);letter-spacing:.05em}
.panel.on{display:flex}
.tabs{display:flex;flex:0 0 auto;border-bottom:1px solid rgba(241,243,238,.16)}
.tabs button{flex:1;background:none;border:0;color:var(--osd-dim);font:600 9px/1 var(--mono);
  letter-spacing:.12em;padding:8px 0;cursor:pointer}
.tabs button.on{color:#0c0f0d;background:var(--amber)}
.playbar{display:flex;align-items:center;gap:8px;padding:7px 9px;border-bottom:1px solid rgba(241,243,238,.16)}
.playbar .cnt{flex:1;color:var(--osd-dim);font:600 9px/1 var(--mono);letter-spacing:.12em}
.playbar button{background:#141815;border:1px solid rgba(241,243,238,.24);color:var(--osd);
  font:600 9px/1 var(--mono);letter-spacing:.12em;padding:6px 9px;cursor:pointer;border-radius:2px}
.rows{flex:1;overflow-y:auto;-webkit-overflow-scrolling:touch}
.row{display:flex;align-items:center;gap:8px;padding:7px 9px;border-bottom:1px solid rgba(241,243,238,.07)}
.row .lb{flex:1;color:var(--osd);text-transform:uppercase;font-size:10px}
.row.off{opacity:.32}
.row .val{color:var(--amber);min-width:52px;text-align:right;font-size:10px}
.row input[type=range]{width:104px;accent-color:var(--amber);background:none}
.row select{background:#141815;color:var(--amber);border:1px solid rgba(241,243,238,.22);
  font:600 10px/1 var(--mono);padding:4px 5px;border-radius:2px}
.sw{width:42px;height:20px;border:1px solid rgba(241,243,238,.3);border-radius:2px;background:#141815;
  position:relative;cursor:pointer;flex:0 0 auto}
.sw i{position:absolute;top:2px;left:2px;width:16px;height:14px;background:var(--osd-dim);transition:.15s}
.sw.on{border-color:var(--amber)}
.sw.on i{left:22px;background:var(--amber)}

/* ---------- gallery ---------- */
.grid{flex:1;overflow-y:auto;display:grid;grid-template-columns:repeat(3,1fr);
  grid-auto-rows:min-content;gap:3px;padding:3px;align-content:start}
.cell{position:relative;aspect-ratio:1;background:#171b18;overflow:hidden;cursor:pointer}
.cell img{position:absolute;inset:0;width:100%;height:100%;object-fit:cover;display:block}
.cell span{position:absolute;left:0;right:0;bottom:0;background:#000;font:600 9px/1.9 var(--mono);
  text-align:center;color:var(--osd);letter-spacing:.04em}
.empty{margin:auto;color:var(--osd-dim);text-align:center;font-size:10px;padding:20px;line-height:1.9}
.big{position:absolute;inset:0;background:#050705;display:none;flex-direction:column}
.big.on{display:flex}
.big img{flex:1;min-height:0;object-fit:contain;width:100%}
.big .bar{display:flex;gap:6px;padding:6px}
.big a,.big button{flex:1;display:flex;align-items:center;justify-content:center;height:30px;
  text-decoration:none;background:#141815;color:var(--osd);border:1px solid rgba(241,243,238,.22);
  font:600 9px/1 var(--mono);letter-spacing:.1em;cursor:pointer}
.big .del{color:var(--rec);border-color:rgba(255,68,54,.5)}

/* ---------- control deck ---------- */
.deck{display:grid;grid-template-columns:1fr auto 1fr;align-items:center;gap:10px;padding:14px 6px 6px}
.rocker{display:flex;border:1px solid var(--shell-lo);border-radius:3px;overflow:hidden;background:#a7aaad}
.rocker button{flex:1;background:none;border:0;padding:8px 0;color:var(--shell-ink);cursor:pointer;
  font:700 8px/1 var(--sans);letter-spacing:.14em}
.rocker button+button{border-left:1px solid var(--shell-lo)}
.rocker button.on{background:var(--shell-ink);color:var(--shell-hi)}

.shutter{width:62px;height:62px;border-radius:50%;border:0;cursor:pointer;padding:0;
  background:radial-gradient(circle at 34% 28%,#f6f7f8,#c8cacc 46%,#9a9d9f);
  box-shadow:0 5px 0 #7e8184,0 8px 14px rgba(0,0,0,.45);transition:transform .06s,box-shadow .06s}
.shutter span{display:block;width:34px;height:34px;margin:auto;border-radius:50%;
  background:radial-gradient(circle at 36% 30%,#ff8a72,#d8442d 60%,#8f2417);box-shadow:inset 0 1px 2px rgba(255,255,255,.5)}
.shutter:active,.shutter.push{transform:translateY(5px);box-shadow:0 0 0 #7e8184,0 3px 8px rgba(0,0,0,.4)}
.shutter[disabled]{filter:grayscale(1) brightness(.85);cursor:wait}

.lampbox{display:flex;flex-direction:column;gap:6px;align-items:stretch}
.chip{background:#a7aaad;border:1px solid var(--shell-lo);border-radius:3px;color:var(--shell-ink);
  font:700 8px/1 var(--sans);letter-spacing:.14em;padding:8px 0;cursor:pointer}
.chip.on{background:var(--amber);border-color:#c9860f;color:#2a1d00}
.lampbox input{width:100%;accent-color:var(--shell-ink)}
.footplate{text-align:center;color:var(--shell-ink);opacity:.5;font:400 8px/1 var(--sans);letter-spacing:.2em;padding:16px 0 2px}
</style></head>
<body>
<div class="shell">
  <div class="topplate"><span class="brand">DIGICAM<b>·8000</b></span><span class="slug" id="slug">OV2640 2MP</span></div>

  <div class="bezel">
    <div class="lcd">
      <img id="live" class="feed" alt="">
      <div class="osd">
        <div class="tl"><span class="v" id="oMode">PHOTO</span><span class="k" id="oRes">—</span></div>
        <div class="tr"><span class="k" id="oCard">SD —</span></div>
        <div class="bl"><span class="k">ISO</span><span class="v" id="oIso">AUTO</span>
                        <span class="k">EV</span><span class="v" id="oEv">0</span>
                        <span class="k">FX</span><span class="v" id="oFx">OFF</span></div>
        <div class="br"><span class="k" id="oShot">0000</span><span class="dot"></span></div>
        <div class="toast" id="toast"></div>
      </div>
      <div class="flash" id="flash"></div>

      <div class="panel" id="menu">
        <div class="tabs" id="tabs"></div>
        <div class="rows" id="rows"></div>
      </div>

      <div class="panel" id="play">
        <div class="playbar"><span class="cnt" id="pcount">CARD</span><button id="reload">REFRESH</button></div>
        <div class="grid" id="grid"></div>
        <div class="big" id="big">
          <img id="bigImg" alt="">
          <div class="bar">
            <button id="bClose">BACK</button>
            <a id="bSave" download>SAVE</a>
            <button id="bDel" class="del">DELETE</button>
          </div>
        </div>
      </div>
    </div>
    <div class="lcdlip"></div>
  </div>

  <div class="deck">
    <div class="rocker" id="rocker">
      <button data-m="view" class="on">VIEW</button>
      <button data-m="menu">MENU</button>
      <button data-m="play">PLAY</button>
    </div>
    <button class="shutter" id="shutter" aria-label="Take a photo"><span></span></button>
    <div class="lampbox">
      <button class="chip" id="lampBtn">LAMP</button>
      <input id="lampLvl" type="range" min="0" max="255" step="5" aria-label="Lamp brightness">
    </div>
  </div>

  <div class="footplate" id="foot">CONNECTED</div>
</div>

<script>
const $=s=>document.querySelector(s), HOST=location.hostname;
const STREAM='http://'+HOST+':81/stream';
let S={}, mode='view', page=0;

const SIZES=[[5,'QVGA 320'],[8,'VGA 640'],[9,'SVGA 800'],[10,'XGA 1024'],[12,'SXGA 1280'],[13,'UXGA 1600']];
const LIVE =[[5,'QVGA 320'],[8,'VGA 640'],[9,'SVGA 800']];
const FX=[[0,'OFF'],[2,'B&W'],[6,'SEPIA'],[1,'NEGATIVE'],[3,'RED'],[4,'GREEN'],[5,'BLUE']];
const WB=[[0,'AUTO'],[1,'SUNNY'],[2,'CLOUDY'],[3,'OFFICE'],[4,'HOME']];
const QU=[[10,'FINE'],[14,'NORMAL'],[20,'BASIC'],[28,'ECONOMY']];
const GC=[[0,'2x'],[1,'4x'],[2,'8x'],[3,'16x'],[4,'32x'],[5,'64x'],[6,'128x']];

const PAGES=[
 {t:'PICT',items:[
   {k:'photo_size',l:'Photo size',o:SIZES},
   {k:'quality',l:'Picture quality',o:QU},
   {k:'special_effect',l:'Filter',o:FX},
   {k:'brightness',l:'Brightness',r:[-2,2,1]},
   {k:'contrast',l:'Contrast',r:[-2,2,1]},
   {k:'saturation',l:'Saturation',r:[-2,2,1]},
   {k:'sharpness',l:'Sharpness',r:[-2,2,1]}]},
 {t:'EXPO',items:[
   {k:'aec',l:'Auto exposure',b:1},
   {k:'ae_level',l:'EV shift',r:[-2,2,1],need:'aec'},
   {k:'aec_value',l:'Shutter (manual)',r:[0,1200,10],needOff:'aec'},
   {k:'aec2',l:'Night mode (DSP AE)',b:1},
   {k:'agc',l:'Auto gain',b:1},
   {k:'agc_gain',l:'Gain / ISO',r:[0,30,1],needOff:'agc'},
   {k:'gainceiling',l:'Gain limit',o:GC,need:'agc'}]},
 {t:'COLR',items:[
   {k:'awb',l:'Auto white bal.',b:1},
   {k:'awb_gain',l:'AWB gain',b:1},
   {k:'wb_mode',l:'WB preset',o:WB,need:'awb_gain'},
   {k:'raw_gma',l:'Gamma correct',b:1},
   {k:'lenc',l:'Lens correction',b:1},
   {k:'bpc',l:'Black pixel fix',b:1},
   {k:'wpc',l:'White pixel fix',b:1}]},
 {t:'VIEW',items:[
   {k:'live_size',l:'Live view size',o:LIVE},
   {k:'hmirror',l:'Mirror',b:1},
   {k:'vflip',l:'Flip',b:1},
   {k:'dcw',l:'Downsize enable',b:1},
   {k:'lamp_on',l:'Lamp (torch)',b:1},
   {k:'lamp_level',l:'Lamp brightness',r:[0,255,5]}]}
];

/* ---- api ---- */
async function set(k,v){
  S[k]=v;
  try{ await fetch('/control?var='+k+'&val='+v); }catch(e){ toast('NO LINK'); }
  if(k=='live_size'){ restart(); }
  paint(); if(mode=='menu') rows();
}
async function status(){
  try{ S=await (await fetch('/status')).json(); paint(); }catch(e){ toast('NO LINK'); }
}
function toast(t,ms){
  const el=$('#toast'); el.textContent=t; el.classList.add('show');
  clearTimeout(el._t); el._t=setTimeout(()=>el.classList.remove('show'),ms||1400);
}
function restart(){ const i=$('#live'); i.removeAttribute('src'); setTimeout(()=>i.src=STREAM+'?t='+Date.now(),250); }

/* ---- osd ---- */
function label(list,v){ const f=list.find(x=>x[0]==v); return f?f[1]:v; }
function paint(){
  $('#oRes').textContent=label(SIZES,S.photo_size)||'—';
  $('#oIso').textContent=S.agc?'AUTO':String(S.agc_gain);
  $('#oEv').textContent=(S.ae_level>0?'+':'')+(S.ae_level||0);
  $('#oFx').textContent=label(FX,S.special_effect);
  $('#oShot').textContent=String(S.shot||0).padStart(4,'0');
  $('#oCard').textContent=S.sd?('SD '+S.sd_free+'MB FREE'):'NO CARD';
  $('#lampBtn').classList.toggle('on',!!S.lamp_on);
  $('#lampLvl').value=S.lamp_level||0;
  $('#foot').textContent=location.hostname+' · '+(S.sd?'CARD OK':'INSERT CARD');
}

/* ---- menu ---- */
function tabs(){
  $('#tabs').innerHTML=PAGES.map((p,i)=>'<button data-p="'+i+'"'+(i==page?' class="on"':'')+'>'+p.t+'</button>').join('');
  $('#tabs').querySelectorAll('button').forEach(b=>b.onclick=()=>{page=+b.dataset.p;tabs();rows();});
}
function rows(){
  const box=$('#rows'); box.innerHTML='';
  PAGES[page].items.forEach(it=>{
    const dim=(it.need&&!S[it.need])||(it.needOff&&S[it.needOff]);
    const r=document.createElement('div'); r.className='row'+(dim?' off':'');
    const l=document.createElement('div'); l.className='lb'; l.textContent=it.l; r.appendChild(l);
    if(it.b){
      const sw=document.createElement('div'); sw.className='sw'+(S[it.k]?' on':''); sw.innerHTML='<i></i>';
      sw.onclick=()=>{ if(!dim) set(it.k,S[it.k]?0:1); }; r.appendChild(sw);
    }else if(it.o){
      const s=document.createElement('select'); s.disabled=dim;
      it.o.forEach(o=>{ const e=document.createElement('option'); e.value=o[0]; e.textContent=o[1];
        if(o[0]==S[it.k]) e.selected=true; s.appendChild(e); });
      s.onchange=()=>set(it.k,s.value); r.appendChild(s);
    }else{
      const v=document.createElement('div'); v.className='val';
      v.textContent=S[it.k];
      const s=document.createElement('input'); s.type='range'; s.disabled=dim;
      s.min=it.r[0]; s.max=it.r[1]; s.step=it.r[2]; s.value=S[it.k];
      s.oninput=()=>v.textContent=s.value;
      s.onchange=()=>set(it.k,s.value);
      r.appendChild(s); r.appendChild(v);
    }
    box.appendChild(r);
  });
}

/* ---- shoot ---- */
async function shoot(){
  const b=$('#shutter'); if(b.disabled) return;
  b.disabled=true; b.classList.add('push');
  if(S.lamp_on) $('#flash').classList.add('go');
  toast('SAVING…',6000);
  try{
    const r=await (await fetch('/shoot')).json();
    toast(r.ok?('SAVED '+r.file.split('/').pop()):r.error.toUpperCase());
    if(r.ok){ S.shot=(S.shot||0)+1; paint(); restart(); }
  }catch(e){ toast('SHUTTER FAILED'); }
  setTimeout(()=>$('#flash').classList.remove('go'),340);
  b.disabled=false; b.classList.remove('push');
}

/* ---- gallery (loads one thumb at a time; the ESP32 has few sockets) ---- */
async function gallery(){
  const g=$('#grid'); g.innerHTML='<div class="empty">READING CARD…</div>';
  let d; try{ d=await (await fetch('/list')).json(); }catch(e){ g.innerHTML='<div class="empty">CARD UNREADABLE</div>'; return; }
  const f=(d.files||[]).sort((a,b)=>a.n<b.n?1:-1);
  if(!f.length){ $('#pcount').textContent='CARD · EMPTY'; g.innerHTML='<div class="empty">NO PHOTOS YET<br>PRESS THE RED BUTTON</div>'; return; }
  $('#pcount').textContent='CARD · '+f.length+' PHOTOS';
  g.innerHTML='';
  f.forEach(x=>{
    const c=document.createElement('div'); c.className='cell';
    const i=new Image(); i.dataset.src='/dl?f='+x.n; i.alt='';
    const t=document.createElement('span'); t.textContent=x.n.replace('.JPG','');
    c.appendChild(i); c.appendChild(t);
    c.onclick=()=>open1(x.n); g.appendChild(c);
  });
  const q=[...g.querySelectorAll('img')];
  (function next(){ const i=q.shift(); if(!i) return;
    i.onload=i.onerror=next; i.src=i.dataset.src; })();
}
function open1(n){
  $('#bigImg').src='/dl?f='+n;
  $('#bSave').href='/dl?f='+n+'&dl=1'; $('#bSave').setAttribute('download',n);
  $('#bDel').onclick=async()=>{ if(!confirm('Delete '+n+'?')) return;
    await fetch('/rm?f='+n); $('#big').classList.remove('on'); gallery(); status(); };
  $('#big').classList.add('on');
}

/* ---- mode ---- */
function go(m){
  mode=m;
  $('#rocker').querySelectorAll('button').forEach(b=>b.classList.toggle('on',b.dataset.m==m));
  document.querySelector('.lcd').classList.toggle('panelopen',m!='view');
  $('#menu').classList.toggle('on',m=='menu');
  $('#play').classList.toggle('on',m=='play');
  $('#big').classList.remove('on');
  $('#oMode').textContent=m=='play'?'REVIEW':(m=='menu'?'SETUP':'PHOTO');
  if(m=='play'){ $('#live').removeAttribute('src'); gallery(); }
  else if(!$('#live').getAttribute('src')) restart();
  if(m=='menu') rows();
}

/* ---- wire up ---- */
$('#rocker').querySelectorAll('button').forEach(b=>b.onclick=()=>go(b.dataset.m));
$('#shutter').onclick=shoot;
$('#bClose').onclick=()=>$('#big').classList.remove('on');
$('#reload').onclick=gallery;
$('#lampBtn').onclick=()=>set('lamp_on',S.lamp_on?0:1);
$('#lampLvl').onchange=e=>set('lamp_level',e.target.value);
$('#lampLvl').oninput=e=>{ clearTimeout(window._lt);
  window._lt=setTimeout(()=>fetch('/control?var=lamp_level&val='+e.target.value),120); };
document.addEventListener('keydown',e=>{ if(e.code=='Space'&&mode!='play'){e.preventDefault();shoot();} });
$('#live').onerror=()=>setTimeout(restart,1500);

tabs(); status(); restart(); setInterval(status,8000);
</script>
</body></html>)HTML";
