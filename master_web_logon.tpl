<html>
<head>
<title>Hackitt And Bodgitt WiFi Logon</title>
</head>
<body>

<style>

body {
    background-color: #404040;
    font-family: sans-serif;
}

#main {
    background-color: #d0d0FF;
    -moz-border-radius: 5px;
    -webkit-border-radius: 5px;
    border-radius: 5px;
    border: 2px solid #000000;
    width: 800px;
    margin: 0 auto;
    padding: 20px
}

.icon {
    background-color: transparent;
    width: 32px;
    height: 32px;
    display: inline-block;
}
</style>

<script type="text/javascript">
var t=function(a,b){return function(c,d){return a.replace(/#{([^}]*)}/g,function(a,f){return Function("x","with(x)return "+f).call(c,d||b||{})})}},s=function(a,b){return b?{get:function(c){return a[c]&&b.parse(a[c])},set:function(c,d){a[c]=b.stringify(d)}}:{}}(this.localStorage||{},JSON),p=function(a,b,c,d){c=c||document;d=c[b="on"+b];a=c[b]=function(e){d=d&&d(e=e||c.event);return(a=a&&b(e))?b:d};c=this},m=function(a,b,c){b=document;c=b.createElement("p");c.innerHTML=a;for(a=b.createDocumentFragment();b=
c.firstChild;)a.appendChild(b);return a},$=function(a,b){a=a.match(/^(\W)?(.*)/);return(b||document)["getElement"+(a[1]?a[1]=="#"?"ById":"sByClassName":"sByTagName")](a[2])},j=function(a){for(a=0;a<4;a++)try{return a?new ActiveXObject([,"Msxml2","Msxml3","Microsoft"][a]+".XMLHTTP"):new XMLHttpRequest}catch(b){}};
</script>



<script type="text/javascript">

var xhr=j();
var currAp="%currSsid%";

function createInputForAp(ap) {
    if (ap.essid=="" && ap.rssi==0) return;
    var div=document.createElement("div");
    div.id="apdiv";
    var rssi=document.createElement("div");
    var rssiVal=-Math.floor(ap.rssi/51)*32;
    rssi.className="icon";
    rssi.style.backgroundPosition="0px "+rssiVal+"px";
    var encrypt=document.createElement("div");
    var encVal="-64"; //assume wpa/wpa2
    if (ap.enc=="0") encVal="0"; //open
    if (ap.enc=="1") encVal="-32"; //wep
    encrypt.className="icon";
    encrypt.style.backgroundPosition="-32px "+encVal+"px";
    var input=document.createElement("input");
    input.type="radio";
    input.name="essid";
    input.value=ap.essid;
    if (currAp==ap.essid) input.checked="1";
    input.id="opt-"+ap.essid;
    var label=document.createElement("label");
    label.htmlFor="opt-"+ap.essid;
    label.textContent=ap.essid;
    div.appendChild(input);
    div.appendChild(rssi);
    div.appendChild(encrypt);
    div.appendChild(label);
    return div;
}

function getSelectedEssid() {
    var e=document.forms.wifiform.elements;
    for (var i=0; i<e.length; i++) {
        if (e[i].type=="radio" && e[i].checked) return e[i].value;
    }
    return currAp;
}


function scanAPs() {
    xhr.open("GET", "wifiscan.cgi");
    xhr.onreadystatechange=function() {
        if (xhr.readyState==4 && xhr.status>=200 && xhr.status<300) {
            var data=JSON.parse(xhr.responseText);
            currAp=getSelectedEssid();
            if (data.result.inProgress=="0" && data.result.APs.length>1) {
                $("#aps").innerHTML="";
                for (var i=0; i<data.result.APs.length; i++) {
                    if (data.result.APs[i].essid=="" && data.result.APs[i].rssi==0) continue;
                    $("#aps").appendChild(createInputForAp(data.result.APs[i]));
                }
                window.setTimeout(scanAPs, 20000);
            } else {
                window.setTimeout(scanAPs, 1000);
            }
        }
    }
    xhr.send();
}


window.onload=function(e) {
    scanAPs();
};
</script>
</head>
<body>
<div id="main">
<p>
Current WiFi mode: %WiFiMode%
</p>
<p>
Note: %WiFiapwarn%
</p>
<form name="wifiform" action="connect.cgi" method="post">
<p>
To connect to a WiFi network, please select one of the detected networks...<br>
<div id="aps">Scanning...</div>
<br>
WiFi password, if applicable: <br />
<input type="text" name="passwd" val="%WiFiPasswd%"> <br />
<input type="submit" name="connect" value="Connect!">
</p>
</div>


</body>
</html>