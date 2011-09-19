
function serializeForm(frm) {
  var result = {};
  $.each(frm.find(":input[name]"), function(idx, cur) {
    var jcur = $(cur);
    if (jcur.is("select")) {
      var opt = jcur.find("option[selected]");
      if (opt != null)
        result[jcur.attr("name")] = opt.val();
    } else if (jcur.is(":checkbox")) {
      if (cur.getAttribute("checked"))
        result[jcur.attr("name")] = "on";
    } else {
      var value = cur.getAttribute("value");
      if (value)
        result[jcur.attr("name")] = value;
    }
  });
  return result;
}
