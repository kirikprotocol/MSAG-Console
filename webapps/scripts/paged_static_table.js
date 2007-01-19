function navigatePage(startPositionInputName, pageSize, pageNum) {
  document.getElementById(startPositionInputName).value = pageNum * pageSize;
  opForm.submit();
  return false;
}