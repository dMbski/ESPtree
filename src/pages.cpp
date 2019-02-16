#ifndef PAGES_CPP
#define PAGES_CPP
#include <pgmspace.h>
//macro R"= conflicts with F macro

char page_main[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type" content="text/html; charset=UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>xmas tree lights</title>
<link href="data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAkAAAATAAAAHwAAACwCAgI4ME525xIeMHoAAAA4AAAALAAAAB8AAAATAAAACQAAAAAAAAAAAAAAAAAAAAAAAAAABTAgQQAAAAcAAAANTnqm/098qP9rmcP/O12K/wAAAA4KYUDhDm9I/w1qRf8AAAAAAAAAAAAAAAAKX0D/HZ1e/xqdXf8xp23/TbOC/2K/lf9Hqn7/QKl7/0Kug/8hZ03/Gohf/wd0Tf8Wh1f/AAJVZwAAAAAAAAAADR69/xAkv/8LcEL/C2ZB/xSCS/9JvIr/QbeE/y2rd/8ZO7X/JEbM/wIl0P8AQGH/AAO4/wAAAAAAAAAAAAAAAAAAAAAqUtH/HJFb/x+gYf8/qHb/GVdG/0V1//8ea+r/DDfL/8Hc+P8aNcb/AoNR/wpgQJgAAAAAAAAAAAAAAAAAAAAAAAAAAAxxSP8RmVb/ACHE/0q8i/9avZT/UK+F/yqSZf8Xdk//Ai/a/xGZVgMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZ3/wx8Q/9Guof/N7F+/xKbZf8AhFL/AGpF/06B4/8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAUvH38dl1v/Nalw/zqref8knmv/EIFV/xKQVP8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAZM8VAF4hU/w1cPf8ZNMb/BF3g/wADsf8FJZr/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABOVVv8nS8//OWja/xR1Tf8Mb0P/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC2FC/zGseP8BiVX/AELU/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACaeZf82r3z/GUz//wpgQP8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABen/waHU/9rmuv/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAC5uyvdFi7X/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABQr9v/MGyv/wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA/v8AAPwjAACAAwAAgAMAAMADAADgDwAA8A8AAPgfAAD4HwAA+D8AAPw/AAD8PwAA/j8AAP5/AAD+fwAA//8AAA==" rel="icon" type="image/x-icon" />
</head><body><HR><table style="width: 100%; border:thick; text-align:center; font-size:3vh; vertical-align:middle; background:orange"><tr><td colspan="4"><a href="/">Menu</a></td>
</tr><tr style="background-color:#FFCC99"><td><a href="/show" target="framed">Show</a></td><td><a href="/configshow" target="framed">Config Show</a></td><td><a href="/configtree" target="framed">Config tree</a></td>
<td><a href="/configwifi" target="framed">Config WiFi</a></td></tr></table><HR><iframe src="/infopage" id="iframed" name="framed" seamless width="100%" height="100%" style="border: turquoise">Click on button from menu to proceed.</iframe>
</body></html>
)=====";

char page_functions[] PROGMEM = R"=====(
function myLoadURLToString(a,b,c){var d=new XMLHttpRequest;d.open("GET",a,!0),d.onreadystatechange=function(){4==d.readyState&&(200==d.status?(console.log("Loaded: "+d.responseText),sstring=d.responseText,b(c)):(console.log("Error loading:"+a),sstring=""))},d.send(null)}function myStringToTable(a){var b=/{/gi,c=sstring.replace(b,"").split("}"),d=document.getElementById(a);for(let b of c){var e,f=b.split(";"),g=d.insertRow(d.rows.lenght);for(e=0;e<f.length;e++)g.insertCell(e).innerHTML=f[e]}}function myTableDeleteRowsN(a,b){for(var c=document.getElementById(a);c.rows.length>b;)c.deleteRow(b)}function myStartTimer(a){setTimeout(myStopTimer,a)}function myStopTimer(){location.reload()}
)=====";

char page_configwifi[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type" content="text/html; charset=UTF-8"></head>
<script src="/functions.js"></script>
<script>
var sstring="";function cmdloadwifilist(){myLoadURLToString("/wifiliststring",myStringToListSSID)}function cmdloadrescan(){myLoadURLToString("/rescancmd",myAfterRescan)}function cmdloadwps(){myLoadURLToString("/wpscmd",myAfterRescan)}function myStringToListSSID(){myTableDeleteRowsN("issidtable",1),myStringToTable("issidtable");for(var a,b=document.getElementById("issidtable"),c=1;c<b.rows.length;c++)a=b.rows[c].cells[0].innerHTML,b.rows[c].cells[0].innerHTML=a+"<input type='radio' name='radio1' value='"+a+"'>"}function cmdchosessid(){var a=document.forms[1];for(let b of a)b.checked&&(document.getElementById("istaname").value=b.value)}function myAfterRescan(){document.getElementById("irescantext").innerHTML=sstring,myStartTimer(15e3)}function cmdsubmitsave(){var a=!0;return 32<document.getElementById("istaname").value.length&&(a=!1,alert("SSID name must be < 33")),64<document.getElementById("istapass").value.length&&(a=!1,alert("Password name must be < 65")),a}
</script>
<body onload="cmdloadwifilist()"><H2 style="text-align: center">WiFi connection configuration:</H2>
<form id="isaveform" method="post" action="/cfgwifisave" onsubmit="return cmdsubmitsave()" style="background-color:#CCFFCC; text-align:center">
<p><b>Standalone AP<input type='radio' name='wlanmode' value='AP'></b><br><sup>Default Web IP: 192.168.4.1<sup></p><p><b>Connect to STA<input type='radio' name='wlanmode' value='STA' checked></b></p>
<hr><p><b>Choose network from the list below or enter SSID:</p><input id="istaname" name="staname" type="text" value="" style="height: 40px; width: 50vw"><br>Password:<br></b>
<input id="istapass" name="stapass" type="password" value="" style="height: 40px; width: 50vw"><br><br><input type="reset" value="Clear">_____<input type="submit" value="Save"><br><br></form>
<form id="issidlist" style="background-color: darkseagreen; text-align:center" onchange="cmdchosessid()"><b>Networks list</b><br><br><table id="issidtable" style="text-align: center; width: 80%;">
<tr style="font-weight: bold;"><td style="width: 60vw">SSID:</td><td style="width: 10vw">RSSI:</td><td style="width: 30vw">Encryption:</td></tr></table></form>
<div id="irescantext" style="text-align: center"><button onclick="cmdloadrescan()">Rescan!</button><br><hr><br><br>First push router's WPS button, then click <button onclick="cmdloadwps()">Connect WPS</button> and wait.</div></body>
</html>
)=====";

char page_infopage[] PROGMEM = R"=====(
<html><head><meta content="en" http-equiv="Content-Language"></head>
<script src="/functions.js"></script><script>var sstring = "";
function RunThis() {myTableDeleteRowsN("idinfotable", 1);myLoadURLToString("/infostring", myStringToTable, "idinfotable");};</script>
<body onload="RunThis()"><table id="idinfotable" style="width: 80%; font-weight: normal; background-color:rgb(209, 255, 209);"><tr style="font-weight: bold">
<td>Name</td><td>Value</td><td></td></tr></table><button onclick="RunThis()">Refresh info</button></body>
</html>
  )=====";

char page_restartpage1[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type"  content="text/html; charset=UTF-8"></head>
<body><div style="text-align: center"><a href="/restartcmd">Click to restart ESP.</a></div></body>
</html>
  )=====";

char page_restartpage2[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type"  content="text/html; charset=UTF-8"></head>
<script type="text/javascript">function Redirect(){top.window.location="/";}
document.write("Wait, You will be redirected to a new page in 20 seconds");setTimeout('Redirect()', 20000);</script><body><h1>Restarting...</h1></body>
</html>
  )=====";
char page_configtree[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type" content="text/html; charset=UTF-8"></head><script src="/functions.js"></script><body><H2 style="text-align: center">ESP tree configuration:</H2>
<div style="text-align: center; font-size: small">This page does not load actual values!<br>For these, See table at	bottom.</div><br><div style="background-color: #CCFFFF"><br>
<form method="post" action="/cfgtree1save" style="background-color: #00FFFF; width: 90%; left:5%;position:relative"><table style="width: 100%;">
<tr><td style="width: 20%">Standalone AP name</td><td><input name="apname" size="32" style="width: 100%" type="text" value="XMAS_TREE_AP"></td></tr>
<tr><td>OTA enable</td><td>Enable<input name="otaenable" type="radio" value="1">Disable<input checked name="otaenable" type="radio" value="0"></td></tr>
<tr><td>AP mode, shutdown WIFI after</td><td><input name="apwifioff" style="width: 15%" type="number" value="300" min="300" max="43200">sec</td></tr>
<tr><td>Autochange Effect<br><sup>0 disable</sup></td><td><input name="autoeffect" style="width: 15%" type="number" value="180" min="60" max="3600">sec</td></tr></table><br>
<input type="reset" value="Clear">_____<input type="submit" value="Save"></form><br><b>Neopixel Stripe configuration:</b><form method="post" action="/cfgtree2save" style="background-color:skyblue ; width: 90%; left:5%;position:relative">
<table style="width: 100%;"><tr><td style="width: 20%">Neopixel pin<br><sup>Ensure right pin</sup></td><td><input name="pinno" style="width: 15%" type="number" value="1" min="1" max="16"></td></tr>
<tr><td>Neopixel type<br><sup>Right collor assigment</sup></td><td><select name="neopixeltype" style="width: 15%"><option value="6">NEO_RGB</option><option value="9">NEO_RBG</option><option value="82">NEO_GRB</option><option value="161">NEO_GBR</option>
<option value="88">NEO_BRG</option><option value="164">NEO_BGR</option></select></td></tr><tr><td>Number of LEDs<br><sup>Works best to 200, check update time.</sup></td><td><input name="ledcounts" style="width: 15%" type="number" value="50" min="2" max="300"></td></tr><tr><td>Maximum Amperage</td><td><input name="ampmax" style="width: 15%" type="number" value="500" min="20" max="60000">mA</td>
</tr></table><br><input type="reset" value="Clear">_____<input type="submit" value="Save"></form></div><iframe src="/infopage" id="iframed" name="framed" seamless width="100%" height="100%" style="border: turquoise">Click	on button from menu to proceed.</iframe>
<div style="text-align: center">Restart ESP<a href="/restartcmd">&#8635;</a><br>Upgrade firmware<a href="/firmware">&#8685;</a></div></body>
</html>
  )=====";
char page_show[] PROGMEM = R"=====(
<html><head><meta http-equiv="content-type" content="text/html; charset=UTF-8"></head><script src="/functions.js"></script>
<script>
var wsSocket,sstring="",ledbuffer=[],numleds=0,ledno=0,PFbuffer=[],rdata=!1;function addToPFbuffer(a){var b=parseInt(a,10),c=(document.getElementById("ipfdrawer").width-10)/5;if(PFbuffer.length==c)PFbuffer.shift(),PFbuffer.push(b);else if(PFbuffer.length<c)PFbuffer.push(b);else for(PFbuffer.push(b);PFbuffer.length>c;)PFbuffer.shift()}function myDrawPF(){document.getElementById("ipfdrawer").width=document.documentElement.clientWidth-10;var a=document.getElementById("ipfdrawer"),c=a.getContext("2d"),d=a.width/PFbuffer.length;c.fillRect(0,0,a.width,1);for(var e=0;e<PFbuffer.length;e++){var f=255-PFbuffer[e],h=54+2*PFbuffer[e],g=128-PFbuffer[e],b="#"+f.toString(16)+h.toString(16)+g.toString(16);c.fillStyle=b,c.fillRect(d*e,a.height-PFbuffer[e],d,a.height)}c.fillRect(0,50,a.width,1),c.fillStyle="#000000",c.fillText("Powerfactor",0,50)}function myDrawLeds(){document.getElementById("ileddrawer").width=document.documentElement.clientWidth-10;for(var a,b=document.getElementById("ileddrawer"),c=b.getContext("2d"),d=b.width/ledbuffer.length,e=0;e<ledbuffer.length;e++)a="#"+ledbuffer[e],c.fillStyle=a,c.fillRect(d*e,0,d,b.height)}function myStartPage(){addToPFbuffer("100"),myDrawPF(),myDrawLeds(),myInitWS(),cmdloadeffectlist()}function cmdloadeffectlist(){myLoadURLToString("/effectliststring",myStringToListEffect)}function myStringToListEffect(){myTableDeleteRowsN("iefftable",1),myStringToTable("iefftable");for(var a,b=document.getElementById("iefftable"),c=1;c<b.rows.length;c++)a=b.rows[c].cells[0].innerHTML,0<a.length&&(b.rows[c].cells[0].innerHTML="<input type='radio' name='radio1' value='"+a+"'>"+a);wsSocket.send("!!"),rdata=!0}function myProcessWS(a){if("!"==a.substr(0,1)){if("#"==a.substr(1,1))for(var b=document.getElementById("iefftable"),c=1;c<b.rows.length;c++)b.rows[c].style.fontWeight=c-1==a.substr(2)?"bold":"normal";else if("f"==a.substr(1,1))document.getElementById("ipowerfact").innerHTML="Power factor:"+a.substr(2)+" %",addToPFbuffer(a.substr(2)),myDrawPF();else if("p"==a.substr(1,1))document.getElementById("ipowerneed").innerHTML="Power need:"+a.substr(2)+" mA";else if("t"==a.substr(1,1))document.getElementById("itasktime").innerHTML="Effect&WSuptade time:"+a.substr(2)+" msec";else if("r"==a.substr(1,1)){var d=parseInt(a.substr(2),10);document.getElementById("iruntime").innerHTML="Runtime:"+d+"sec"}else if("L"!=a.substr(1,1))"N"==a.substr(1,1)&&(ledno=a.substr(2),console.log("LED no: "+ledno));else if(numleds=a.substr(2),ledbuffer.length<numleds)for(var e=0;e<numleds;e++)ledbuffer[e]="FFFFFF";}else if("*"==a.substr(0,1)){var f=a.substr(1);ledno>=numleds&&(ledno=0),ledbuffer[ledno]=f,ledno++}myDrawLeds()}function myInitWS(){wsSocket=new WebSocket("ws://"+window.location.hostname+":81/"),wsSocket.onmessage=function(a){document.getElementById("iwsconsole").value=a.data,myProcessWS(a.data)},wsSocket.onerror=function(a){alert("WebSocket error observed:"+a)},wsSocket.onclose=function(){alert("WebSocket closed")}}function cmdchoseeff(){var a=document.forms[0];for(let b of a)b.checked&&wsSocket.send("!#"+b.value)}function cmdpreviewleds(){wsSocket.send("!p")}function cmdpausedata(){rdata?wsSocket.send("!s"):wsSocket.send("!!"),rdata=!rdata}
</script>
<body onload="myStartPage()">
<H2 style="text-align: center">Select effect:</H2><div style="background:navajowhite;"><form id="iefflist" onchange="cmdchoseeff()" style="margin-left:10%"><table id="iefftable" style="text-align: left; width: 80%; font-size: large"><tr style="font-weight: bold;"><td style="width: 5%">No:</td><td style="width: 50%">Effect Name:</td></tr></table></form></div>
<hr><button onclick="cmdpausedata()" style="float: right;">Pause/start data</button><table style="background:lemonchiffon; width: 100%;"><tr><td id="ipowerneed">1</td><td id="itasktime">2</td>
</tr><tr><td id="ipowerfact">1</td><td id="iruntime">2</td></tr></table><canvas width="600" height="100" id="ipfdrawer"></canvas>
<hr><button onclick="cmdpreviewleds()" style="float: right;">Preview LEDs</button><br><canvas width="600" height="20" id="ileddrawer"></canvas><br><hr><div><samp><b>WebSocket debug:</b><br><textarea id="iwsconsole" style="width: 20vw; height: 10vh;"></textarea></samp></div></body>
</html>
  )=====";
#endif