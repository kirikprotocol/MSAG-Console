<form action="j_security_check" method=post><%
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
</div>
</form>