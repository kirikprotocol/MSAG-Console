<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.TasksStatistics,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*,
                 ru.novosoft.smsc.infosme.backend.Statistics,
                 ru.novosoft.smsc.infosme.backend.CountersSet,
                 ru.novosoft.smsc.infosme.backend.DateCountersSet,
                 ru.novosoft.smsc.infosme.backend.HourCountersSet,
                 java.text.SimpleDateFormat"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TasksStatistics" />
<jsp:setProperty name="bean" property="*"/>
<%
	//ServiceIDForShowStatus = ;
	TITLE="Informer SME Administration";
	MENU0_SELECTION = "MENU0_SERVICES";
	//MENU1_SELECTION = "WSME_INDEX";

  int rowN = 0;
	int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:right">Task</th>
  <td colspan=3><select name=taskId>
  <option value="<%=StringEncoderDecoder.encode(bean.ALL_TASKS_MARKER)%>">All</option>
  <%for (Iterator i = bean.getAllTasks().iterator(); i.hasNext();) {
      String taskId = (String) i.next();
      String taskIdEnc = StringEncoderDecoder.encode(taskId);
        %><option value="<%=taskIdEnc%>"><%=StringEncoderDecoder.encode(bean.getTaskName(taskId))%></option><%
    }
  %></select></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:right">From</th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:right">To</th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(out, "mbQuery",  "Query",  "Query statistics");
page_menu_space(out);
page_menu_end(out);

  Statistics statistics = bean.getStatistics();
  if (statistics != null)
  {
    %><div class=content>
    <script>
      function toggleVisible(p, c)
      {
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.runtimeStyle.display = o ? "none" : "block";
      }
    </script>
    <table class=list cellspacing=0>
    <col width="60%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <tr>
      <th>Date</th>
      <th>generated</th>
      <th>delivered</th>
      <th>retried</th>
      <th>failed</th>
    </tr>
    <%
      for (Iterator i = statistics.getCountersByDates().iterator(); i.hasNext();) {
        DateCountersSet dateCounters =    (DateCountersSet) i.next();
        SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
        String date = formatter.format(dateCounters.getDate());
        String dateHex = StringEncoderDecoder.encodeHEX(date);
        %><tr class=row0>
          <td><div id="p<%=dateHex%>" class=collapsing_list_<%=i.hasNext() ? "closed" : "opened"%> onClick="toggleVisible(this, opForm.all.c<%=dateHex%>);"><%=date%><div></td>
          <td><%=dateCounters.generated%></td>
          <td><%=dateCounters.delivered%></td>
          <td><%=dateCounters.retried%></td>
          <td><%=dateCounters.failed%></td>
        </tr><tr id="c<%=dateHex%>" style="display:<%=i.hasNext() ? "none" : "block"%>"><td colspan=5><table class=list cellspacing=0 cellpadding=0 border=0>
          <col width="60%">
          <col width="10%">
          <col width="10%">
          <col width="10%">
          <col width="10%">
<%
        for (Iterator j = dateCounters.getHourStat().iterator(); j.hasNext();) {
          HourCountersSet hourCounters = (HourCountersSet) j.next();
          %><tr class=row1>
            <td><%=hourCounters.getHour()%></td>
            <td><%=hourCounters.generated%></td>
            <td><%=hourCounters.delivered%></td>
            <td><%=hourCounters.retried%></td>
            <td><%=hourCounters.failed%></td>
          </tr><%
        }
        %></table></td></tr><%
      }
    %>
    </table>
    </div><%
    page_menu_begin(out);
    page_menu_button(out, "mbDone",  "Done",  "Done editing");
    page_menu_button(out, "mbCancel", "Cancel", "Cancel changes", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>