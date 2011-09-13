function initLocales() {
  jQuery(function($) {
    $.datepicker.regional['ru'] = {
      closeText: 'Закрыть',
      prevText: '&#x3c;Пред',
      nextText: 'След&#x3e;',
      currentText: 'Сегодня',
      monthNames: ['Январь','Февраль','Март','Апрель','Май','Июнь',
        'Июль','Август','Сентябрь','Октябрь','Ноябрь','Декабрь'],
      monthNamesShort: ['Янв','Фев','Мар','Апр','Май','Июн',
        'Июл','Авг','Сен','Окт','Ноя','Дек'],
      dayNames: ['воскресенье','понедельник','вторник','среда','четверг','пятница','суббота'],
      dayNamesShort: ['вск','пнд','втр','срд','чтв','птн','сбт'],
      dayNamesMin: ['Вс','Пн','Вт','Ср','Чт','Пт','Сб'],
      weekHeader: 'Не',
      dateFormat: 'dd.mm.yy',
      firstDay: 1,
      isRTL: false,
      showMonthAfterYear: false,
      yearSuffix: ''};

    $.datepicker.regional['en'] = {
      closeText: 'Close',
      prevText: '&#x3c;Prev',
      nextText: 'Next&#x3e;',
      currentText: 'Today',
      monthNames: ['January','February','March','April','May','June',
        'July','August','September','October','November','December'],
      monthNamesShort: ['Jan','Feb','Mar','Apr','May','Jun',
        'Jul','Aug','Sep','Oct','Nov','Dec'],
      dayNames: ['sunday','monday','tuesday','wednesday','thursday','friday','saturday'],
      dayNamesShort: ['sun','mon','tue','wed','thu','fri','sat'],
      dayNamesMin: ['SSS','Mo','Tu','We','Th','Fr','Sa'],
      weekHeader: 'Week',
      dateFormat: 'dd.mm.yy',
      firstDay: 1,
      isRTL: false,
      showMonthAfterYear: false,
      yearSuffix: ''};
  });
}

function initInputDate(elementId, value, minDate, maxDate, inputTime, numberOfMonths, locale) {
  initLocales();

  $(function() {
    var options = {
      showAnim:"",
      constrainInput : false,
      showOn: "button",
      dateFormat: "dd.mm.yy"
    };

    if (numberOfMonths == 1) {
      options["changeMonth"] = true;
      options["changeYear"] = true;
    } else {
      options["numberOfMonths"] = numberOfMonths;
    }

    if (inputTime) {
      options["beforeShow"] = function(input, inst) {
        this.setAttribute("oldValue", this.value)
      };
      options["onSelect"] = function(dateText, inst) {
        var oldDate = this.getAttribute("oldValue");
        var i = oldDate.indexOf(" ");
        var oldTime = i < 0 ? " 00:00:00" : oldDate.substring(i);
        this.value = dateText + oldTime;
      }
    }

    $("#" + elementId).datepicker(options);

    if (locale == "ru") {
      $("#" + elementId).datepicker("option", $.datepicker.regional['ru']);
    }

    if (minDate)
      $("#" + elementId).datepicker("option", "minDate", minDate);

    if (maxDate)
      $("#" + elementId).datepicker("option", "maxDate", maxDate);

    $("#" + elementId).val(value);

  });

}