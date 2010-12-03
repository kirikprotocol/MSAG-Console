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
    setElementOpacity(bodyElement,0.3);
    checked = false;    
    if (updateUsingSubmit)
      return closestForm.submit();

    var column = columnElement.value;
    var page = pageElement.value;
    var pageSize = pageSizeElement.value;
    var previousPageSize = previousPageSizeElement.value;
    var onResponse = function(text) {
      setElementOpacity(bodyElement,1.0);
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


}


function setElementOpacity(elem, nOpacity) {
  var opacityProp = getOpacityProperty();

  if (!elem || !opacityProp) return; // Если не существует элемент с указанным id или браузер не поддерживает ни один из известных функции способов управления прозрачностью

  if (opacityProp=="filter")  // Internet Exploder 5.5+
  {
    nOpacity *= 100;

    // Если уже установлена прозрачность, то меняем её через коллекцию filters, иначе добавляем прозрачность через style.filter
    var oAlpha = elem.filters['DXImageTransform.Microsoft.alpha'] || elem.filters.alpha;
    if (oAlpha) oAlpha.opacity = nOpacity;
    else elem.style.filter += "progid:DXImageTransform.Microsoft.Alpha(opacity="+nOpacity+")"; // Для того чтобы не затереть другие фильтры используем "+="
  }
  else // Другие браузеры
    elem.style[opacityProp] = nOpacity;
}

function getOpacityProperty() {
    if (typeof document.body.style.opacity == 'string') // CSS3 compliant (Moz 1.7+, Safari 1.2+, Opera 9)
      return 'opacity';
    else if (typeof document.body.style.MozOpacity == 'string') // Mozilla 1.6 и младше, Firefox 0.8
      return 'MozOpacity';
    else if (typeof document.body.style.KhtmlOpacity == 'string') // Konqueror 3.1, Safari 1.1
      return 'KhtmlOpacity';
    else if (document.body.filters && navigator.appVersion.match(/MSIE ([\d.]+);/)[1]>=5.5) // Internet Exploder 5.5+
      return 'filter';

    return false; //нет прозрачности
}




