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

function myTableDeleteRowsN(tableid, rowstokeep) {
  var table = document.getElementById(tableid);
  while (table.rows.length > rowstokeep) {
    table.deleteRow(rowstokeep);
  }
};
function myStartTimer(timedelayms) {
  setTimeout(myStopTimer, timedelayms);
};
function myStopTimer() {
  location.reload();
};
function mySafeInnerHtml(value) {
  var ret = "";
  if (value != null) {
    ret = value.innerHTML;
  }
  return ret;
}
function myConvertTableToInputs(tableid) {
  //convert column values [ptext][ptype TNSBRC][value todisplay][argname tosend][min (R=radiolabel)][max (R=1-checked)] to html input fields
  // [ex name][T][32]= example |ex name|32|
  // [ex for number][N][44][sendparname][0][50]=|ex for number|input field type number min=0 max=50, def value=44
  var table = document.getElementById(tableid);
  for (var r = 0; r < table.rows.length; r++) {
    //var ptext = safeinnerhtml(table.rows[r].cells[0]);
    var ptype = mySafeInnerHtml(table.rows[r].cells[1]);
    var pval = mySafeInnerHtml(table.rows[r].cells[2]);
    var pname = mySafeInnerHtml(table.rows[r].cells[3]);
    var pmin = mySafeInnerHtml(table.rows[r].cells[4]);
    var pmax = mySafeInnerHtml(table.rows[r].cells[5]);
    var inshtml = ptype;
    for (var c = 2; c < table.rows[r].cells.length; c++) {
      table.rows[r].cells[c].innerHTML = "";
    }
    if (ptype == "T") {
      inshtml = "<b>" + pval + "</b>";
    }
    else if (ptype == "N") {
      inshtml = "<input type='number' name='" + pname + "' value='" + pval + "' min='" + pmin + "' max='" + pmax + "' id='i" + pname + "'>";
    }
    else if (ptype == "S") {
      inshtml = "<input style='width: 50%' type='range' name='" + pname + "' value='" + pval + "' min='" + pmin + "' max='" + pmax + "' id='i" + pname +
        "' oninput='io" + pname + ".value=i" + pname + ".value'>" + "<output id='io" + pname + "'>" + pval + "</output>";
    }
    else if (ptype == "B") {
      inshtml = "<input style='width: 5%;' type='range' name='" + pname + "' value='" + pval + "' min='0' max='1' id='i" + pname +
        "' oninput='io" + pname + ".value=i" + pname + ".value'>" + "<output id='io" + pname + "'>" + pval + "</output>";
    }
    else if (ptype == "R") {
      inshtml = "<i>" + pmin + "</i><input type='radio' name='" + pname + "' value='" + pval;
      if (pmax == 1) inshtml += "' checked>";
      else inshtml += "'>";
    }
    else if (ptype == "C") {
      inshtml = "<input type='text' name='" + pname + "' value='" + pval + "' minlength='" + pmin + "' maxlength='" + pmax + "' id='i" + pname + "'>";
    }
    if (table.rows[r].cells[1] != null) table.rows[r].cells[1].innerHTML = inshtml;
  }
};
