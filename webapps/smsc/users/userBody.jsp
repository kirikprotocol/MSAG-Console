<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr><td colspan=2 class=secInner><div class=secView>User info</div></td></tr>
<%int rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>login:</th>
	<td><input class=txtW name=login value="<%=bean.getLogin()%>" <%=bean.isNew() ? "" : "readonly"%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>password:</th>
	<td><input class=txtW type=password name=password value=""></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>confirm password:</th>
	<td><input class=txtW type=password name=confirmPassword value=""></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>roles:</th>
	<td><input class=check type=checkbox name=roles id=role01 value="apply"    <%=bean.isUserInRole("apply")    ? "checked" : ""%>>&nbsp;<label for=role1>Apply config changes</label><br>
		<input class=check type=checkbox name=roles id=role02 value="perfmon"  <%=bean.isUserInRole("perfmon")  ? "checked" : ""%>>&nbsp;<label for=role2>Perfomance monitoring</label><br>
		<input class=check type=checkbox name=roles id=role03 value="smsView"  <%=bean.isUserInRole("smsView")  ? "checked" : ""%>>&nbsp;<label for=role2>SMS View</label><br>
		<input class=check type=checkbox name=roles id=role04 value="users"    <%=bean.isUserInRole("users")    ? "checked" : ""%>>&nbsp;<label for=role3>Users</label><br>
		<input class=check type=checkbox name=roles id=role05 value="routes"   <%=bean.isUserInRole("routes")   ? "checked" : ""%>>&nbsp;<label for=role4>Routes</label><br>
		<input class=check type=checkbox name=roles id=role06 value="subjects" <%=bean.isUserInRole("subjects") ? "checked" : ""%>>&nbsp;<label for=role5>Subjects</label><br>
		<input class=check type=checkbox name=roles id=role07 value="aliases"  <%=bean.isUserInRole("aliases")  ? "checked" : ""%>>&nbsp;<label for=role6>Aliases</label><br>
		<input class=check type=checkbox name=roles id=role08 value="profiles" <%=bean.isUserInRole("profiles") ? "checked" : ""%>>&nbsp;<label for=role7>Profiles</label><br>
		<input class=check type=checkbox name=roles id=role09 value="hosts"    <%=bean.isUserInRole("hosts")    ? "checked" : ""%>>&nbsp;<label for=role8>Hosts</label><br>
		<input class=check type=checkbox name=roles id=role10 value="services" <%=bean.isUserInRole("services") ? "checked" : ""%>>&nbsp;<label for=role9>Services</label>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>first:</th>
	<td><input class=txtW name=firstName value="<%=bean.getFirstName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>last name:</th>
	<td><input class=txtW name=lastName value="<%=bean.getLastName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>dept:</th>
	<td><input class=txtW name=dept value="<%=bean.getDept()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>work phone:</th>
	<td><input class=txtW name=workPhone value="<%=bean.getWorkPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>home phone:</th>
	<td><input class=txtW name=homePhone value="<%=bean.getHomePhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>cellular phone:</th>
	<td><input class=txtW name=cellPhone value="<%=bean.getCellPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>email:</th>
	<td><input class=txtW name=email value="<%=bean.getEmail()%>" validation="email" onkeyup="resetValidation(this)"></td>
</tr>
</table>