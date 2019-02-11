function myLoadURLToString(spurl, callbackf, callbackvar) {
  var req = new XMLHttpRequest();
  req.open('GET', spurl, true);
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
      if (req.status == 200) {
        console.log("Loaded: " + req.responseText);
        sstring = req.responseText;
        callbackf(callbackvar);
      }
      else {
        console.log("Error loading:" + spurl);
        sstring = "";
      }
    }
  };
  req.send(null);
};
function myStringToTable(tableid) {
  var regexstart = /{/gi;
  var ssrows = sstring.replace(regexstart, '').split('}');
  var table = document.getElementById(tableid);
  for (let ssr of ssrows) {
    var sscols = ssr.split(';');
    var row = table.insertRow(table.rows.lenght);
    var i;
    for (i = 0; i < sscols.length; i++) {
      row.insertCell(i).innerHTML = sscols[i];
    }
  };
};

function myTableDeleteRowsN(tableid, rowstokeep){
  var table = document.getElementById(tableid);
	while (table.rows.length > rowstokeep) {
    table.deleteRow(rowstokeep);
  }  
};
function myStartTimer(timedelayms) {
  setTimeout(myStopTimer, timedelayms);
}
function myStopTimer() {
  location.reload();
}