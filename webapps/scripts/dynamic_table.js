// This scripts generate properties table

function resetValidationChanges(elem) {
  try {
    elem.style.borderColor = "#C8BE9E";
    elem.style.color = "#161E49";
  } catch (ex) {
  }
}

function getColumnBaseElement(tableId, columnId) {
  return document.getElementById(tableId + "_newcell_" + columnId);
}

function createInput(id, value, validation, readOnly) {
  var input = document.createElement("input");
  input.name = id;
  input.id = id;
  input.value = value;
  input.className = "txtW";
  input.validation = validation;
  if (readOnly) // not allow edit after add
    input.readOnly = true;
  return input;
}

function createImage(src) {
  var image = document.createElement("img");
  image.src = src;
  return image;
}

function createSelect(id, value, values, validation, disabled) {
  var select = document.createElement("select");
  select.id = id;
  select.name = id;
  select.className="selectW";
  select.validation = validation;
  select.disabled = disabled;
  for(var i = 0; i < values.length; i++)
    select.options[i] = new Option(values[i], i, false, value == i);
  return select;
}

function validateColumnElement(element) {
  return validateField(element);
}


// Text column =========================================================================================================


function textColumn(columnId, validation, allowEditAfterAdd) {
  this.columnId = columnId;
  this.validation = validation;
  this.allowEditAfterAdd = allowEditAfterAdd;

  this.validateBaseElement = validateTextColumnBaseElement;
  this.createColumnElement = createTextColumnElement;
}

function validateTextColumnBaseElement(tableId) {
  if (this.validation != null) {
    var columnBaseElement = getColumnBaseElement(tableId, this.columnId);
    columnBaseElement.validation = this.validation;
    var result = validateColumnElement(columnBaseElement);
    columnBaseElement.validation=null;
    return result;
  }
  return true;
}

function createTextColumnElement(tableId, newRow, newCount) {
  var id = tableId + "_cell_" + this.columnId + newCount;
  var value = getColumnBaseElement(tableId, this.columnId).value;
  getColumnBaseElement(tableId, this.columnId).value = "";
  newRow.insertCell(newRow.cells.length).appendChild(createInput(id, value, this.validation, !this.allowEditAfterAdd));
}


// Select column =======================================================================================================


function selectColumn(columnId, validation, values, allowEditAfterAdd) {
  this.columnId = columnId;
  this.validation = validation;
  this.values = values;
  this.allowEditAfterAdd = allowEditAfterAdd;

  this.validateBaseElement = validateSelectColumnBaseElement;
  this.createColumnElement = createSelectColumnElement;
}

function validateSelectColumnBaseElement(tableId) {
  if (this.validation != null) {
    var columnBaseElement = getColumnBaseElement(tableId, this.columnId);
    columnBaseElement.validation = this.validation;
    var result = validateColumnElement(columnBaseElement);
    columnBaseElement.validation=null;
    return result;
  }
  return true;
}

function createSelectColumnElement(tableId, newRow, newCount) {
  var id = tableId + "_cell_" + this.columnId + newCount;
  var value = getColumnBaseElement(tableId, this.columnId).value;
  newRow.insertCell(newRow.cells.length).appendChild(createSelect(id, value, this.values, this.validation, !this.allowEditAfterAdd));
}


// Row control button column ===========================================================================================


function rowControlButtonColumn(columnId, allowRemoveAddedRows) {
  this.columnId = columnId;
  this.allowRemoveAddedRows = allowRemoveAddedRows;

  this.validateBaseElement = validateRowControlButtonColumnBaseElement;
  this.createColumnElement = createRowControlButtonColumnElement;
}

function validateRowControlButtonColumnBaseElement(tableId) {
  return true;
}

function createRowControlButtonColumnElement(tableId, newRow, newCount) {
  if (!this.allowRemoveAddedRows)
    return document.createElement("th");
  var image = createImage("/images/but_del.gif");
  image.setAttribute('rownum', newCount);
  image.setAttribute('tableid', tableId);
  image.attachEvent("onclick", removeRow);
  newRow.insertCell(newRow.cells.length).appendChild(image);
}

function removeRow() {
  delRow(event.srcElement.attributes.tableid.nodeValue, event.srcElement.attributes.rownum.nodeValue);
}

function delRow(tableId, rownum) {
  var tableElem = document.getElementById(tableId);
  var rowId = "row_" + rownum ;
  var rowElem = tableElem.rows(rowId);
  tableElem.deleteRow(rowElem.rowIndex);
}

function addRow(tableId, countElementId, columns) {
  var tableElem = document.getElementById(tableId);
  var newCount = parseInt(document.getElementById(countElementId).value);
  // Create new row
  var newRow = tableElem.insertRow(tableElem.rows.length - 1);
  newRow.className = "row" + (tableElem.rows.length & 1);
  newRow.id = "row_" + newCount;
  // Check inserted data
  for (var i = 0; i < columns.length; i++)
    if (!columns[i].validateBaseElement(tableId))
      return;
  // Fill row
  for (var i = 0; i < columns.length; i++)
    columns[i].createColumnElement(tableId, newRow, newCount);

  document.getElementById(countElementId).value = newCount + 1;

  // Clear validation results for base row
  for (var i = 0; i < columns.length; i++)
    resetValidationChanges(getColumnBaseElement(tableId, columns[i].columnId));

  // Set focus to first element
  for (var i = 0; i < columns.length; i++) {
    var baseElement = getColumnBaseElement(tableId, columns[i].columnId);
    if (baseElement != null) {
      baseElement.focus();
      return;
    }
  }
}