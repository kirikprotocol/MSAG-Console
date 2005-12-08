<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("users.title");
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:

		break;
	case Index.RESULT_ERROR:

		break;
	case Index.RESULT_FILTER:
		response.sendRedirect("usersFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("usersAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("usersEdit.jsp?login="+URLEncoder.encode(bean.getEditUserLogin()));
		return;
default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "users.addUser",  "users.addUserHint");
page_menu_button(session, out, "mbDelete", "users.deleteUser", "users.deleteUserHint");
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editUserLogin>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function edit(name_to_edit)
{
	document.getElementById('jbutton').name = "mbEdit";
	opForm.editUserLogin.value = name_to_edit;
	opForm.submit();
	return false;
}
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
<table class=list cellspacing=0 width="100%">
<col width="1%">
<col width="25%" align=left>
<col width="25%" align=left>
<col width="25%" align=left>
<col width="25%" align=left>
<thead>
<tr>
	<th class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="javascript:setSort('login')" <%=bean.getSort().endsWith("login") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.loginHint")%>"><%=getLocString("common.sortmodes.login")%></a></th>
	<th><a href="javascript:setSort('firstName')" <%=bean.getSort().endsWith("firstName") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.firstNameHint")%>"><%=getLocString("common.sortmodes.firstName")%></a></th>
	<th><a href="javascript:setSort('lastName')" <%=bean.getSort().endsWith("lastName") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.lastNameHint")%>"><%=getLocString("common.sortmodes.lastName")%></a></th>
	<th><a href="javascript:setSort('dept')" <%=bean.getSort().endsWith("dept") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.departmentHint")%>"><%=getLocString("common.sortmodes.department")%></a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getUsers().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
String login = (String) item.getValue("login");
String encLogin = StringEncoderDecoder.encode(login);
String encFirstName = StringEncoderDecoder.encode((String) item.getValue("firstName"));
String encLastName = StringEncoderDecoder.encode((String) item.getValue("lastName"));
String encDept = StringEncoderDecoder.encode((String) item.getValue("dept"));
%>
<tr class=row<%=row&1%>>
	<td><input class=check type=checkbox name=checkedUserLogins value="<%=encLogin%>" <%=bean.isLoginChecked(login) ? "checked" : ""%>></td>
	<td><a href="javascript:edit('<%=encLogin%>')" title="<%=getLocString("users.editSubTitle")%>" ><%=encLogin%></a></td>
	<td><%=encFirstName%>&nbsp;</td>
	<td><%=encLastName%>&nbsp;</td>
	<td><%=encDept%>&nbsp;</td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbAdd",  "users.addUser",  "users.addUserHint");
page_menu_button(session, out, "mbDelete", "users.deleteUser", "users.deleteUserHint");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
