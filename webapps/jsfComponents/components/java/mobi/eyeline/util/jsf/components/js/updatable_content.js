function initUpdatableContent(contentId, updatePeriod) {
  $(function() {
    var c = $("#" + contentId);

    c.bind("updateContent", function() {
      var closestForm = $(this).parents("form");
      if (closestForm == null)
        return;

      var formValues = serializeForm(closestForm);
      formValues["eyelineComponentUpdate"] = contentId;

      $.ajaxSetup({cache: false});
      $.post(closestForm.attr("action"), formValues, function(data, status, resp) {
        if (status == 'success') {
          c.html(data);
          var nextUpdateTimeout = resp.getResponseHeader("nextUpdateTimeout");
          if (nextUpdateTimeout != null && nextUpdateTimeout != "0") {
            window.setTimeout(function(){c.trigger("updateContent")}, nextUpdateTimeout);
          }
        }
      });
    });

    if (updatePeriod > 0)
      window.setTimeout(function(){c.trigger("updateContent")}, updatePeriod);

  });
}