<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
FORM_URI = "j_security_check";
FORM_METHOD = "post";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><div class=content><%
if(request.getParameter("loginError")!=null) { 
	%><span class=CF00>Login error</span><%
}%><table class=properties_list>
<col width=1%>
<col width=99%>
<tr class=row0>
	<th>Login:</th>
	<td><input class=txt name="j_username"></td>
</tr>
<tr class=row1>
	<th>Password:</th>
	<td><input class=txt type=password name="j_password"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "jsubmit",  "Login",  "Login");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>