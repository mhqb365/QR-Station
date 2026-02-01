#include "WebPage.h"

const char* config_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<style>body{font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif;background:#0f0c29;background:linear-gradient(45deg,#24243e,#302b63,#0f0c29);color:#fff;margin:0;padding:20px;min-height:100vh;}
.container{max-width:600px;margin:0 auto;}.card{background:rgba(255,255,255,0.05);backdrop-filter:blur(10px);border:1px solid rgba(255,255,255,0.1);padding:25px;border-radius:20px;margin-bottom:20px;box-shadow:0 10px 30px rgba(0,0,0,0.5);}
h2{color:#00d2ff;margin-top:0;}h3{color:#3a7bd5;background:#fff;display:inline-block;padding:2px 10px;border-radius:5px;font-size:0.9em;margin-bottom:15px;}
.form-group{margin-bottom:15px;text-align:left;}label{display:block;margin-bottom:5px;font-size:0.85em;color:#aaa;}input,select{width:100%;padding:12px;background:rgba(0,0,0,0.2);border:1px solid rgba(255,255,255,0.2);border-radius:10px;color:#fff;box-sizing:border-box;transition:0.3s;cursor:pointer;font-size:16px;-webkit-appearance:none;}input:focus,select:focus{outline:none;border-color:#00d2ff;background:rgba(0,0,0,0.4);}option{background:#302b63;color:#fff;}
.btn{background:linear-gradient(to right,#00d2ff,#3a7bd5);border:none;color:white;padding:15px;font-size:16px;cursor:pointer;border-radius:10px;font-weight:bold;width:100%;margin-top:10px;box-shadow:0 5px 15px rgba(0,210,255,0.3);}
.btn:active{transform:scale(0.98);}.nav{display:flex;gap:10px;margin-bottom:25px;}.nav a{flex:1;text-decoration:none;background:rgba(255,255,255,0.1);color:#fff;padding:10px;border-radius:10px;text-align:center;font-size:0.9em;border:1px solid transparent;}.nav a.active{background:rgba(0,210,255,0.2);border-color:#00d2ff;}
.tabs-nav{display:flex;gap:5px;margin-bottom:15px;}.tab-btn{flex:1;padding:10px;background:rgba(255,255,255,0.1);border:1px solid rgba(255,255,255,0.1);color:#fff;border-radius:10px;cursor:pointer;font-size:0.9em;transition:0.3s;font-weight:bold;}.tab-btn.active{background:rgba(0,210,255,0.2);border-color:#00d2ff;color:#00d2ff;}.tab-pane{display:none;}.tab-pane.active{display:block;}
.switch{position:relative;display:inline-block;width:34px;height:20px;}.switch input{opacity:0;width:0;height:0;}.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#444;transition:0.4s;}.slider:before{position:absolute;content:'';height:14px;width:14px;left:3px;bottom:3px;background-color:white;transition:0.4s;}input:checked+.slider{background-color:#00d2ff;}input:checked+.slider:before{transform:translateX(14px);}.slider.round{border-radius:20px;}.slider.round:before{border-radius:50%;}
.status-dot{width:8px;height:8px;border-radius:50%;box-shadow:0 0 5px rgba(255,255,255,0.2);}.status-dot.connected{background-color:#00ff00;box-shadow:0 0 8px rgba(0,255,0,0.5);}.status-dot.disconnected{background-color:#ff3b3b;box-shadow:0 0 8px rgba(255,59,59,0.5);}.status-dot.connecting{background-color:#ffcc00;box-shadow:0 0 8px rgba(255,204,0,0.5);animation:pulse 1s infinite alternate;}.status-dot.error{background-color:#ff00ff;box-shadow:0 0 8px rgba(255,0,255,0.5);}@keyframes pulse{from{opacity:0.5;transform:scale(0.8);}to{opacity:1;transform:scale(1.1);}}
</style></head><body>
<div class='container'><div class='nav'><a href='/' class='active'>Cài đặt</a><a href='/updatefw'>Nâng cấp Firmware</a></div>
<h2 style='text-align: center;'>QR Station</h2>
<form action='/save' method='POST'>
<div id='g1'>
<div class='tabs-nav'>
<button type='button' class='tab-btn active' onclick='openTab("g1",0)'>QR 1</button>
<button type='button' class='tab-btn' onclick='openTab("g1",1)'>QR 2</button>
<button type='button' class='tab-btn' onclick='openTab("g1",2)'>QR 3</button>
</div>
<div class='tab-pane active'><div class='card'><h3>QR 1</h3>
<input type='hidden' name='bn0' id='bnV0' value='%BN0%'>
<div class='form-group'><label>Ngân hàng</label><select name='bin0' id='bin0' onchange='upd(0)'></select></div>
<div class='form-group'><label>Số tài khoản</label><input type='text' name='acc0' value='%ACC0%'></div>
<div class='form-group'><label>Chủ tài khoản</label><input type='text' name='on0' value='%ON0%'></div></div></div>
<div class='tab-pane'><div class='card'><h3>QR 2</h3>
<input type='hidden' name='bn1' id='bnV1' value='%BN1%'>
<div class='form-group'><label>Ngân hàng</label><select name='bin1' id='bin1' onchange='upd(1)'></select></div>
<div class='form-group'><label>Số tài khoản</label><input type='text' name='acc1' value='%ACC1%'></div>
<div class='form-group'><label>Chủ tài khoản</label><input type='text' name='on1' value='%ON1%'></div></div></div>
<div class='tab-pane'><div class='card'><h3>QR 3</h3>
<input type='hidden' name='bn2' id='bnV2' value='%BN2%'>
<div class='form-group'><label>Ngân hàng</label><select name='bin2' id='bin2' onchange='upd(2)'></select></div>
<div class='form-group'><label>Số tài khoản</label><input type='text' name='acc2' value='%ACC2%'></div>
<div class='form-group'><label>Chủ tài khoản</label><input type='text' name='on2' value='%ON2%'></div></div></div>
</div>
<h4 style='text-align: center;margin: 20px 0;'>OR</h4>
<div class='card'><h3>Cài đặt nhanh</h3>
<div class='form-group'><label>Cài đặt nhanh các thông tin ngân hàng bằng file JSON</label></div>
<div style='display:flex;gap:10px;'>
<button type='button' class='btn' style='flex:1;background:#ff416c;' onclick='document.getElementById("fi").click()'>Nhập file</button>
<a href='/export' style='flex:1;text-decoration:none;'><button type='button' class='btn' style='background:#3a7bd5;'>Xuất file</button></a>
</div><input type='file' id='fi' style='display:none' accept='.json' onchange='imp(this)'></div>
<div id='g2'>
<div class='tabs-nav'>
<button type='button' class='tab-btn active' onclick='openTab("g2",0)'>WiFi</button>
<button type='button' class='tab-btn' onclick='openTab("g2",1)'>MQTT</button>
<button type='button' class='tab-btn' onclick='openTab("g2",2)'>Admin</button>
</div>
<div class='tab-pane active'>
<div class='card'>
<div class='form-group'><label>Tên WiFi</label>
<div style='display:flex;gap:10px;'>
<select name='ws' id='ws' style='flex:1;'>%WIFI_LIST%</select>
<button type='button' id='scb' class='btn' style='width:auto;margin:0;padding:0 15px;background:#3a7bd5;' onclick='sc()'>TÌM KIẾM</button>
</div></div>
<div class='form-group'><label>Mật khẩu</label><input type='password' name='wp' id='wp' value='%WP%'></div>
<button type='button' id='cnb' class='btn' style='background:#f39c12;margin-top:10px;' onclick='cn()'>KẾT NỐI</button>
<div id='wfl' style='margin-top:20px;'></div>
</div></div>
<div class='tab-pane'>
<div class='card'>
<div style='display:flex;justify-content:space-between;align-items:center;margin-bottom:10px;'>
<div style='display:flex;align-items:center;gap:8px;'>
<h3 style='margin:0;font-size:14px;'>Thông báo số dư</h3>
<div id='mqtt_status_dot' class='status-dot disconnected' title='Disconnected'></div>
</div>
<label class='switch'>
<input type='checkbox' id='mqtt_toggle' %MQTT_ENABLED%>
<span class='slider round'></span>
</label>
</div>
<div id='mqtt_fields'>
<div class='form-group'><label>Máy chủ MQTT</label><input type='text' name='ms' value='%MS%'></div>
<div class='form-group'><label>Tài khoản</label><input type='text' name='mu' value='%MU%'></div>
<div class='form-group'><label>Mật khẩu</label><input type='password' name='mp' value='%MP%'></div>
<input type='hidden' name='me' id='mqtt_enabled_val' value='%MQTT_ENABLED_VAL%'>
</div></div></div>
<div class='tab-pane'>
<div class='card'>
<div class='form-group'><label>Tài khoản</label><input type='text' name='au' value='%AU%'></div>
<div class='form-group'><label>Mật khẩu</label><input type='password' name='ap' value='%AP%'></div>
</div></div>
</div>


<button type='submit' class='btn' style='background:linear-gradient(to right,#00b894,#00d2ff);margin-bottom:20px;'>LƯU THAY ĐỔI</button>

<div class='card' style='border-color:rgba(255,118,117,0.3);margin-top:10px;'>
<div style='display:flex;gap:10px;'>
<button type='button' class='btn' style='flex:1;background:#3a7bd5;' onclick='rb()'>Khởi động lại</button>
<button type='button' class='btn' style='flex:1;background:#d63031;' onclick='rst()'>Khôi phục gốc</button>
</div></div>
</form>
<script>
const banks=[{b:'970436',n:'Vietcombank'},{b:'970415',n:'VietinBank'},{b:'970418',n:'BIDV'},{b:'970405',n:'Agribank'},{b:'970407',n:'Techcombank'},{b:'970422',n:'MBBank'},{b:'970423',n:'TPBank'},{b:'970416',n:'ACB'},{b:'970432',n:'VPBank'},{b:'sacom',b:'970403',n:'Sacombank'},{b:'970441',n:'VIB'},{b:'970425',n:'AnBinhBank'},{b:'970437',n:'HDBank'}];
const vals=['%BIN0%','%BIN1%','%BIN2%'];
function openTab(p,i){
const g=document.getElementById(p);
g.querySelectorAll('.tab-pane').forEach(e=>e.classList.remove('active'));
g.querySelectorAll('.tab-btn').forEach(e=>e.classList.remove('active'));
g.querySelectorAll('.tab-pane')[i].classList.add('active');
g.querySelectorAll('.tab-btn')[i].classList.add('active');
}
function upd(i){const s=document.getElementById('bin'+i);document.getElementById('bnV'+i).value=s.options[s.selectedIndex].text;}
function imp(e){const f=e.files[0];if(!f)return;const r=new FileReader();r.onload=function(x){const d=JSON.parse(x.target.result);d.forEach((a,i)=>{if(i>2)return;document.getElementById('bin'+i).value=a.bin;document.getElementsByName('acc'+i)[0].value=a.acc;document.getElementsByName('on'+i)[0].value=a.on;upd(i);});};r.readAsText(f);}
function rb(){if(confirm('Khởi động lại thiết bị?')){window.location.href='/reboot';}}
function rst(){if(confirm('Bạn có chắc chắn? Tất cả dữ liệu sẽ bị xóa!')){window.location.href='/reset';}}
function sc(){const b=document.getElementById('scb');b.innerHTML='...';b.disabled=true;
fetch('/scan').then(r=>r.json()).then(d=>{
const s=document.getElementById('ws');s.innerHTML='';
d.forEach(w=>{const o=document.createElement('option');o.value=w.s;o.text=w.s+' ('+w.r+'dBm)';s.appendChild(o);});
b.innerHTML='SCAN';b.disabled=false;});}
function cn(){const s=document.getElementById('ws').value;const p=document.getElementById('wp').value;const b=document.getElementById('cnb');
if(!s){alert('Vui lòng chọn WiFi trước');return;}b.innerHTML='Đang kết nối';b.disabled=true;
fetch('/connect_wifi?s='+encodeURIComponent(s)+'&p='+encodeURIComponent(p)).then(r=>r.text()).then(t=>{alert(t);if(t.includes('Connected'))lw();b.innerHTML='KẾT NỐI';b.disabled=false;});}
function lw(){fetch('/list_wifi').then(r=>r.json()).then(d=>{
const l=document.getElementById('wfl');l.innerHTML='<p style="font-size:0.8em;color:#aaa;border-bottom:1px solid rgba(255,255,255,0.1);padding-bottom:5px;margin-bottom:10px;">Danh sách đã lưu</p>';
d.forEach((w,i)=>{
const v=document.createElement('div');v.className='form-group';v.style='display:flex;justify-content:space-between;align-items:center;background:rgba(255,255,255,0.05);padding:8px 12px;border-radius:8px;margin-bottom:5px;';
v.innerHTML=`<span style='font-size:0.9em;'>${w.s}</span><button type='button' style='background:none;border:none;color:#ff416c;cursor:pointer;font-size:1.2em;padding:0 5px;' onclick='dw(${i})'>&times;</button>`;
l.appendChild(v);});});}
function dw(i){if(confirm('Xóa mạng này?')){fetch('/del_wifi?i='+i).then(()=>lw());}}
function toggleMqttFields(){const t=document.getElementById('mqtt_toggle');const f=document.getElementById('mqtt_fields');const v=document.getElementById('mqtt_enabled_val');if(t.checked){f.style.opacity='1';f.style.pointerEvents='auto';v.value='1';}else{f.style.opacity='0.5';f.style.pointerEvents='none';v.value='0';}}
const mqttToggle=document.getElementById('mqtt_toggle');if(mqttToggle){mqttToggle.addEventListener('change',toggleMqttFields);toggleMqttFields();}
function updateMqttStatus(){fetch('/api/mqtt_status').then(r=>r.json()).then(d=>{const dot=document.getElementById('mqtt_status_dot');if(!dot)return;dot.className='status-dot';if(d.status==='connected'){dot.classList.add('connected');dot.title='Connected';}else if(d.status==='connecting'){dot.classList.add('connecting');dot.title='Connecting...';}else if(d.status==='disabled'){dot.classList.add('disconnected');dot.title='Disabled';}else{dot.classList.add('disconnected');dot.title='Disconnected';}}).catch(()=>{});}
updateMqttStatus();setInterval(updateMqttStatus,3000);
for(let i=0;i<3;i++){const s=document.getElementById('bin'+i);banks.forEach(bk=>{const o=document.createElement('option');o.value=bk.b;o.text=bk.n;if(bk.b===vals[i])o.selected=true;s.appendChild(o);});upd(i);}
lw();
</script></div></body></html>
)rawliteral";

const char* update_html = R"rawliteral(
<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<style>body{font-family:sans-serif;background:#0f0c29;color:#fff;text-align:center;padding:30px;}
.card{background:rgba(255,255,255,0.05);backdrop-filter:blur(10px);padding:30px;border-radius:20px;display:inline-block;border:1px solid rgba(255,255,255,0.1);}
.btn{background:linear-gradient(to right,#ff416c,#ff4b2b);border:none;color:white;padding:12px 24px;font-size:16px;margin:20px 0;cursor:pointer;border-radius:8px;font-weight:bold;width:100%;}
input[type=file]{margin:20px 0;background:#3d3d3d;padding:15px;border-radius:10px;width:100%;box-sizing:border-box;color:#fff;font-size:16px;}.nav{margin-bottom:30px;}</style></head><body>
<div class='nav'><a href='/' style='color:#00d2ff;text-decoration:none;'>Quay lại</a></div>
<div class='card'><h2>Nâng cấp Firmware</h2>
<form method='POST' action='/update' enctype='multipart/form-data'>
<input type='file' name='update' accept='.bin'><br>
<input type='submit' value='NÂNG CẤP' class='btn'>
</form></div></body></html>
)rawliteral";
