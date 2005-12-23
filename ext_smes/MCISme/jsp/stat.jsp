<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*, java.text.SimpleDateFormat,
                 ru.novosoft.smsc.mcisme.backend.Statistics,
                 ru.novosoft.smsc.mcisme.backend.DateCountersSet,
                 ru.novosoft.smsc.mcisme.backend.HourCountersSet"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.EventsStatistics" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int rowN = 0;
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<table class=properties_list>
<col width="10%">
<col width="40%">
<col width="10%">
<col width="40%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:right"><%= getLocString("mcisme.label.from")%></th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:right"><%= getLocString("mcisme.label.till")%></th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "mcisme.hint.query_stat");
page_menu_space(out);
page_menu_end(out);

  Statistics statistics = bean.getStatistics();
  if (statistics != null)
  {
    %><div class=content>
    <script type="text/javascript">
      function toggleVisible(p, c)
      {
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.style.display = o ? "none" : "block";
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
      <th>Missed</th>
      <th>Delivered</th>
      <th>Failed</th>
      <th>Notifications</th>
    </tr>
    <%
      for (Iterator i = statistics.getCountersByDates().iterator(); i.hasNext();) {
        DateCountersSet dateCounters =    (DateCountersSet) i.next();
        SimpleDateFormat formatter = new SimpleDateFormat("dd-MMM-yyyy");
        String date = formatter.format(dateCounters.getDate());
        String dateHex = StringEncoderDecoder.encodeHEX(date);
        %><tr class=row0>
          <td><div id="p<%=dateHex%>" class=collapsing_list_<%=i.hasNext() ? "closed" : "opened"%> onClick="toggleVisible(this, document.getElementById('c<%=dateHex%>'));"><%=date%><div></td>
          <td><%=dateCounters.missed%></td>
          <td><%=dateCounters.delivered%></td>
          <td><%=dateCounters.failed%></td>
          <td><%=dateCounters.notified%></td>
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
            <td><%=hourCounters.missed%></td>
            <td><%=hourCounters.delivered%></td>
            <td><%=hourCounters.failed%></td>
            <td><%=hourCounters.notified%></td>
          </tr><%
        }
        %></table></td></tr><%
      }
    %>
    </table>
    </div><%
  }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>