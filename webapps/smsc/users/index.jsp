<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
				 ru.novosoft.smsc.admin.route.SourceList,
				 ru.novosoft.smsc.admin.route.DestinationList
"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.users.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Users";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
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
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_USERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add user",  "Add new user");
page_menu_button(out, "mbDelete", "Delete user(s)", "Delete selected user(s)");
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
	document.all.jbutton.name = "mbEdit";
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
	<th class=ico><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("login") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by login" onclick='return setSort("login")'>name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("firstName") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by login" onclick='return setSort("firstName")'>first name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("lastName") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by login" onclick='return setSort("lastName")'>last name</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("dept") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by login" onclick='return setSort("dept")'>department</a></th>
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
	<td><a href="#" title="Edit user" onClick='return edit("<%=encLogin%>")'><%=encLogin%></a></td>
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
page_menu_button(out, "mbAdd",  "Add user",  "Add new user");
page_menu_button(out, "mbDelete", "Delete user(s)", "Delete selected user(s)");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
