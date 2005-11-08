<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet,
                 ru.novosoft.smsc.jsp.util.tables.impl.mscman.MscDataItem,
                 org.apache.log4j.Category,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.smsc.IndexBean"%>
<%@ page import="ru.novosoft.smsc.admin.mscman.*"%>
<%@ page import="ru.novosoft.smsc.jsp.mscman.*"%>
<jsp:useBean id="mscManagerFormBean" scope="session" class="ru.novosoft.smsc.jsp.mscman.MscManagerFormBean" />
<%
	MscManagerFormBean bean = mscManagerFormBean;
%>
<jsp:setProperty name="mscManagerFormBean" property="*"/>
<%
    TITLE=getLocString("mscman.title");
    MENU0_SELECTION = "MENU0_MSCMAN";

    bean.setPrefix(request.getParameter("prefix"));
    int beanResult = PageBean.RESULT_OK;
    switch(beanResult = bean.process(request))
    {
        case PageBean.RESULT_DONE:
            response.sendRedirect("index.jsp?refreshed=true&prefix=" + URLEncoder.encode(bean.getPrefix()));
            return;
        case IndexBean.RESULT_FILTER:
        case PageBean.RESULT_OK:
            break;
        case PageBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<input type=hidden name=sort value="<%=bean.getSort()%>">
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<%=getLocString("mscman.commutatorNumberPrefix")%> <input class=txt name=prefix value="<%=bean.getPrefix()%>">
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "common.buttons.query");
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
<div class=page_subtitle><%=getLocString("mscman.comSubTitle")%></div>
<%  QueryResultSet mscList = bean.getMscs();
    if (false && (mscList == null || mscList.getTotalSize() <= 0)) {
      %><div><%=getLocString("mscman.noMscDefined")%></div><br><%
    } else {
        int posIdx = 0;%>
<table class=list cellspacing=0>
<col width=20%>
<col width=25%>
<col width=10%>
<col width=45%>
<tr>
	  <th><a href="#" <%=bean.getSort().endsWith("commutator") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.commutatorHint")%>" onclick='return setSort("commutator")'><%=getLocString("common.sortmodes.commutator")%></a></th>
	  <th><a href="#" <%=bean.getSort().endsWith("status")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.statusHint")%>"     onclick='return setSort("status")'    ><%=getLocString("common.sortmodes.status")%></a></th>
	  <th><a href="#" <%=bean.getSort().endsWith("failures")   ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.failuresHint")%>"   onclick='return setSort("failures")'  ><%=getLocString("common.sortmodes.failures")%></a></th>
	  <th><%=getLocString("common.sortmodes.actions")%></th>
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
		<%button(out, !item.ismLock() ? getLocImageFileName("but_lock") : getLocImageFileName("but_lock_dis"),
             "mbBlock",      "Lock",       "mscman.lockButtonHint",       "opForm.mscKey.value='" + commutator + "'");%>
		<%button(out, item.isaLock() || item.ismLock() ? getLocImageFileName("but_unlock") : getLocImageFileName("but_unlock_dis"),
             "mbClear",      "UnLock",     "mscman.unlockButtonHint",     "opForm.mscKey.value='" + commutator + "'");%>
		<%button(out, getLocImageFileName("but_unregister"),
             "mbUnregister", "UnRegister", "mscman.unregButtonHint", "opForm.mscKey.value='" + commutator + "'");%>
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
<div class=page_subtitle><%=getLocString("mscman.regTitle")%></div>
<table class=list cellspacing=1 width="100%">
<tr class=row0>
     <td width="1%">
        <input type="hidden" name="mscKey" value="none">
        <input class=txt type="text" name="mscNum" value="<%=bean.getMscNum()%>" size=21 maxlength=21>
    </td>
<td width="1%"><%button(out, getLocImageFileName("but_register"), "mbRegister", "Register", "mscman.regButtonHint");%></td>
<td width="98%">&nbsp;</td>
</tr>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

