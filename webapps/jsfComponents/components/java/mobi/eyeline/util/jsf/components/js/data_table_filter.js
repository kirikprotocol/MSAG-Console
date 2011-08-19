function DataTableFilter(filterId) {

  var id = "action."+filterId;

  this.clickApply = function(action) {
    var e = document.getElementById(id);
    e.value = action;
    for (var k = 0; k < document.forms.length; k++) {
      var form = document.forms[k];
      if (form.elements[id] != null) {
        return form.submit();
      }
    }
    return false;
  }
}
