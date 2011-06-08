/**
 * Класс TextColumn
 * @param columnId
 */
function TextColumn(columnId, allowEditAfterAdd) {
  this.columnId = columnId;

  //--------------------------------------------------------------------------------------------------------------------
  var createInput = function(id, value) {
    var input = document.createElement("input");
    input.name = id;
    input.id = id;
    input.setAttribute("id", id);
    input.setAttribute("value", value);
    input.value = value;
    input.type="text";
    input.className="txtW";
    return input;
  };

  //--------------------------------------------------------------------------------------------------------------------
  var getValueElementId = function(tableId, rowNum) {
    return tableId + "_" + rowNum + "_" + columnId;
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.isAllowedToCreateColumnElement = function(tableId, value) {
    return true;
  }

  //--------------------------------------------------------------------------------------------------------------------
  this.createColumnElement = function (tableId, newRow, newCount, value) {
    var id = getValueElementId(tableId, newCount);
    var newCell = newRow.insertCell(newRow.cells.length);
    if (!allowEditAfterAdd) {
      var input = document.createElement("input");
      input.name = id;
      input.id = id;
      input.value = value;
      input.type="hidden";
      newCell.appendChild(input);
      var label = document.createElement("label");
      label.textContent = value;
      newCell.appendChild(label);
    } else {
      newCell.appendChild(createInput(id, value));
    }
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.createLastColumnElement = function (tableId, newRow) {
    var id = tableId + "_newcell_" + columnId;
    newRow.insertCell(newRow.cells.length).appendChild(createInput(id, ''));
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.removeColumnElement = function(tableId, rowNum) {
    // do nothing
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.getLastRowValue = function(tableId) {
    var id = tableId + "_newcell_" + columnId;
    return document.getElementById(id).value;
  }

  this.clearLastRowValue = function(tableId) {
    var id = tableId + "_newcell_" + columnId;
    document.getElementById(id).value = "";
  }
}

/**
 * Класс SelectColumn
 * @param columnId
 * @param values
 */
function SelectColumn(columnId, values, allowEditAfterAdd, uniqueValues) {
  this.columnId = columnId;

  //--------------------------------------------------------------------------------------------------------------------
  var getValueElementId = function(tableId, rowNum) {
    return tableId + "_" + rowNum + "_" + columnId;
  };

  //--------------------------------------------------------------------------------------------------------------------
  var createSelect = function(id, value, values) {
    var select = document.createElement("select");
    select.id = id;
    select.name = id;
    select.className="selectW";
    for (var i = 0; i < values.length; i++)
      select.options[i] = new Option(values[i], values[i], i == 0, value == values[i]);
    return select;
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.isAllowedToCreateColumnElement = function(tableId, value) {
    if (uniqueValues) {
      var lastColumnElement = document.getElementById(tableId + "_newcell_" + columnId);
      return lastColumnElement.options.length > 0;
    }
    return true;
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.createColumnElement = function(tableId, newRow, newCount, value) {
    var id = getValueElementId(tableId, newCount);
    var newCell = newRow.insertCell(newRow.cells.length);
    if (!allowEditAfterAdd) {
      var input = document.createElement("input");
      input.name = id;
      input.id = id;
      input.value = value;
      input.type="hidden";
      newCell.appendChild(input);
      var label = document.createElement("label");
      label.textContent = value;
      newCell.appendChild(label);
    } else {
      newCell.appendChild(createSelect(id, value, values));
    }

    if (uniqueValues) {
      var lastColumnElement = document.getElementById(tableId + "_newcell_" + columnId);
      var options = lastColumnElement.options;
      for (var i=0; i<options.length; i++) {
        if (options[i].value == value) {
          options[i] = null;
          break;
        }
      }
    }

  };

  this.clearLastRowValue = function(tableId) {
    var id = tableId + "_newcell_" + columnId;
    document.getElementById(id).selectedIndex = 0;
  } ;
  
  //--------------------------------------------------------------------------------------------------------------------
  this.removeColumnElement = function(tableId, rowNum) {
    if (uniqueValues) {
      var value = document.getElementById(getValueElementId(tableId, rowNum)).value;
      var lastColumnElement = document.getElementById(tableId + "_newcell_" + columnId);
      var options = lastColumnElement.options;
      options[options.length] = new Option(value, value, false, false);
    }
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.createLastColumnElement = function (tableId, newRow) {
    var id = tableId + "_newcell_" + columnId;
    newRow.insertCell(newRow.cells.length).appendChild(createSelect(id, '', values));
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.getLastRowValue = function(tableId) {
    var id = tableId + "_newcell_" + columnId;
    return document.getElementById(id).value;
  }
}


/**
 * Класс DynamicTable
 * @param tableId
 * @param columns
 */
function DynamicTable(contextPath, tableId, columns) {

  var tableElem = document.getElementById(tableId);
  var tableInstance = this;

  //--------------------------------------------------------------------------------------------------------------------
  this.delRow = function (rownum) {

    for (var j=0; j< columns.length; j++)
      columns[j].removeColumnElement(tableId, rownum);

    var rowId = tableId + "_row_" + rownum;
    var rowElem = tableElem.rows[rowId];
    tableElem.deleteRow(rowElem.rowIndex);
    var count = tableElem.rows.length;

    for (var i = 0; i < count; i++) {
      var row = tableElem.rows[i];
      row.setAttribute("class", "row" + ((i + 1) & 1));
    }
  };

  //--------------------------------------------------------------------------------------------------------------------
  this.addRow = function(values) {

    for (var j=0; j<columns.length; j++) {
      if (!columns[j].isAllowedToCreateColumnElement(tableId, values[j]))
        return;
    }

    var newCount = tableElem.rows.length;
    // Create new row
    var newRow = tableElem.insertRow(newCount - 1);
    newRow.setAttribute("class", "row" + (newCount & 1));
    newRow.id = tableId + "_row_" + newCount;

    // Fill row
    for (var i = 0; i < columns.length; i++) {
      var value = values[i];
      columns[i].createColumnElement(tableId, newRow, newCount, value);
    }

    var image = document.createElement("div");
    image.setAttribute("class", "eyeline_delbutton");
    image.innerHTML='&nbsp';
    image.onclick = function(e) {
      tableInstance.delRow(newCount);
    };
    newRow.insertCell(newRow.cells.length).appendChild(image);

    var lastRow = tableElem.rows[tableId + "_newrow"];
    lastRow.setAttribute("class", "row" + ((newCount + 1) & 1))
  };


  //--------------------------------------------------------------------------------------------------------------------
  var init = function() {
    var newCount = tableElem.rows.length;
    // Create new row
    var newRow = tableElem.insertRow(newCount);
    newRow.setAttribute("class", "row" + (newCount & 1));
    newRow.id = tableId + "_newrow";

    // Fill row
    for (var i = 0; i < columns.length; i++)
      columns[i].createLastColumnElement(tableId, newRow);

    var image = document.createElement("div");
    image.setAttribute("class", "eyeline_addbutton");
    image.innerHTML='&nbsp';
    image.onclick = function(e) {
      var values = new Array();
      for (var i = 0; i < columns.length; i++) {
        values[i] = columns[i].getLastRowValue(tableId);
        columns[i].clearLastRowValue(tableId)
      }

      tableInstance.addRow(values);
    };
    newRow.insertCell(newRow.cells.length).appendChild(image);
  };

  init();

}