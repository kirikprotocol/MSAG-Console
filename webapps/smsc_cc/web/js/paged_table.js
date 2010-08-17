function PagedTable(tableId, requestUrl) {

  var columnElement = document.getElementById(tableId + '_column');
  var pageElement = document.getElementById(tableId + '_page');
  var selectedColumn = document.getElementById(tableId + '_selectedColumn');
  var selectedRow = document.getElementById(tableId + '_selectedRow');
  var pageSizeElement = document.getElementById(tableId + '_pageSize');
  var previousPageSizeElement = document.getElementById(tableId + '_previousPageSize');
  var bodyElement = document.getElementById(tableId);
  var filterElement = document.getElementById(tableId + '_filter');

  var constructArgs = function (rootElement) {
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
  };

  this.updateTable = function() {
    var column = columnElement.value;
    var page = pageElement.value;
    var pageSize = pageSizeElement.value;
    var previousPageSize = previousPageSizeElement.value;
    var onResponse = function(text) {
      if (column != columnElement.value) return;
      bodyElement.innerHTML = text;
    };
    var params = '?eyelineComponentUpdate=' + tableId + '&' + tableId + '_column=' + column + '&' + tableId + '_page=' + page + '&' + tableId + '_pageSize=' + pageSize + '&' + tableId + '_previousPageSize=' + previousPageSize;
    if (filterElement != null)
      params += '&' + constructArgs(filterElement);

    params += '&' + constructArgs(bodyElement);

    new EXmlHttpRequest(requestUrl, params, onResponse).send('');
  };

  this.setSortOrder = function(column) {
    columnElement.value=column;
    this.updateTable();
  };

  this.setPage = function(page) {
    pageElement.value = page;
    this.updateTable();
  };

  this.setPageSize = function (pageSize) {
    pageSizeElement.value = pageSize;
    this.updateTable();
    previousPageSizeElement.value = pageSizeElement.value;
  };

  this.selectAll = function () {
    for (var i = 0; i < previousPageSizeElement.value; i++) {
      var c = document.getElementById(tableId + '_rowCheck' + i);
      if (c == null) break;
      c.checked = !c.checked;
    }
  };

  this.selectElement = function(column, row) {
    selectedColumn.value = column;
    selectedRow.value = row;
    for (var k = 0; k < document.forms.length; k++) {
      if (document.forms[k].elements[tableId + '_selectedColumn'] != null)
        return document.forms[k].submit();
    }
  };

  this.autoUpdateTable = function(timeout) {
    this.updateTable();
    window.setTimeout(this.autoUpdateTable, timeout);
  }


}