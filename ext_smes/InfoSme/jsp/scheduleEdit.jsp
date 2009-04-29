<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.ScheduleEdit,
                 ru.novosoft.smsc.util.StringEncoderDecoder, java.util.Iterator"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.ScheduleEdit" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

	int beanResult = bean.process(request);
	switch(beanResult)
	{
		case ScheduleEdit.RESULT_DONE:
			response.sendRedirect("shedules.jsp");
			return;
		default:
      {
        %><%@ include file="inc/menu_switch.jsp"%><%
      }
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<input type=hidden name=create value=<%=bean.isCreate()%>>
<input type=hidden name=oldSchedule value="<%=bean.getOldSchedule()%>">
<script type="text/javascript">
function selectType()
{
  var typeSel = document.getElementById('typeSelect').options[document.getElementById('typeSelect').selectedIndex];

  if (typeSel != null && typeSel.value != null)
  {
    var tableRows = document.getElementById('tableId').rows;

    var row_endDateTime = tableRows.row_endDateTime;
    var row_everyNDays = tableRows.row_everyNDays ;
    var row_everyNWeeks = tableRows.row_everyNWeeks;
    var row_weekDays = tableRows.row_weekDays;
    var row_dayOfMonth = tableRows.row_dayOfMonth;
    var row_weekDay = tableRows.row_weekDay;
    var row_monthes = tableRows.row_monthes;
    var row_intervalTime = tableRows.row_intervalTime;

    row_endDateTime.style.display="none";
    row_everyNDays.style.display="none";
    row_everyNWeeks.style.display="none";
    row_weekDays.style.display="none";
    row_dayOfMonth.style.display="none";
    row_weekDay.style.display="none";
    row_monthes.style.display="none";
    row_intervalTime.style.display="none";

    if ("once" == typeSel.value) {
    } else if ("daily" == typeSel.value) {
      row_endDateTime.style.display="block";
      row_everyNDays.style.display="block";
    } else if ("weekly" == typeSel.value) {
      row_endDateTime.style.display="block";
      row_everyNWeeks.style.display="block";
      row_weekDays.style.display="block";
    } else if ("monthly" == typeSel.value) {
      row_endDateTime.style.display="block";
      row_dayOfMonth.style.display="block";
      row_weekDay.style.display="block";
      row_monthes.style.display="block";
    } else if ("interval" == typeSel.value) {
      row_endDateTime.style.display="block";
      row_intervalTime.style.display="block";
    }
  }
  return true;
}
function selectMontlyType()
{
  if (document.getElementById('monthlyTypeDay').checked == true)
  {
    document.getElementById('monthlyTypeDayCell').disabled=false;
    document.getElementById('monthlyTypeWeekCell').disabled=true;
    document.getElementById('weekDayNSelect').disabled=true;
    document.getElementById('weekDaySelect').disabled=true;
  } else {
    document.getElementById('monthlyTypeDayCell').disabled=true;
    document.getElementById('monthlyTypeWeekCell').disabled=false;
    document.getElementById('weekDayNSelect').disabled=false;
    document.getElementById('weekDaySelect').disabled=false;
  }
}
</script>
<table id=tableId class=properties_list>
<col width="10%">
<tr class=row0>
  <th><%=getLocString("infosme.label.name")%></th>
  <td><input class=txt name=name value="<%=StringEncoderDecoder.encode(bean.getName())%>"></td>
</tr>
<tr class=row1>
  <th><%=getLocString("infosme.label.type")%></th>
  <td><select name=execute id=typeSelect onChange="return selectType();">
    <option value="once"     <%="once"    .equalsIgnoreCase(bean.getExecute()) ? "selected" : ""%>><%=getLocString("infosme.label.once")%></option>
    <option value="daily"    <%="daily"   .equalsIgnoreCase(bean.getExecute()) ? "selected" : ""%>><%=getLocString("infosme.label.dayly")%></option>
    <option value="weekly"   <%="weekly"  .equalsIgnoreCase(bean.getExecute()) ? "selected" : ""%>><%=getLocString("infosme.label.weekly")%></option>
    <option value="monthly"  <%="monthly" .equalsIgnoreCase(bean.getExecute()) ? "selected" : ""%>><%=getLocString("infosme.label.monthly")%></option>
    <option value="interval" <%="interval".equalsIgnoreCase(bean.getExecute()) ? "selected" : ""%>><%=getLocString("infosme.label.interval")%></option>
  </select></td>
</tr>
<tr class=row0>
  <th><%=getLocString("infosme.label.tasks")%></th>
  <td><table><col width="1%"><col width="99%"><%
    for (Iterator i = bean.getAllTasks().iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      String taskIdEnc = StringEncoderDecoder.encode(taskId);
      String taskIdHex = StringEncoderDecoder.encodeHEX(taskId);
      %><tr><td style="border:none;"><input class=check type=checkbox name=checkedTasks id=checkedTasks_<%=taskIdHex%> value="<%=taskIdEnc%>" <%=bean.isTaskChecked(taskId) ? "checked" : ""%>></td><td style="border:none;"><label for=checkedTasks_<%=taskIdHex%>><%=StringEncoderDecoder.encode(bean.getTaskName(taskId))%></label></td></tr><%
    }
  %></table></td>
</tr>
<tr class=row1>
  <th><%=getLocString("infosme.label.start")%></th>
  <td nowrap><input type=text id=startDateTime name=startDateTime class=calendarField value="<%=StringEncoderDecoder.encode(bean.getStartDateTime())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(startDateTime, false, true);">...</button></td>
</tr>
<tr id=row_endDateTime class=row0>
  <th><%=getLocString("infosme.label.end")%></th>
  <td nowrap><input type=text id=endDateTime name=endDateTime class=calendarField value="<%=StringEncoderDecoder.encode(bean.getEndDateTime())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(endDateTime, false, true);">...</button></td>
</tr>
<tr id=row_everyNDays class=row1>
  <th><%=getLocString("infosme.label.everyNDays")%></th>
  <td><input class=txt name=everyNDays value="<%=StringEncoderDecoder.encode(bean.getEveryNDays())%>"></td>
</tr>
<tr id=row_everyNWeeks class=row1>
  <th><%=getLocString("infosme.label.everyNWeeks")%></th>
  <td><input class=txt name=everyNWeeks value="<%=StringEncoderDecoder.encode(bean.getEveryNWeeks())%>"></td>
</tr>
<tr id=row_weekDays class=row0>
  <th><%=getLocString("infosme.label.weekDays")%></th>
  <td><table>
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <col width="1%"><col width="32%">
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Mon value=Mon <%=bean.isWeekDayChecked("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Mon><%=getLocString("infosme.label.monday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Thu value=Thu <%=bean.isWeekDayChecked("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Thu><%=getLocString("infosme.label.thursday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Sat value=Sat <%=bean.isWeekDayChecked("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Sat><%=getLocString("infosme.label.saturday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Tue value=Tue <%=bean.isWeekDayChecked("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Tue><%=getLocString("infosme.label.tuesday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Fri value=Fri <%=bean.isWeekDayChecked("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Fri><%=getLocString("infosme.label.friday")%></label></td>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Sun value=Sun <%=bean.isWeekDayChecked("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Sun><%=getLocString("infosme.label.sunday")%></label></td>
    <tr>
    <tr>
      <td style="border:none;"><input class=check type=checkbox name=checkedWeekDays id=checkedWeekDays_Wed value=Wed <%=bean.isWeekDayChecked("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=checkedWeekDays_Wed><%=getLocString("infosme.label.wednesday")%></label></td>
    <tr>
  </table></td>
</tr>
<tr id=row_dayOfMonth class=row1>
  <th><input type=radio class=radio name=monthlyType id=monthlyTypeDay value=day onclick="return selectMontlyType();" <%="day".equalsIgnoreCase(bean.getMonthlyType()) ? "checked" : ""%>><label for=monthlyTypeDay><%=getLocString("infosme.label.monthDay")%></label></th>
  <td id=monthlyTypeDayCell><input class=txt name=dayOfMonth value="<%=StringEncoderDecoder.encode(bean.getDayOfMonth())%>"></td>
</tr>
<tr id=row_weekDay class=row0>
  <th><input type=radio class=radio name=monthlyType id=monthlyTypeWeek value=week onclick="return selectMontlyType();" <%="week".equalsIgnoreCase(bean.getMonthlyType()) ? "checked" : ""%>><label for=monthlyTypeWeek><%=getLocString("infosme.label.weekDay")%></label></th>
  <td id=monthlyTypeWeekCell><select name=weekDayN id=weekDayNSelect>
    <option value="first"  <%="first" .equalsIgnoreCase(bean.getWeekDayN()) ? "selected" : ""%>><%=getLocString("infosme.label.first")%></option>
    <option value="second" <%="second".equalsIgnoreCase(bean.getWeekDayN()) ? "selected" : ""%>><%=getLocString("infosme.label.second")%></option>
    <option value="third"  <%="third" .equalsIgnoreCase(bean.getWeekDayN()) ? "selected" : ""%>><%=getLocString("infosme.label.third")%></option>
    <option value="fourth" <%="fourth".equalsIgnoreCase(bean.getWeekDayN()) ? "selected" : ""%>><%=getLocString("infosme.label.fourth")%></option>
    <option value="last"   <%="last"  .equalsIgnoreCase(bean.getWeekDayN()) ? "selected" : ""%>><%=getLocString("infosme.label.last")%></option>
  </select><select name=weekDay id=weekDaySelect>
    <option value="Mon" <%="Mon".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.monday")%></option>
    <option value="Tue" <%="Tue".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.tuesday")%></option>
    <option value="Wed" <%="Wed".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.wednesday")%></option>
    <option value="Thu" <%="Thu".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.thursday")%></option>
    <option value="Fri" <%="Fri".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.friday")%></option>
    <option value="Sat" <%="Sat".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.saturday")%></option>
    <option value="Sun" <%="Sun".equalsIgnoreCase(bean.getWeekDay()) ? "selected" : ""%>><%=getLocString("infosme.label.sunday")%></option>
  </select></td>
</tr>
<tr id=row_monthes class=row1>
  <th><%=getLocString("infosme.label.month")%></th>
  <td>
    <table>
    <col width="1%"><col width="24%">
    <col width="1%"><col width="24%">
    <col width="1%"><col width="24%">
    <col width="1%"><col width="24%">
    <tr>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_jan value="Jan" <%=bean.isMonthChecked("Jan") ? "checked" : ""%>></td><td style="border:none;"><label for=month_jan><%=getLocString("infosme.label.january")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_apr value="Apr" <%=bean.isMonthChecked("Apr") ? "checked" : ""%>></td><td style="border:none;"><label for=month_apr><%=getLocString("infosme.label.april")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_jul value="Jul" <%=bean.isMonthChecked("Jul") ? "checked" : ""%>></td><td style="border:none;"><label for=month_jul><%=getLocString("infosme.label.july")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_oct value="Oct" <%=bean.isMonthChecked("Oct") ? "checked" : ""%>></td><td style="border:none;"><label for=month_oct><%=getLocString("infosme.label.october")%></label></td>
    </tr>
    <tr style="border:none;">
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_feb value="Feb" <%=bean.isMonthChecked("Feb") ? "checked" : ""%>></td><td style="border:none;"><label for=month_feb><%=getLocString("infosme.label.february")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_may value="May" <%=bean.isMonthChecked("May") ? "checked" : ""%>></td><td style="border:none;"><label for=month_may><%=getLocString("infosme.label.may")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_aug value="Aug" <%=bean.isMonthChecked("Aug") ? "checked" : ""%>></td><td style="border:none;"><label for=month_aug><%=getLocString("infosme.label.august")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_nov value="Nov" <%=bean.isMonthChecked("Nov") ? "checked" : ""%>></td><td style="border:none;"><label for=month_nov><%=getLocString("infosme.label.november")%></label></td>
    </tr>
    <tr style="border:none;">
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_mar value="Mar" <%=bean.isMonthChecked("Mar") ? "checked" : ""%>></td><td style="border:none;"><label for=month_mar><%=getLocString("infosme.label.march")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_jun value="Jun" <%=bean.isMonthChecked("Jun") ? "checked" : ""%>></td><td style="border:none;"><label for=month_jun><%=getLocString("infosme.label.june")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_sep value="Sep" <%=bean.isMonthChecked("Sep") ? "checked" : ""%>></td><td style="border:none;"><label for=month_sep><%=getLocString("infosme.label.september")%></label></td>
      <td width="1%" style="border:none;"><input type=checkbox class=check name=checkedMonths id=month_dec value="Dec" <%=bean.isMonthChecked("Dec") ? "checked" : ""%>></td><td style="border:none;"><label for=month_dec><%=getLocString("infosme.label.december")%></label></td>
    </tr>
    </table>
  </td>
</tr>
<tr id=row_intervalTime class=row1>
  <th><%=getLocString("infosme.label.intervalTime")%></th>
  <td><input class=txt name=intervalTime value="<%=StringEncoderDecoder.encode(bean.getIntervalTime())%>"></td>
</tr>
</table>
<script type="text/javascript">
selectType();
selectMontlyType();
</script>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",  "infosme.hint.done_editing");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "infosme.hint.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>