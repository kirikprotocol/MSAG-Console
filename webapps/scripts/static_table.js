function sortColumn(sortedColumnIdInputName, columnId) {
  document.getElementById(sortedColumnIdInputName).value = columnId;
  opForm.submit();
  return false;
}

function selectCell(selectedColumnIdInputName, columnId, selectedCellIdInputName, cellId) {
  document.getElementById(selectedColumnIdInputName).value = columnId;
  document.getElementById(selectedCellIdInputName).value = cellId;
  opForm.submit();
  return false;
}