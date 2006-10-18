<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.AlarmStatFormBean,
                 java.text.SimpleDateFormat" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.tables.StatsTableScheme"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.tables.StatsTableRow"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.tables.AllStatsTableScheme"%>
<jsp:useBean id="alarmStatBean" scope="session" class="ru.novosoft.smsc.jsp.smsc.snmp.AlarmStatFormBean"/>
<%
    AlarmStatFormBean bean = alarmStatBean;
%>
<jsp:setProperty name="alarmStatBean" property="*"/>
<%
    TITLE = getLocString("smsview.title");
    MENU0_SELECTION = "MENU0_SMSVIEW";

    int beanResult = bean.process(request);
    switch (beanResult) {
        case AlarmStatFormBean.RESULT_DONE:
            response.sendRedirect("alarmStat.jsp");
            return;
        case AlarmStatFormBean.RESULT_FILTER:
        case AlarmStatFormBean.RESULT_OK:
            break;
        case AlarmStatFormBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/calendar.jsp" %>
<div class=content>
<div class=page_subtitle><%=getLocString("common.titles.searchParams")%></div>
<table class=properties_list cell>
<tr class=row0>
    <th><%=getLocString("snmp.viewDate")%><input type=text id=date name=date class=calendarField value="<%=bean.getDate()%>"
                      maxlength=20 style="z-index:22;">
        <button class=calendarButton type=button onclick="return showCalendar(date, false, false);">...</button></th>
    <td nowrap>
    </td>
</tr>
</table>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbQuery", "common.buttons.queryExcl", "common.buttons.runQuery");
    page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearQueryParams");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content><%
    if (bean.isHasResults()) {%>
    <input type=hidden name=sort>
    <input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
    <script>
        function setSort(sorting)
        {
            if (sorting == "<%=bean.getSort()%>")
                opForm.sort.value = "-<%=bean.getSort()%>";
            else
                opForm.sort.value = sorting;

            opForm.submit();
            return false;
        }
    </script>
    <br>
    <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ results ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <%
        SimpleDateFormat dateFormatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss", getLoc());
    %>
    <div class=page_subtitle><%=getLocString("common.titles.searchResultsPre") + bean.getTotalSize() + getLocString("common.titles.searchResultsPost")%></div>

    <%-- Stats table filter --%>
    <%=getLocString("snmp.filter")%>
    <select name="tableFilter" id="tableFilter" onchange="opFormSubmit()">
      <option value="<%=AlarmStatFormBean.ALL_STATS%>" <%=bean.getTableFilter() != null && bean.getTableFilter().equals(AlarmStatFormBean.ALL_STATS) ? "SELECTED" : ""%>><%=getLocString("snmp.allStats")%></option>
      <option value="<%=AlarmStatFormBean.OPENED_STATS%>" <%=bean.getTableFilter() != null && bean.getTableFilter().equals(AlarmStatFormBean.OPENED_STATS) ? "SELECTED" : ""%>><%=getLocString("snmp.openedStats")%></option>
      <option value="<%=AlarmStatFormBean.CLOSED_STATS%>" <%=bean.getTableFilter() != null && bean.getTableFilter().equals(AlarmStatFormBean.CLOSED_STATS) ? "SELECTED" : ""%>><%=getLocString("snmp.closedStats")%></option>
    </select>

    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>

    <table class=list cellspacing=0>
        <thead> <%-- Stats table header --%>
            <tr class=row0>
                <%
//                  final StatsTableScheme scheme = bean.getTableToShow().getScheme();
                  for (Iterator iter = AllStatsTableScheme.SCHEME.getFields(); iter.hasNext();) {
                    final String fieldName = (String)iter.next();
                %>
                  <th>
                    <a href="javascript:setSort('<%=fieldName%>')" <%=bean.getSort() != null ? (bean.getSort().endsWith(fieldName) ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : "") : ""%>
                       title="<%=getLocString("snmp." + fieldName)%>"><%=getLocString("snmp." + fieldName)%></a>
                  </th>
                <%
                  }
                %>
            </tr>
        </thead>
        <tbody> <%-- Stats table body --%>
          <%
            int firstIndex = bean.getStartPositionInt();
            int lastIndex = bean.getStartPositionInt() + bean.getPageSizeInt();
            if (lastIndex >= bean.getTotalSizeInt() || bean.getPageSizeInt() < 0)
                lastIndex = bean.getTotalSizeInt();

            {
                int rowN = 0;
                // Show neccessary fields
                for (int cnt = firstIndex; cnt <= lastIndex; cnt++, rowN++) {
                    final StatsTableRow row = bean.getTableToShow().getRow(cnt);
                    if (row == null) {
                      rowN--;
                      continue;
                    }

                  %>
                  <tr class=row<%=rowN&1%>0>
                  <%
                    for (Iterator fieldNames = AllStatsTableScheme.SCHEME.getFields(); fieldNames.hasNext();) {
                      final String fieldValue = row.getValueAsString((String)fieldNames.next());
                  %>
                      <td nowrap style="padding-left: 5px; border-left:dotted 1px #C0C0C0;">
                        <%= (fieldValue != null) ? StringEncoderDecoder.encode(fieldValue) : "&nbsp;"%>
                      </td>
                  <%
                    }
                  %>
                  </tr>
                  <%
                }
            }
          %>
        </tbody>
    </table>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
    <% } %>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>