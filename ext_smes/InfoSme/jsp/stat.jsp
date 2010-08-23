<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.util.StringEncoderDecoder,
                 java.util.*, ru.novosoft.smsc.infosme.backend.Statistics,
                 ru.novosoft.smsc.infosme.backend.DateCountersSet,
                 ru.novosoft.smsc.infosme.backend.HourCountersSet,
                 java.text.SimpleDateFormat, ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ page import="ru.novosoft.smsc.infosme.beans.TasksStatistics" %>
<%@ page import="ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper" %>
<%@ page import="ru.novosoft.smsc.infosme.backend.tables.stat.TaskStatTableHelper" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TasksStatistics" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("infosme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int rowN = 0;
	int beanResult = bean.process(request);
  if (beanResult == TasksStatistics.RESULT_EXPORT) {
    response.sendRedirect(CPATH+"/esme_InfoSme/csv_download.jsp?taskId=" + StringEncoderDecoder.encode(bean.getTaskId()) + "&fromDate=" + StringEncoderDecoder.encode(bean.getFromDate()) + "&tillDate=" + StringEncoderDecoder.encode(bean.getTillDate()) + "&view=" + bean.getView()
      +(bean.getArchiveDate() != null ? "&archiveDate="+bean.getArchiveDate() : ""));
    return;
  }
%>
<%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/calendar.jsp"%>
<div class=content>
<div class=page_subtitle><%= getLocString("infosme.subtitle.stat_params")%></div>
<input type=hidden name=initialized value="<%=bean.isInitialized()%>">
<input type=hidden name=taskId value="<%=bean.getTaskId()%>">
<input type=hidden name=archiveDate value="<%=bean.getArchiveDate()%>">
<table class=properties_list>
<col width="5%">
<col width="45%">
<col width="5%">
<col width="45%">
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("infosme.label.task")%></th>
  <td colspan=1><%= bean.getTaskId() == null || bean.getTaskId().length() == 0 ? getLocString("infosme.label.all") : bean.getTaskName()%></td>
  <th style="text-align:left"><%= getLocString("infosme.label.view")%></th>
  <td colspan=2>
    <select name="view">
      <option value="<%=TasksStatistics.VIEW_DATES%>" <%=bean.getView() == TasksStatistics.VIEW_DATES ? "selected" : ""%>><%=getLocString("infosme.label.stat.view.dates")%></option>
      <option value="<%=TasksStatistics.VIEW_TASKS%>" <%=bean.getView() == TasksStatistics.VIEW_TASKS ? "selected" : ""%>><%=getLocString("infosme.label.stat.view.tasks")%></option>
    </select>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th style="text-align:left"><%= getLocString("infosme.label.from_date")%></th>
  <td><input class=calendarField id=fromDate name=fromDate value="<%=StringEncoderDecoder.encode(bean.getFromDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(fromDate, false, true);">...</button></td>
  <th style="text-align:left"><%= getLocString("infosme.label.till_date")%></th>
  <td><input class=calendarField id=tillDate name=tillDate value="<%=StringEncoderDecoder.encode(bean.getTillDate())%>" maxlength=20 style="z-index:22;"><button class=calendarButton type=button onclick="return showCalendar(tillDate, false, true);">...</button></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "infosme.hint.query_stat");
page_menu_space(out);
page_menu_end(out);

  if (bean.getView() == TasksStatistics.VIEW_DATES) {
  Statistics statistics = bean.getStatistics();
  if (statistics != null)
  {
    %><div class=content>
    <script type="text/javascript">
      function toggleVisible(p, c)
      {
        var o = p.className == "collapsing_list_opened";
        p.className = o ? "collapsing_list_closed"  : "collapsing_list_opened";
        c.style.display = o ? "none" : "";
      }
    </script>
    <table class=list cellspacing=0>
    <col width="60%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <col width="10%">
    <tr>
      <th><%= getLocString("infosme.label.date")%></th>
      <th><%= getLocString("infosme.label.generated")%></th>
      <th><%= getLocString("infosme.label.delivered")%></th>
      <th><%= getLocString("infosme.label.retried")%></th>
      <th><%= getLocString("infosme.label.failed")%></th>
    </tr>
    <%
      for (Iterator i = statistics.getCountersByDates().iterator(); i.hasNext();) {
        DateCountersSet dateCounters =    (DateCountersSet) i.next();
        if (dateCounters != null) {
        SimpleDateFormat formatter = new SimpleDateFormat("dd-MM-yyyy");
        String date = formatter.format(dateCounters.getDate());
        String dateHex = StringEncoderDecoder.encodeHEX(date);
        %><tr class=row0>
          <td><div id="p<%=dateHex%>" class="collapsing_list_<%=i.hasNext() ? "closed" : "opened"%>" onClick="toggleVisible(this, document.getElementById('c<%=dateHex%>'));"><%=date%></div></td>
          <td><%=dateCounters.generated%></td>
          <td><%=dateCounters.delivered%></td>
          <td><%=dateCounters.retried%></td>
          <td><%=dateCounters.failed%></td>
        </tr><tr id="c<%=dateHex%>" style="display:<%=i.hasNext() ? "none" : ""%>"><td colspan=5><table class=list cellspacing=0 cellpadding=0 border=0>
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
      }
    %>
      <tr class=row1>
       <td><%= getLocString("infosme.label.total")%></td>
       <td><%= statistics.getCounters().generated%></td>
       <td><%= statistics.getCounters().delivered%></td>
       <td><%= statistics.getCounters().retried%></td>
       <td><%= statistics.getCounters().failed%></td>
      </tr>
    </table>
<%--
final String exportFile = "temp/"+bean.getExportFilePath();
if (exportFile != null) {
%><br/><a class=font href="<%= exportFile%>">Download as CSV file</a><%
}--%>
    </div>
<%}%>
<% } else { %>
<%final TaskStatTableHelper tableHelper = bean.getTasksStatsTable();%>
<%@ include file="/WEB-INF/inc/paged_static_table.jsp"%>
<div class=content>
<div class=page_subtitle><%=getLocString("infosme.label.total")%>:</div>
<table class=list>
  <tr class=row1>
    <td width="1" nowrap><%=getLocString("infosme.label.generated")%></td>
    <td width="1" nowrap><%=tableHelper.getTotalGenerated()%></td>
    <td width="100%">&nbsp;</td>
  </tr>
  <tr class=row0>
    <td width="1" nowrap><%=getLocString("infosme.label.delivered")%></td>
    <td width="1" nowrap><%=tableHelper.getTotalDelivered()%></td>
    <td width="100%">&nbsp;</td>
  </tr>
  <tr class=row1>
    <td width="1" nowrap><%=getLocString("infosme.label.retried")%></td>
    <td width="1" nowrap><%=tableHelper.getTotalRetried()%></td>
    <td width="100%">&nbsp;</td>
  </tr>
  <tr class=row0>
    <td width="1" nowrap><%=getLocString("infosme.label.failed")%></td>
    <td width="1" nowrap><%=tableHelper.getTotalFailed()%></td>
    <td width="100%">&nbsp;</td>
  </tr>
</table>
  </div>
<% } %>
<% if (bean.isInitialized()) {
  page_menu_begin(out);
  page_menu_space(out);
  page_menu_button(session, out, "mbExport",  "common.buttons.export",  "infosme.hint.download_statistics");
  page_menu_end(out);
} %>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>