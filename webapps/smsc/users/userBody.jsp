<%@ page import="java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%><div class=content>
<div class=page_subtitle>User info</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%int rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th>login:</th>
	<td><input class=txt name=login value="<%=bean.getLogin()%>" <%=bean.isNew() ? "" : "readonly"%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>password:</th>
	<td><input class=txt type=password name=password value=""></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>confirm password:</th>
	<td><input class=txt type=password name=confirmPassword value=""></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>roles:</th>
	<td>
    <input class=check type=checkbox name=roles id=role01 value="apply"             <%=bean.isUserInRole("apply")             ? "checked" : ""%>>&nbsp;<label for=role01>Apply config changes</label><br>
		<input class=check type=checkbox name=roles id=role02 value="smsc_service"      <%=bean.isUserInRole("smsc_service")      ? "checked" : ""%>>&nbsp;<label for=role02>SMSC configuration</label><br>
		<input class=check type=checkbox name=roles id=role03 value="locale_resources"  <%=bean.isUserInRole("locale_resources")  ? "checked" : ""%>>&nbsp;<label for=role03>Locale resources uploading and viewing</label><br>
		<input class=check type=checkbox name=roles id=role04 value="dl"                <%=bean.isUserInRole("dl")                ? "checked" : ""%>>&nbsp;<label for=role04>Distribution Lists</label><br>
		<input class=check type=checkbox name=roles id=role05 value="perfmon"           <%=bean.isUserInRole("perfmon")           ? "checked" : ""%>>&nbsp;<label for=role05>Perfomance monitoring</label><br>
		<input class=check type=checkbox name=roles id=role06 value="topmon"            <%=bean.isUserInRole("topmon")            ? "checked" : ""%>>&nbsp;<label for=role06>Sme monitoring</label><br>
		<input class=check type=checkbox name=roles id=role07 value="smsView"           <%=bean.isUserInRole("smsView")           ? "checked" : ""%>>&nbsp;<label for=role07>SMS View</label><br>
		<input class=check type=checkbox name=roles id=role08 value="smsView_archive"   <%=bean.isUserInRole("smsView_archive")   ? "checked" : ""%>>&nbsp;<label for=role08>SMS View Archive only</label><br>
		<input class=check type=checkbox name=roles id=role09 value="smsView_operative" <%=bean.isUserInRole("smsView_operative") ? "checked" : ""%>>&nbsp;<label for=role09>SMS View Opertaive only</label><br>
		<input class=check type=checkbox name=roles id=role10 value="smsstat"           <%=bean.isUserInRole("smsstat")           ? "checked" : ""%>>&nbsp;<label for=role10>SMS Statistics</label><br>
		<input class=check type=checkbox name=roles id=role11 value="mscman"            <%=bean.isUserInRole("mscman")            ? "checked" : ""%>>&nbsp;<label for=role11>MSCs</label><br>
		<input class=check type=checkbox name=roles id=role12 value="users"             <%=bean.isUserInRole("users")             ? "checked" : ""%>>&nbsp;<label for=role12>Users</label><br>
		<input class=check type=checkbox name=roles id=role13 value="routes"            <%=bean.isUserInRole("routes")            ? "checked" : ""%>>&nbsp;<label for=role13>Routes</label><br>
		<input class=check type=checkbox name=roles id=role14 value="subjects"          <%=bean.isUserInRole("subjects")          ? "checked" : ""%>>&nbsp;<label for=role14>Subjects</label><br>
		<input class=check type=checkbox name=roles id=role15 value="aliases"           <%=bean.isUserInRole("aliases")           ? "checked" : ""%>>&nbsp;<label for=role15>Aliases</label><br>
		<input class=check type=checkbox name=roles id=role16 value="profiles"          <%=bean.isUserInRole("profiles")          ? "checked" : ""%>>&nbsp;<label for=role16>Profiles</label><br>
		<input class=check type=checkbox name=roles id=role17 value="hosts"             <%=bean.isUserInRole("hosts")             ? "checked" : ""%>>&nbsp;<label for=role17>Hosts</label><br>
		<input class=check type=checkbox name=roles id=role18 value="services"          <%=bean.isUserInRole("services")          ? "checked" : ""%>>&nbsp;<label for=role18>Services</label>
    <%
      if (bean.getServiceRoles().size() > 0)
        out.print("<br>");
      for (Iterator i = bean.getServiceRoles().iterator(); i.hasNext();) {
        String roleName = (String) i.next();
        String roleNameEnc = StringEncoderDecoder.encode(roleName);
        %><input class=check type=checkbox id="role_check_<%=roleNameEnc%>" name=roles value="<%=roleNameEnc%>" <%=bean.isUserInRole(roleName) ? "checked" : ""%>>&nbsp;<label for="role_check_<%=roleNameEnc%>">External SME "<%=StringEncoderDecoder.encode(bean.getServiceIdFromRole(roleName))%>"</label><%
        if (i.hasNext())
          out.print("<br>");
    }
	%>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>first:</th>
	<td><input class=txt name=firstName value="<%=bean.getFirstName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>last name:</th>
	<td><input class=txt name=lastName value="<%=bean.getLastName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>dept:</th>
	<td><input class=txt name=dept value="<%=bean.getDept()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>work phone:</th>
	<td><input class=txt name=workPhone value="<%=bean.getWorkPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>home phone:</th>
	<td><input class=txt name=homePhone value="<%=bean.getHomePhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>cellular phone:</th>
	<td><input class=txt name=cellPhone value="<%=bean.getCellPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>email:</th>
	<td><input class=txt name=email value="<%=bean.getEmail()%>" validation="email" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div>