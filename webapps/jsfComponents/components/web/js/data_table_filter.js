function DataTableFilter(filterId) {

  var id = "action"+filterId;

  var jel = $("#"+id);
  var jform = jel.parents("form");

  this.clickApply = function(action) {
    jel.val(action);
    return jform.submit();
  }
}
