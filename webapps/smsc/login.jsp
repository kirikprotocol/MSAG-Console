<<<<<<< login.jsp
<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
FORM_URI = "j_security_check";
FORM_METHOD = "post";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><div class="content"><%
if(request.getParameter("loginError")!=null) { 
	%><span class="CF00">Login error</span><%
}%><table class="properties_list">
<col width="1%">
<col width="99%">
<tr class="row0">
	<th>Login:</th>
	<td><input class="txt" name="j_username"></td>
</tr>
<tr class="row0">
	<th>Password:</th>
	<td><input class="txt" type="password" name="j_password"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "jsubmit",  "Login",  "Login");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>
=======
<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
FORM_URI = "j_security_check";
FORM_METHOD = "post";
String error = null;
Object obj = request.getAttribute("appContext"); // todo returned appContext is null! And init errors aren't visible
if (obj != null) {error = ((SMSCAppContext) obj).getInitErrorCode();}
if (error != null) {errorMessages.add(new SMSCJspException(error, SMSCJspException.ERROR_CLASS_ERROR));}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><div class="content"><%
if(request.getParameter("loginError")!=null) {
	%><span class=CF00><%=getLocString("login.loginError")%></span><%
}%><table class="properties_list">
<col width="1%">
<col width="99%">
<tr class="row0">
	<th><%=getLocString("login.login")%></th>
	<td><input class="txt" name="j_username"></td>
</tr>
<tr class="row0">
	<th><%=getLocString("login.password")%></th>
	<td><input class="txt" type="password" name="j_password"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "jsubmit", "login.loginButton", "login.loginButtonHint");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>>>>>>>> 1.5
