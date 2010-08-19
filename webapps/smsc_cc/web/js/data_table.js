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
        var value = el.getAttribute("value");
        if (value == null && el.checked)
          value = "true";

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
    if (updateUsingSubmit)
      return closestForm.submit();

    var column = columnElement.value;
    var page = pageElement.value;
    var pageSize = pageSizeElement.value;
    var previousPageSize = previousPageSizeElement.value;
    var onResponse = function(text) {
      if (column != columnElement.value) return;
      bodyElement.innerHTML = text;
    };

    var params = 'eyelineComponentUpdate=' + tableId + '&' + prepareFormParameters(columnElement.id);

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
    var startPos = previousPageSizeElement.value * pageElement.value;
    var endPos = startPos + previousPageSizeElement.value;
    for (var i = startPos; i < endPos; i++) {
      var c = document.getElementById(tableId + '_rowCheck' + i);
      if (c == null) break;
      c.checked = !c.checked;
    }
  };

  this.showRowInnerData = function(rowId) {
    var element = document.getElementById("innerData" + rowId);
    var headerElement = document.getElementById("innerDataHeader" + rowId);
    if (element.style.display == "none") {
      element.style.display="";
      headerElement.className = 'inner_data_opened';
    } else {
      element.style.display="none";
      headerElement.className = 'inner_data_closed';
    }
  };

  /**
   * Автоматически обновляет содержимое таблицы с заданным интервалом
   * @param timeout интервал обновления в миллисекундах
   */
  this.autoUpdateTable = function(timeout) {
    this.updateTable();
    window.setTimeout(this.autoUpdateTable, timeout);
  }


}