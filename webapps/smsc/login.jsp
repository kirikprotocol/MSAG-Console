<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
FORM_URI = "j_security_check";
FORM_METHOD = "post";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%><%
%><%--@ include file="loginBody.jsp"--%><%
if(request.getParameter("loginError")!=null) { 
	%><span class=CF00>Login error</span><%
}%><table class=secRep>
<col align=right width=1%>
<col align=left width=99%>
<tr class=row0>
	<th>Login:</th>
	<td><input class=txtW name="j_username"></td>
</tr>
<tr class=row1>
	<th>Password:</th>
	<td><input class=txtW type=password name="j_password"></td>
</tr>
</table>
<div class=secButtons>
	<input type=submit name=jsubmit value="Login">
</div><%
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%><%
%><%@ include file="/WEB-INF/inc/code_footer.jsp"%>