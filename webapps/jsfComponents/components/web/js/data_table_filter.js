function DataTableFilter(filterId) {

  var id = "action"+filterId;

  var jel = $("#"+id);
  var jform = jel.parents("form");

  this.clickApply = function(action) {
    jel.val(action);
    $("[name='source']").val(null)
    return jform.submit();
  }
}
