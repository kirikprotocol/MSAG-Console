function serializeForm(frm) {
  var result = {};
  $.each(frm.find(":input[name]"), function(idx, cur) {
    var jcur = $(cur);
    if (jcur.is("select")) {
      var opt = jcur.find("option[selected]");
      if (opt != null)
        result[jcur.attr("name")] = opt.val();
    } else if (jcur.is(":checkbox")) {
      if (cur.getAttribute("checked"))
        result[jcur.attr("name")] = "on";
    } else {
      var value = cur.getAttribute("value");
      if (value)
        result[jcur.attr("name")] = value;
    }
  });
  return result;
}

function EXmlHttpRequest(url, parameters, onResponse) {

  var http_request = false;

  var alertContents = function() {
    if (http_request.readyState == 4) {
      if (http_request.status == 200) {
        onResponse(http_request.responseText,http_request.getResponseHeader("Content-Type"));
      }
    }
  };

  this.send = function() {
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
      http_request = new XMLHttpRequest();
      if (http_request.overrideMimeType) {
        http_request.overrideMimeType('text/xml');
      }
    } else if (window.ActiveXObject) { // IE
      try {
        http_request = new ActiveXObject("Msxml2.XMLHTTP");
      } catch (e) {
        try {
          http_request = new ActiveXObject("Microsoft.XMLHTTP");
        } catch (e) {
        }
      }
    }
    if (!http_request)
      return false;

    http_request.open('POST', url, true);
    http_request.onreadystatechange = alertContents;
    http_request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    http_request.setRequestHeader("Content-length", parameters.length);
    http_request.setRequestHeader("Connection", "close");
    http_request.send(parameters);
  }
}

function constructArgs(rootElement) {
  var args = "";
  for (var i = 0; i < rootElement.children.length; i++) {
    var c = rootElement.children[i];
    if (c.getAttribute("id") != null && c.getAttribute("value") != null) {
      if (args.length > 0)
        args += '&';
      args += c.getAttribute("id") + '=' + c.getAttribute("value");

    }
    if (c.getAttribute("id") != null && c.checked) {
      if (args.length > 0)
        args += '&';
      args += c.getAttribute("id") + '=true';
    } else {
      var v = constructArgs(c);
      if (v.length > 0) {
        if (args.length > 0)
          args += '&';
        args += v;
      }
    }
  }

  return args;
}
