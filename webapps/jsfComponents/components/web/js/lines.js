

/**
 * Хелпер для управления блок обновления на стороне браузера
 * @param contentId идентификатор блока, которым надо управлять
 *
 *
 */


function Lines(contentId, updatePeriod, height, width) {

  var bodyElement = $("#"+contentId);
  var closestForm = bodyElement.parents("form");
  var requestUrl = closestForm.attr("action");

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

    var onResponse = function(text) {
      g.draw(text);
      if(updatePeriod>0) {
        checkUpdate();
      }
    };

    var params = serializeForm(closestForm);
    params["eyelineComponentUpdate"] = contentId;
    $.ajaxSetup({cache: false});
    $.post(requestUrl, params, onResponse);

  };

  callUpdate();

}