<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.journal.Index,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.util.*,
                 ru.novosoft.smsc.admin.journal.Action,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.journal.Actions,
                 java.text.DateFormat,
                 ru.novosoft.smsc.util.SortedList,
                 ru.novosoft.smsc.admin.journal.SubjectTypes,
                 ru.novosoft.smsc.jsp.PageBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.journal.Index" />
<jsp:setProperty name="bean" property="*"/>
<%
  TITLE="Journal";
  MENU0_SELECTION = "MENU0_JOURNAL";

  int beanResult = bean.RESULT_OK;
  switch(beanResult = bean.process(request))
  {
    case Index.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Index.RESULT_FILTER:
      response.sendRedirect("filter.jsp");
      return;
    case Index.RESULT_OK:
      break;
    case Index.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
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

<table class=list cellspacing=0>
<col width="10%">
<col width="10%">
<%--col width="12%"--%>
<col width="10%">
<col width="25%">
<col width="10%">
<col width="10%">
<col width="25%">
<thead>
<tr>
	<th><a href="#" <%=bean.getSort().endsWith("timestamp")  ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by time"  onclick='return setSort("timestamp") '>time</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("user") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by user" onclick='return setSort("user")'>user</a></th>
	<%--th><a href="#" <%=bean.getSort().endsWith("sessionId") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by session" onclick='return setSort("sessionId")'>session ID</a></th--%>
	<th><a href="#" <%=bean.getSort().endsWith("subjectType") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by subject type" onclick='return setSort("subjectType")'>subject type</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("subjectId") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by subject ID" onclick='return setSort("subjectId")'>subject ID</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("action") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by action" onclick='return setSort("action")'>action</a></th>
  <th colspan=2>additional info</th>
</tr>
</thead>
<tbody>
<%
  int row = 0;
  DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.MEDIUM, request.getLocale());
  for (Iterator i = bean.getActions().iterator(); i.hasNext(); )
  {
    Action action = (Action) i.next();
    final String rowspan = action.getAdditionalKeys().size() > 0 ? " rowspan=" + action.getAdditionalKeys().size() : "";
%><tr class=row<%=(row++)&1%>>
	  <td<%=rowspan%> nowrap><%=StringEncoderDecoder.encode(dateFormat.format(action.getTimestamp()))%></td>
    <td<%=rowspan%>><%=StringEncoderDecoder.encode(action.getUser())%></td>
    <%--td<%=rowspan%>><%=StringEncoderDecoder.encode(action.getSessionId())%></td--%>
    <td<%=rowspan%>><%=StringEncoderDecoder.encode(SubjectTypes.typeToString(action.getSubjectType()))%></td>
    <td<%=rowspan%>><%=StringEncoderDecoder.encode(action.getSubjectId())%></td>
    <td<%=rowspan%>><%=StringEncoderDecoder.encode(Actions.actionToString(action.getAction()))%></td>
    <%
    if (action.getAdditionalKeys().size() > 0) {
      boolean firstRow = true;
      for (Iterator j = new SortedList(action.getAdditionalKeys()).iterator(); j.hasNext();) {
        String additionalKey = (String) j.next();
        String additionalValue = action.getAdditionalValue(additionalKey);
        %><%=firstRow ? "" : "<tr>"%><td align=right><%=additionalKey%>:</td><td><%=additionalValue%></td></tr><%
      }
    } else {
      out.print("<td>&nbsp;</td><td>&nbsp;</td>");
    }
    %>
  </tr><%
  }
%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbClearNonappliable",  "Clear non-appliable",  "Clear all non-appliable entries", "return confirm('Are you sure to clear all non-appliable entries?');");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>