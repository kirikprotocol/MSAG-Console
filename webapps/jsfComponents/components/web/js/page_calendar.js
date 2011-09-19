function PageCalendar(contentId) {

  var bodyElement = $("#"+contentId);
  var closestForm = bodyElement.parents("form");
  var requestUrl = closestForm.attr("action");

  var dateElement = document.getElementById(contentId + '_date');
  var updateUsingSubmit = false;

  /**
   * Обновляет содержимое
   */
  this.setDate = function (date) {

    dateElement.value=date;

    if (updateUsingSubmit)
      return closestForm.submit();

    var onResponse = function(text) {
      bodyElement.html(text);
    };

    var params = serializeForm(closestForm);
    params["eyelineComponentUpdate"] = contentId;
    $.ajaxSetup({cache: false});
    $.post(requestUrl, params, onResponse);
  };
}