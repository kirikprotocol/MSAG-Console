function initCollapsingGroup(groupId) {
  var header = $("#" + groupId + "Header");
  var body = $("#" + groupId + "Body");
  var status = $("#" + groupId + "Status");

  header.click(function () {
    var idx = status.val();
    header.removeClass("eyeline_collapsingGroupHeader"+idx).addClass("eyeline_collapsingGroupHeader"+(1-idx));
    body.removeClass("eyeline_collapsingGroupBody"+idx).addClass("eyeline_collapsingGroupBody"+(1-idx));
    status.val(1 - idx);
  });
}