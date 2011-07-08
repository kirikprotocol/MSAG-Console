/**
 * Хелпер для управления data table на стороне браузера
 * @param tableId идентификатор таблицы, которой надо управлять
 * @param updateUsingSubmit флаг, если он true, то обновление содержимого таблицы происходит через сабмит формы. Иначе - через Ajax
 */
function DataTable(tableId, updateUsingSubmit, _progress, _titleError) {

  var columnElement = document.getElementById(tableId + '_column');
  var pageElement = document.getElementById(tableId + '_page');
  var pageSizeElement = document.getElementById(tableId + '_pageSize');
  var selectedOnly = document.getElementById(tableId+"_showSelected");
  var previousPageSizeElement = document.getElementById(tableId + '_previousPageSize');
  var bodyElement = document.getElementById(tableId);
  var progressElement = document.getElementById(tableId+"_progress");
  var progressContentElement = document.getElementById(tableId+"_progress_content");
  var checked = false;
  var titleError = _titleError;

  /**
   * Ищет форму, которая содержит элемент с указанным идентификатором
   * @param elementId идентификатор элемента
   */
  var getClosestForm = function(elementId) {
    for (var k = 0; k < document.forms.length; k++) {
      var form = document.forms[k];
      if (form.elements[elementId] != null)
        return form;
    }
    return null;
  };

  var closestForm = getClosestForm(columnElement.id);
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

            for (var op=0; op < el.options.length; op++) {
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



  /**
   * Обновляет содержимое таблицы
   */

  var emptyProgress = function() {
    setProgressFunction("");
  };

  var updateFunction = this.updateTable = function() {

    checked = false;
    if (updateUsingSubmit)
      return closestForm.submit();

    emptyProgress();
    sendRequest();

  };



  var sendRequest = function() {

    var onResponse = function(text, contentType) {
      if(contentType != null) {
        var i = contentType.indexOf(";");
        if(i != null) {
          var t = contentType.substring(0, i);
          if(t == "application/json") {
            var rObject =  eval( '('+text+')' );
            if(rObject.type == "progress") {
              setProgressFunction(rObject.data+"%");
              window.setTimeout(sendRequest, 1000);
            }else {
              bodyElement.innerHTML = getError(rObject.data, titleError);
              hideProgress();
            }
            return;
          }
        }
      }
      hideProgress();
      bodyElement.innerHTML = text;
    };

    var params = 'eyelineComponentUpdate=' + tableId + '&' + prepareFormParameters();
    new EXmlHttpRequest(requestUrl, params, onResponse).send();
  };

  /**
   * Устанавливает новый порядок сортировки в таблице и обновляет её
   * @param column порядок сортировки
   */
  this.setSortOrder = function(column) {
    columnElement.value = column;
    this.updateTable();
  };

  /**
   * Устанавливает номер страницы для отображения и обновляет таблицу
   * @param page номер страницы
   */
  this.setPage = function(page) {
    pageElement.value = page;
    this.updateTable();
  };

  this.onlySelected = function() {
    var selected = selectedOnly.value == "true";
    selectedOnly.value = !selected;
    this.updateTable();
  };

  /**
   * Устанавливает новый размер страницы и обновляет таблицу
   * @param pageSize размер страницы
   */
  this.setPageSize = function (pageSize) {
    var startIndex = pageElement.value * pageSizeElement.value;
    pageSizeElement.value = pageSize;
    this.updateTable();
    pageElement.value = Math.floor(startIndex / pageSize);
    previousPageSizeElement.value = pageSizeElement.value;
  };

  /**
   * Инвертирует выделение всех строк текущей страницы
   */
  this.selectAll = function () {
    checked = !checked;
    var endPos = previousPageSizeElement.value;
    var inputs = document.getElementsByTagName("input");
    var prefix = tableId + '_rowCheck';
    for (var i = 0; i < inputs.length; i++) {
      var c = inputs[i];
      if (c.id != null && c.id.indexOf(prefix) == 0)
        c.checked = checked;
    }
  };

  var _expandRow = function(rowId, expand) {
    var headerElement = document.getElementById("innerDataHeader" + rowId);
    if (headerElement == null)
      return;

    headerElement.className = (expand) ? 'inner_data_opened' : 'inner_data_closed';

    var elements = document.getElementsByTagName('tr');
    for (var j = 0; j < elements.length; j++) {
      var element = elements[j];
      var name = element.getAttribute('name');
      if (name != null && name == ('innerData' + rowId))
        element.style.display = expand ? "" : "none";
    }
  };

  this.expandRow = function(rowId) {
    var headerElement = document.getElementById("innerDataHeader" + rowId);
    if (headerElement == null)
      return;

    _expandRow(rowId, headerElement.className == 'inner_data_closed');
  };

  this.expandAll = function() {
    var expandElement = document.getElementById(tableId + "_expand");
    var tableElement = document.getElementById(tableId + "_table");
    if (expandElement == null || tableElement == null)
      return;

    var expand = expandElement.className == 'inner_data_closed';
    var rows = tableElement.rows;

    for (var i = 0; i < rows.length; i++) {
      var row = rows[i];
      if (row.id != null)
        _expandRow(row.id, expand);
    }

    expandElement.className = expand ? 'inner_data_opened' : 'inner_data_closed';
  };

  var setProgressFunction = this.setProgress = function(value) {
    var el = bodyElement;
    if (el.offsetParent) {
      var x=0;
      var y=0;
      var w = el.offsetWidth+'px';
      var h = el.offsetHeight+'px';
      do {
        x+=el.offsetLeft;
        y+=el.offsetTop;
      }
      while (el = el.offsetParent);

      progressElement.style.left=x+'px';
      progressElement.style.top= y+'px';
      progressElement.style.width = w;
      progressElement.style.height = h;
      progressContentElement.innerHTML=getProgressBar(value);
    }
  };

  var hideProgress = function() {
    progressElement.style.top = '-500px';
    progressElement.style.width = '1px';
    progressElement.style.height = '1px';
  };

}

function changeSelectAll(tableId, checked) {
  if(document.getElementById(tableId+"_showSelected").value == "true") {
    return;
  }
  var selectAll = document.getElementById(tableId+"_selectAll");

  var inputs = document.getElementsByTagName("input");
  var prefix = tableId + '_rowCheck';
  for (i = 0; i < inputs.length; i++) {
    c = inputs[i];
    if (c.id != null && c.id.indexOf(prefix) == 0) {
      c.checked = checked;
    }
  }

  selectAll.value = checked;
  document.getElementById(tableId+"_select").value = "[]";

  var select = document.getElementById(tableId+"_select");
  var selectObject = !(/[^,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]/.test(
      select.value.replace(/"(\\.|[^"\\])*"/g, ''))) &&
      eval('(' + select.value + ')');
  updateSelectCount(tableId, selectObject);
  if(!checked) {
    document.getElementById(tableId+"_check").className = 'select_page';
  }

}

function isAllChecked(tableId, checked) {
  var inputs = document.getElementsByTagName("input");
  var prefix = tableId + '_rowCheck';
  for (var i = 0; i < inputs.length; i++) {
    var c = inputs[i];
    if (c.id != null && c.id.indexOf(prefix) == 0) {
      if(c.checked == checked) {
        continue
      }
      return false;
    }
  }
  return true;
}

function changeSelectAllPage(el, tableId) {
  if(document.getElementById(tableId+"_showSelected").value == "true") {
    return;
  }
  var checked = el.getAttribute("selectpage") != "true";
  var inputs = document.getElementsByTagName("input");
  var prefix = tableId + '_rowCheck';
  var allChecked = isAllChecked(tableId, checked);
  if(allChecked) {
    checked = !checked;
  }
  for (var i = 0; i < inputs.length; i++) {
    var c = inputs[i];
    if (c.id != null && c.id.indexOf(prefix) == 0) {
      c.checked = checked;
      c.onclick();
    }
  }
  el.setAttribute("selectpage", checked);
  el.className=(checked ? 'select_page_checked' : 'select_page');
}


function changeSelect(checked, rowId, tableId) {
  var increment = checked ? 1 : -1;
  if(document.getElementById(tableId+"_selectAll").value == "true") {
    checked = !checked;
  }
  var select = document.getElementById(tableId+"_select");
  var selectObject = !(/[^,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]/.test(
      select.value.replace(/"(\\.|[^"\\])*"/g, ''))) &&
      eval('(' + select.value + ')');
  if(checked) {
    for(var i1=0; i1<selectObject.length; i1++) {
      if(selectObject[i1] == rowId) {
        return;
      }
    }
    selectObject[selectObject.length] = rowId;
  }else {
    var k = -1;
    for(var i=0; i<selectObject.length; i++) {
      if(selectObject[i] == rowId) {
        k = i;
        break;
      }
    }
    if(k != -1) {
      selectObject.splice(k,1);
    }
  }
  select.value = arrayToJson(selectObject);
  updateSelectCount(tableId, selectObject);
  if(!checked) {
    document.getElementById(tableId+"_check").className = 'select_page';
  }
}

function arrayToJson(arr) {
  var res = "[";
  var first = 1;
  for(var i=0; i<arr.length; i++) {
    if(first == 0) {
      res+=","
    }else {
      first = 0;
    }
    res+="\""+arr[i]+"\"";
  }
  return res+"]"
}


function updateSelectCount(tableId, selectObject) {
  var selectAll = document.getElementById(tableId+"_selectAll").value == "true";

  var count = document.getElementById(tableId+"_selectedCount");
  if(selectAll) {
    count.innerHTML = parseInt(document.getElementById(tableId+'_totalCount').innerHTML) - selectObject.length;
  }else {
    count.innerHTML = selectObject.length;
  }
}


function getProgressBar(value) {
  return value;
}



function getError(error, errorTitle) {
  return "<table class=\"x73\" cellpadding=\"0\" cellspacing=\"0\" border=\"0\" summary=\"\">\n" +
      "      <tbody>\n" +
      "      <tr>\n" +
      "        <td>\n" +
      "          <div class=\"xdj\">\n" +
      "            <div>\n" +
      "              <h1 class=\"x72\">\n" +
      "                 <div class=\"x71\">"+errorTitle+"</div" +
      "              </h1>\n" +
      "            </div>\n" +
      "            <div class=\"xap\">\n" +
      error +
      "            </div>\n" +
      "          </div>\n" +
      "        </td>\n" +
      "      </tr>\n" +
      "      </tbody>\n" +
      "    </table>";
}












