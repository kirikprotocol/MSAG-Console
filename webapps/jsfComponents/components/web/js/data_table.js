function lookupValueInArray(array, value) {
  for (var idx = 0; idx < array.length; idx++) {
    if (array[idx] == value)
      return idx;
  }
  return -1;
}

function removeValueFromArray(array, idx) {
  var result = new Array(array.length - 1);
  var k = 0;
  for (var i = 0; i < array.length; i++) {
    if (i == idx)
      continue;
    result[k++] = array[i];
  }
  return result;
}

function arrayToJson(arr) {
  var res = "[";
  var first = 1;
  for (var i = 0; i < arr.length; i++) {
    if (first == 0) {
      res += ","
    } else {
      first = 0;
    }
    res += "\"" + arr[i] + "\"";
  }
  return res + "]"
}

function DataTable(tableId, tableOptions) {

  var currentPageNumber = tableOptions.currentPageNumber;
  var currentSortOrder = tableOptions.sortOrder;
  if (!currentSortOrder)
    currentSortOrder = "";
  var currentPageSize = tableOptions.pageSize;
  var allRowsSelected = false;
  var currentMode = "all";

  var tableElement = $("#" + tableId);
  var closestForm = tableElement.parents("form");
  var requestUrl = closestForm.attr("action");
  var bodyElement = tableElement.find("tbody");
  var select = $("#" + tableId + "_select");

  var requestRows = function() {

    var params = serializeForm(closestForm);
    params["eyelineComponentUpdate"] = tableId;
    $.ajaxSetup({cache: false});

    var progress = null;

    var onResponse = function(data, status, resp) {
      if (status != 'success')
        return;

      if (typeof(data) == "object") {
        if (data.type == "progress") {
          progress = data.data;
          progressOverlay.showProgress(data.data + "%");
          window.setTimeout(_sendRequest, 1000);
        } else {
          progress = 100;
          progressOverlay.showError(data.data);
        }
      } else {
        var rowsCount = resp.getResponseHeader("rowsCount");
        progress = 100;
        progressOverlay.hide();
        bodyElement.html(data);
        navbar.setTotal(rowsCount)
      }
    };

    var _updateOverlay = function() {
      if (progress != 100) {
        if (progress != null)
          progress+="%";
        progressOverlay.showProgress(progress);
      }

    };

    var _sendRequest = function() {
      $.post(requestUrl, params, onResponse);
    };

    progress = null;
    window.setTimeout(_updateOverlay,300);
    _sendRequest();
  };

  var _update = function() {
    $("#" + tableId + "_column").val(currentSortOrder);
    $("#" + tableId + "_page").val(currentPageNumber);
    $("#" + tableId + "_pageSize").val(currentPageSize);
    $("#" + tableId + "_showSelected").val(currentMode == "all" ? "false" : "true");
    requestRows();
  };

  this.update = function() {
    _update();
  };

  var progressOverlay = new ProgressOverlay(tableElement);


  var selectionControl;
  if (tableOptions.selectButton) {
    var _selectRow = this.selectRow = function(checked, rowId) {
      var selectObject = eval('(' + select.val() + ')');
      var idx = lookupValueInArray(selectObject, rowId);
      if (allRowsSelected == checked) {
        if (idx < 0)
          return;
        selectObject = removeValueFromArray(selectObject, idx);
      } else if (idx < 0) {
        selectObject[selectObject.length] = rowId;
      }
      select.val(arrayToJson(selectObject));
      if (allRowsSelected)
        navbar.setSelected(navbar.getTotal() - selectObject.length);
      else
        navbar.setSelected(selectObject.length);
    };

    var selectPage = function() {
      var checkboxes = $("[id*='" + tableId + "_rowCheck']");
      var checkedBoxes = checkboxes.filter(":checked");

      var checked = (checkedBoxes.length == checkboxes.length);

      var prefixLen = (tableId + "_row").length;
      $.each(checkboxes, function(idx, el) {
        el.checked = !checked;
        _selectRow(!checked, el.name.substr(prefixLen));
      });
    };

    selectionControl = new SelectionControl(tableOptions.selectButton, {
      onSelectPage : selectPage,
      labels : tableOptions.selectionLabels,
      onSelectAll : function(checked) {
        $("#" + tableId + "_select").val("[]");
        $("#" + tableId + "_selectAll").val(checked);
        var checkboxes = $("[id*='" + tableId + "_rowCheck']");
        $.each(checkboxes, function(idx, el) {
          el.checked = checked;
        });
        navbar.setSelected(checked ? navbar.getTotal() : 0);
        allRowsSelected = checked;
      }
    });
  }


  var toggleButton;
  if (tableOptions.toggleButton) {
    this.expandRow = function(rowId) {
      var headerElement = $("[id='innerDataHeader" + rowId + "']");
      if (headerElement == null)
        return;

      if (headerElement.hasClass('eyeline_inner_data_closed')) {
        headerElement.removeClass('eyeline_inner_data_closed').addClass('eyeline_inner_data_opened');
        $("tr[name='innerData" + rowId + "']").show();
      } else {
        headerElement.removeClass('eyeline_inner_data_opened').addClass('eyeline_inner_data_closed')
        $("tr[name='innerData" + rowId + "']").hide();
      }
    };

    toggleButton = new ToggleButtonControl(tableOptions.toggleButton, {
      onChange : function(opened) {
        if (opened) {
          $("[id*='innerDataHeader" + tableId + "']").removeClass('eyeline_inner_data_closed').addClass('eyeline_inner_data_opened');
          $("tr[name*='innerData" + tableId + "']").show();
        } else {
          $("[id*='innerDataHeader" + tableId + "']").removeClass('eyeline_inner_data_opened').addClass('eyeline_inner_data_closed');
          $("tr[name*='innerData" + tableId + "']").hide();
        }
      }});
  }

  var navbar = new NavBarControl(tableOptions.navbar, {
    total: tableOptions.totalRows,
    pageSize: currentPageSize,
    selectionEnabled : (selectionControl != null),
    allowedPageSize: [10,20,30,40,50],
    pageNumber: currentPageNumber,
    labels : tableOptions.navbarLabels,
    selected: 0,
    onChange : function(pageNumber, pageSize) {
      currentPageNumber = pageNumber;
      currentPageSize = pageSize;
      _update();
    },
    onChangeMode : function(mode) {
      currentMode = mode;
      if (selectionControl)
        selectionControl.changeLock(mode != "all");
      _update();
    }
  });

  var sortableColumns;
  if (tableOptions.columns) {
    sortableColumns = new SortableColumnsControl({
      columnIds: tableOptions.columns,
      sortOrder: currentSortOrder,
      onChange : function(newSort) {
        if (newSort.charAt(0) == '-')
          currentSortOrder = "-" + newSort.substr(tableId.length + 2);
        else
          currentSortOrder = newSort.substr(tableId.length + 1);
        _update();
      }
    });
  }


};

// ::::::::::::::::::::::::::::::::::::::::: ОВЕРЛЕЙ С ПРОГРЕССОМ ::::::::::::::::::::::::::::::::::::::::::::::::::::::

var ProgressOverlay = function(parent) {

  var visible = false;

  var progressElement = $("<table>").addClass("eyeline_overlay");
  $('<tr><td align="center" valign="bottom">' +
      '<div>' +
      '<table style="width:0%"><tr>' +
      '<td><div class="eyeline_loading"></div></td>' +
      '<td><span></span></td>' +
      '</tr></table>' +
      '</div>').appendTo(progressElement);

  var progressContentElement = progressElement.find('span');
  var progressPicElement = progressElement.find('.eyeline_loading');

  progressElement.appendTo(parent);

  var showOverlay = function(value) {
    progressContentElement.text(value ? value : "");

    if (visible)
      return;

    var el = parent[0];
    if (el.offsetParent) {
      var x = el.offsetLeft, y = el.offsetTop, w = el.offsetWidth, h = el.offsetHeight;
      while (el = el.offsetParent) {
        x += el.offsetLeft;
        y += el.offsetTop;
      }
      progressElement.css("left", x + 'px').css("top", y + 'px').css("width", w + 'px').css("height", h + 'px');
    }
    progressElement.show();
    visible = true;
  };

  this.showError = function(value) {
    progressPicElement.hide();
    progressContentElement.removeClass("eyeline_progress_ok").addClass("eyeline_progress_error");
    showOverlay(value);
  };

  this.showProgress = function(value) {
    progressPicElement.show();
    progressContentElement.removeClass("eyeline_progress_error").addClass("eyeline_progress_ok");
    showOverlay(value);
  };

  this.hide = function() {
    if (visible)
      progressElement.hide();
    visible = false;
  }
};

// ::::::::::::::::::::::::::::::::::::::::::::::: ПАНЕЛЬ НАВИГАЦИИ ::::::::::::::::::::::::::::::::::::::::::::::::::::

var NavBarControl = function(id, navbarOptions) {
  // Read options
  var totalSize = navbarOptions.total;
  var pageSize = navbarOptions.pageSize;
  var allowedPageSize = navbarOptions.allowedPageSize;
  var numberOfPages = totalSize / pageSize;
  var pageNumber = Math.min(numberOfPages, navbarOptions.pageNumber);
  var onChange = navbarOptions.onChange;
  var onChangeMode = navbarOptions.onChangeMode;
  var selectedSize = navbarOptions.selected;
  var labels = navbarOptions.labels;
  if (!selectedSize)
    selectedSize = 0;
  var mode = "all";

  var navBarTable = $("<table>").addClass("eyeline_navbar_panel");
  var navBarTableBody = $("<tbody>").appendTo(navBarTable);
  var navBarTableRow = $("<tr>").appendTo(navBarTableBody);

  // Панель навигации ================================================================================================

  var firstButton = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_button eyeline_navbar_first");
  firstButton.click(function() {
    setPage(1, onChange);
  });

  var prevButton = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_button eyeline_navbar_prev");
  prevButton.click(function() {
    setPage(pageNumber - 1, onChange);
  });

  var pageButtons = new Array(9);
  for (var i = 0; i < 9; i++) {
    var pageBut = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_button eyeline_navbar_page");
    pageBut.click(function() {
      setPage(parseInt(this.getAttribute("curPage")), onChange);
    });
    pageButtons[i] = pageBut;
  }

  var nextButton = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_button eyeline_navbar_next");
  nextButton.click(function() {
    setPage(pageNumber + 1, onChange);
  });

  var lastButton = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_button eyeline_navbar_last");
  lastButton.click(function() {
    setPage(numberOfPages, onChange);
  });

  // Панель счетчиков ================================================================================================

  var counters = $("<td>").appendTo(navBarTableRow).addClass("eyeline_navbar_counters");
  counters.append(labels[0] + ": ");
  var total = $("<span>").appendTo(counters).text(totalSize);

  if (navbarOptions.selectionEnabled) {
    counters.append(" | ");
    var selected = $("<span>").appendTo(counters).text(labels[1] + ": " + selectedSize).addClass("eyeline_navbar_selected");
    selected.click(function() {
      if (mode == "all") {
        mode = "selected";
        selected.text(labels[3]);
      } else {
        mode = "all";
        selected.text(labels[1] + ": " + selectedSize);
      }
      if (onChangeMode)
        onChangeMode(mode);
    });
  }
  counters.append(" | " + labels[2] + ": ");
  var onpage = $("<select>").appendTo(counters);
  for (var j = 0; j < allowedPageSize.length; j++) {
    var psize = allowedPageSize[j];
    var opt = $('<option>', { value :  psize}).text(psize);
    if (psize == pageSize)
      opt.attr("selected", "selected");
    onpage.append(opt);
  }
  onpage.change(function() {
    var selectedValue = onpage.children("option:selected")[0].getAttribute("value");
    setPageSize(parseInt(selectedValue), onChange);
  });

  // Вспомогательные функции =========================================================================================

  var setPage = function(pageNumb, onChange) {
    if (pageNumb > numberOfPages)
      pageNumb = numberOfPages;
    if (pageNumb < 1)
      pageNumb = 1;

    firstButton.css("display", pageNumb <= 1 ? "none" : "");
    prevButton.css("display", pageNumb <= 1 ? "none" : "");
    nextButton.css("display", (numberOfPages == 0 || pageNumb == numberOfPages) ? "none" : "");
    lastButton.css("display", (numberOfPages == 0 || pageNumb == numberOfPages) ? "none" : "");

    var firstVisiblePage = Math.max(pageNumb - 4, 1);
    var lastVisiblePage = Math.min(pageNumb + 4, numberOfPages);
    if (lastVisiblePage == firstVisiblePage)
      lastVisiblePage = 0;

    if (pageNumber)
      pageButtons[pageNumber - firstVisiblePage].removeClass("eyeline_navbar_current_page");
    pageButtons[pageNumb - firstVisiblePage].addClass("eyeline_navbar_current_page");

    for (var curPage = firstVisiblePage; curPage <= lastVisiblePage; curPage++)
      pageButtons[curPage - firstVisiblePage].css("display", "").text(curPage).attr("curPage", curPage);

    for (var i = lastVisiblePage + 1; i < firstVisiblePage + 9; i++)
      pageButtons[i - firstVisiblePage].css("display", "none");

    pageNumber = pageNumb;
    if (onChange)
      onChange(pageNumb, pageSize);
  };

  var setPageSize = function(pageS, onChange) {
    var curPos = pageNumber * pageSize;
    numberOfPages = Math.round(totalSize / pageS);
    if (pageS * numberOfPages < totalSize)
      numberOfPages++;
    var pageN = Math.round(curPos / pageS);
    if (pageN != pageNumber)
      setPage(pageN, null);
    pageSize = pageS;
    if (onChange)
      onChange(pageNumber, pageSize);
  };

  this.setSelected = function(cnt) {
    selectedSize = cnt;
    selected.text(labels[1] + ": " + cnt);
  };

  this.setTotal = function(totalRows) {
    totalSize = totalRows;
    total.text(totalRows);
    setPageSize(pageSize, null);
    setPage(pageNumber, null);
  };

  this.getTotal = function() {
    return totalSize;
  };

  this.setTotal(totalSize);

  navBarTable.appendTo("#" + id);
};

// ::::::::::::::::::::::::::::::::::: КНОПКА СОРТИРОВКИ ПО КОЛОНКЕ ТАБЛИЦЫ ::::::::::::::::::::::::::::::::::::::::::::

var SortableColumnsControl = function(columnsOptions) {

  var onchange = columnsOptions.onChange;

  var columns = {};
  for (var i = 0; i < columnsOptions.columnIds.length; i++) {
    var column = $("#" + columnsOptions.columnIds[i]);
    column.addClass("eyeline_data_table_column");
    column.click(function() {
      setSort(this.id);
    });
    columns[i] = column;
  }

  var parseSortOrder = function(sortOrderColumn) {
    var result = {};
    if (sortOrderColumn.length == 0)
      return result;

    if (sortOrderColumn.charAt(0) == '-') {
      result["column"] = sortOrderColumn.substr(1);
      result["asc"] = false;
    } else {
      result["column"] = sortOrderColumn;
      result["asc"] = true;
    }
    return result;
  };

  var setSort = function(columnId) {
    var oldSortColumn = sortOrder["column"];
    var asc = sortOrder["asc"];
    if (oldSortColumn) {
      $("#" + oldSortColumn).removeClass("eyeline_up eyeline_down");
      if (oldSortColumn == columnId)
        asc = !asc;
    }

    $("#" + columnId).addClass(asc ? "eyeline_up" : "eyeline_down");
    sortOrder["column"] = columnId;
    sortOrder["asc"] = asc;

    if (onchange)
      onchange((asc ? "" : "-") + columnId);
  };

  var sortOrder = parseSortOrder(columnsOptions.sortOrder);

  if (sortOrder["column"]) {
    var columnId = sortOrder["column"];
    var asc = sortOrder["asc"];
    $("#" + columnId).addClass(asc ? "eyeline_up" : "eyeline_down");
  }
};

// :::::::::::::::::::::::: КНОПКА УСТАНОВКИ/СНЯТИЯ ВЯДЕЛЕНИЯ ВСЕХ ЗАПИСЕЙ/ЗАПИСЕЙ НА СТРАНИЦЕ :::::::::::::::::::::::::

var SelectionControl = function(id, selectionOptions) {
  var labels = selectionOptions.labels;
  var selectAllHandler = selectionOptions.onSelectAll;
  var selectPageHandler = selectionOptions.onSelectPage;

  var selectTable = $("<table>");
  var selectTableBody = $("<tbody>").appendTo(selectTable);
  var selectTableRow = $("<tr>").appendTo(selectTableBody);
  var td1 = $("<td>").appendTo(selectTableRow);
  var selectPage = $("<div>&nbsp;</div>").appendTo(td1).addClass("select_page_button");
  var td2 = $("<td>").appendTo(selectTableRow);
  var showMenu = $("<div>&nbsp;</div>").appendTo(td2).addClass("select_menu_button");

  var selectMenu = $("<div>").appendTo("#" + id).addClass("select_menu_content");
  var selectAllButton = $("<span>"+labels[0]+"</span>").appendTo(selectMenu);
  selectMenu.append("<br>");
  var selectNothingButton = $("<span>"+labels[1]+"</span>").appendTo(selectMenu);


  var allowCheckAll = true;

  showMenu.click(function() {
    selectMenu.toggle();
  });

  selectAllButton.click(function() {
    selectMenu.hide();
    selectAllHandler(true);
  });

  selectNothingButton.click(function() {
    selectMenu.hide();
    selectAllHandler(false);
  });

  selectPage.click(function() {
    if (allowCheckAll)
      selectPageHandler();
  });

  this.changeLock = function(flag) {
    if (flag)
      this.lock();
    else
      this.unlock();
  };

  this.lock = function() {
    showMenu.hide();
    selectMenu.hide();
    allowCheckAll = false;
  };

  this.unlock = function() {
    selectMenu.hide();
    showMenu.show();
    allowCheckAll = true;
  };

  selectTable.appendTo("#" + id);
};

// ::::::::::::::::::::::::::::::: КНОПКА ПОКАЗА/СКРЫТИЯ ВСЕХ INNER ROWS :::::::::::::::::::::::::::::::::::::::::::::::

var ToggleButtonControl = function(id, toggleOptions) {
  var toggleFunc = toggleOptions.onChange;
  var button = $("<div>").addClass("toggle_button_closed");
  button.click(function() {
    var opened = button.hasClass("toggle_button_opened");
    if (opened)
      button.removeClass("toggle_button_opened").addClass("toggle_button_closed");
    else
      button.removeClass("toggle_button_closed").addClass("toggle_button_opened");
    toggleFunc(!opened);
  });
  button.appendTo("#" + id);
};