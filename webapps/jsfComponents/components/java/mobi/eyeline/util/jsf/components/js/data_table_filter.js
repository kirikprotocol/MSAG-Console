function DataTableFilter(filterId) {

  this.clickApply = function() {
    for (var k = 0; k < document.forms.length; k++) {
      var form = document.forms[k];
      if (form.elements[filterId] != null) {
        return form.submit();
      }
    }
    return false;
  }
}