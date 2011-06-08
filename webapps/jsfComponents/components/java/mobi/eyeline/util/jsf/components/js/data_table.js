/**
 * Хелпер для управления data table на стороне браузера
 * @param tableId идентификатор таблицы, которой надо управлять
 * @param updateUsingSubmit флаг, если он true, то обновление содержимого таблицы происходит через сабмит формы. Иначе - через Ajax
 */
function DataTable(tableId, updateUsingSubmit) {

  var columnElement = document.getElementById(tableId + '_column');
  var pageElement = document.getElementById(tableId + '_page');
  var pageSizeElement = document.getElementById(tableId + '_pageSize');
  var previousPageSizeElement = document.getElementById(tableId + '_previousPageSize');
  var bodyElement = document.getElementById(tableId);
  var overlay = document.getElementById(tableId+"_overlay");
  var checked = false;

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

  this.updateTable = function() {
    this.setOverlay();
    checked = false;    
    if (updateUsingSubmit)
      return closestForm.submit();

    var column = columnElement.value;
    var page = pageElement.value;
    var pageSize = pageSizeElement.value;
    var previousPageSize = previousPageSizeElement.value;
    var onResponse = function(text) {
      hideOverlay();
      if (column != columnElement.value) return;
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

    headerElement.className = (expand) ? 'eyeline_inner_data_opened' : 'eyeline_inner_data_closed';

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

    _expandRow(rowId, headerElement.className == 'eyeline_inner_data_closed');
  };

  this.expandAll = function() {
    var expandElement = document.getElementById(tableId + "_expand");
    var tableElement = document.getElementById(tableId + "_table");
    if (expandElement == null || tableElement == null)
      return;

    var expand = expandElement.className == 'eyeline_inner_data_closed';
    var rows = tableElement.rows;

    for (var i = 0; i < rows.length; i++) {
      var row = rows[i];
      if (row.id != null)
        _expandRow(row.id, expand);
    }

    expandElement.className = expand ? 'eyeline_inner_data_opened' : 'eyeline_inner_data_closed';
  };

  this.setOverlay = function() {
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

     overlay.style.left=x+'px';
     overlay.style.top= y+'px';
     overlay.style.width = w;
     overlay.style.height = h;     
   }
  };

  var hideOverlay = function() {
    overlay.style.top = '-500px';
    overlay.style.width = '1px';
    overlay.style.height = '1px';
  }

}










