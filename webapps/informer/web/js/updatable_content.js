/**
 * Хелпер для управления блок обновления на стороне браузера
 * @param contentId идентификатор блока, которым надо управлять
 *
 *
 */

function UpdateContent(contentId, updatePeriod) {

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
    for (var i = 0; i < rootElement.children.length; i++) {
      var el = rootElement.children[i];

      var pname = el.getAttribute("id");
      if (pname == null)
        pname = el.getAttribute("name");
      if (pname != null) {
        if (el.tagName == "INPUT" && el.getAttribute("type") == "checkbox") {
          if (el.getAttribute("checked") != null)
            args += pname + '=true';
          continue;
        }
        var value = el.getAttribute("value");
        if (value == null) {
          if (el.checked)
            value = "true";
          else if (el.tagName == "SELECT") {

            for (var op = 0; op < el.options.length; op++) {
              if (el.options[op].getAttribute("selected") != null) {
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
    return args;
  };

  var checkUpdate = function() {
    var enabledEl = document.getElementById("enabled" + contentId);
    if (enabledEl != null && enabledEl.getAttribute("enabledVal") == "true")
      window.setTimeout(callUpdate, updatePeriod * 1000)
  };

  /**
   * Обновляет содержимое
   */
  var callUpdate = function () {
    var onResponse = function(text) {
      bodyElement.innerHTML = text;

      var scripts = bodyElement.getElementsByTagName("script");
      if (scripts) {
        for (var i = 0; i < scripts.length; i++) {
          eval(scripts[i].text);
        }
      }
      checkUpdate();
    };

    var params = 'eyelineComponentUpdate=' + contentId + '&' + prepareFormParameters();
    new EXmlHttpRequest(requestUrl, params, onResponse).send();
  };

  checkUpdate();

}