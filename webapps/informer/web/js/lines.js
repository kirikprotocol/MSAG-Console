/**
 * Хелпер для управления блок обновления на стороне браузера
 * @param contentId идентификатор блока, которым надо управлять
 *
 *
 */


function Lines(contentId, updatePeriod, height, width) {

  var bodyElement = document.getElementById(contentId);

  /**
   * Ищет форму, которая содержит элемент с указанным идентификатором
   * @param elementId идентификатор элемента
   */
  var getClosestForm = function(elementId) {
    var cand = bodyElement.parentNode;
    while (cand != null) {
      for (var k = 0; k < document.forms.length; k++) {
        var form = document.forms[k];
        if (form == cand)
          return form;
      }
      cand = cand.parentNode;
    }
    return null;
  };

  var closestForm = getClosestForm(contentId);
  var requestUrl = closestForm.getAttribute("action");

  /**
   * Обходит все элементы формы и строит список параметров для сабмита
   */
  var prepareFormParameters = function() {
    return constructArgs(closestForm);
  };

  /**
   * Обходит все элементы внутри rootElement и строит строку для сабмита
   * @param rootElement корневой элемент
   */
  var constructArgs = function (rootElement) {
    var args = "";
    if(rootElement.children != null) {
      for (var i = 0; i < rootElement.children.length; i++) {
        var el = rootElement.children[i];

        var pname = el.getAttribute("id");
        if (pname == null || pname.length == 0)
          pname = el.getAttribute("name");
        if (pname == null || pname.length == 0)
          pname = el.id;
        if (pname == null || pname.length == 0)
          pname = el.name;

        if (pname != null && pname.length != 0) {

          if (el.tagName == "INPUT" && el.getAttribute("type") == "checkbox") {
            if (el.getAttribute("checked") != null && (el.getAttribute("checked") || el.getAttribute("checked") == "checked"))
              args += pname + '=true';

            continue;
          }
          var value = el.getAttribute("value");
          if (value == null) {
            if (el.checked)
              value = "true";
            else if (el.tagName == "SELECT") {

              for (var op = 0; op < el.options.length; op++) {
                if (el.options[op].getAttribute("selected") != null && el.options[op].getAttribute("selected") != '') {
                  value = el.options[op].value;
                  break;
                }
              }
            }
          }

          if (value != null) {
            if (args.length > 0)
              args += '&';
            args += pname + '=' + value;
            continue;
          }
        }

        var v = constructArgs(el);
        if (v.length > 0) {
          if (args.length > 0)
            args += '&';
          args += v;
        }
      }
    }
    return args;
  };



  var checkUpdate = function() {
    window.setTimeout(callUpdate, updatePeriod * 1000)
  };

  function Graph() {
    var r = Raphael(contentId);

    this.draw = function(response) {
      if(response == null) {
        return;
      }

      var responseObject = !(/[^,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]/.test(
          response.replace(/"(\\.|[^"\\])*"/g, ''))) &&
          eval('(' + response + ')');
      var labels = responseObject.labels;
      var first = true;
      r.clear();
      var lines = responseObject.lines;
      for(var i=0; i<lines.length; i++) {

        var x = [];
        var y = [];
        var j = 0;
        for(var k in lines[i].values) {
          x[j] = k;
          y[j] = lines[i].values[j];
          j++;
        }

        var color = lines[i].color;
        var _shade =  lines[i].shade;

        var options = {nostroke: false, shade: _shade, colors:[color], smooth: false, axisxstep: labels.length-1};
        if(first) {
          options.axis = "0 0 1 1";
        }else {
          options.axis = "0 0 0 0";
        }
        var ls = r.g.linechart(20,20,width-40,height-40, [x], [y], options);

        if(first) {
          for(var t=0; t < ls.axis[0].text.items.length; t++) {
            var old = ls.axis[0].text.items[t].attr('text');
            var newT = labels[t];
            ls.axis[0].text.items[t].attr({'text': newT});
          }
          first = false;
        }
      }

    }
  }

  var g = new Graph();

  var callUpdate = function () {

    var onResponse = function(text,contentType) {
      g.draw(text);
      if(updatePeriod>0) {
        checkUpdate();
      }
    };

    var params = 'eyelineComponentUpdate=' + contentId + '&' + prepareFormParameters();
    new EXmlHttpRequest(requestUrl, params, onResponse).send();
  };

  callUpdate();

}