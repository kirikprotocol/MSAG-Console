<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscDataItem,
                 org.apache.log4j.Category,
                 java.net.URLEncoder"%>
<%@ page import="ru.novosoft.smsc.admin.mscman.*"%>
<%@ page import="ru.novosoft.smsc.jsp.mscman.*"%>
<jsp:useBean id="mscManagerFormBean" scope="session" class="ru.novosoft.smsc.jsp.mscman.MscManagerFormBean" />
<%
	MscManagerFormBean bean = mscManagerFormBean;
%>
<jsp:setProperty name="mscManagerFormBean" property="*"/>
<%
    TITLE="Mobile Switching Centers lock status";
    MENU0_SELECTION = "MENU0_MSCMAN";

    bean.setPrefix(request.getParameter("prefix"));
    int beanResult = mscManagerFormBean.RESULT_OK;
    switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
    {
        case MscManagerFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp?refreshed=true&prefix=" + URLEncoder.encode(bean.getPrefix(), "UTF-8"));
            return;
        case MscManagerFormBean.RESULT_FILTER:
        case MscManagerFormBean.RESULT_OK:
            STATUS.append("Ok");
            break;
        case MscManagerFormBean.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<input type=hidden name=sort value="<%=bean.getSort()%>">
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
Commutator&nbsp;number&nbsp;prefix:<input class=txt name=prefix value="<%=bean.getPrefix()%>">
</div>
<%
  page_menu_begin(out);
  page_menu_button(out, "mbQuery",  "Query",  "Query");
  page_menu_space(out);
  page_menu_end(out);
%>
<div class=content>
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
<div class=page_subtitle>Commutators status</div>
<%  QueryResultSet mscList = bean.getMscs();
    if (false && (mscList == null || mscList.getTotalSize() <= 0)) {
      %><div>No Mobile Switching Centers defined</div><br><%
    } else {
        int posIdx = 0;%>
<table class=list cellspacing=0>
<col width=20%>
<col width=25%>
<col width=10%>
<col width=45%>
<tr>
	  <th><a href="#" <%=bean.getSort().endsWith("commutator") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by commutator" onclick='return setSort("commutator")'>Commutator</a></th>
	  <th><a href="#" <%=bean.getSort().endsWith("status")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by status"     onclick='return setSort("status")'    >Status</a></th>
	  <th><a href="#" <%=bean.getSort().endsWith("failures")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by failures"   onclick='return setSort("failures")'  >Failures</a></th>
	  <th>Actions</th>
</tr><%
for (Iterator i = mscList.iterator(); i.hasNext();) {
  MscDataItem item = (MscDataItem) i.next();
  final String commutator = item.getCommutator();
%>
<tr class=row<%=(posIdx++)%2%>>
    <td><%= commutator == null || commutator.length() == 0? "&lt;&lt;undefined&gt;&gt;" : StringEncoderDecoder.encode(commutator)%></td>
    <td align=center><%= StringEncoderDecoder.encode(item.getStatus())%></td>
    <td align=center><%= item.getFailuresCount()%></td>
    <td align=right>
		<%button(out, !item.ismLock() ? "but_lock.gif" : "but_lock_dis.gif",
             "mbBlock",      "Lock",       "Lock this MSC",       "opForm.mscKey.value='" + commutator + "'");%>
		<%button(out, item.isaLock() || item.ismLock() ? "but_unlock.gif" : "but_unlock_dis.gif",
             "mbClear",      "UnLock",     "UnLock this MSC",     "opForm.mscKey.value='" + commutator + "'");%>
		<%button(out, "but_unregister.gif",
             "mbUnregister", "UnRegister", "Unregister this MSC", "opForm.mscKey.value='" + commutator + "'");%>
    </td>
</tr><%
} //for
%>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
<%
} //if mscList == null
%>
<br>
<div class=page_subtitle>Register new Mobile Switching Center</div>
<table class=list cellspacing=1 width="100%">
<tr class=row0>
     <td width="1%">
        <input type="hidden" name="mscKey" value="none">
        <input class=txt type="text" name="mscNum" value="<%=bean.getMscNum()%>" size=21 maxlength=21>
    </td>
<td width="1%"><%button(out, "but_register.gif", "mbRegister", "Register", "Register new MSC");%></td>
<td width="98%">&nbsp;</td>
</tr>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

