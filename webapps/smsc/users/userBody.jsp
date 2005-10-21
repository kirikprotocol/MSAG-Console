<%@ page import="java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<div class=page_subtitle><%=getLocString("users.subTitle")%></div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%int rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("common.sortmodes.login")%>:</th>
	<td><input class=txt name=login value="<%=bean.getLogin()%>" <%=bean.isNew() ? "" : "readonly"%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.password")%>:</th>
	<td><input class=txt type=password name=password value=""></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.confirmPassword")%>:</th>
	<td><input class=txt type=password name=confirmPassword value=""></td>
</tr>

<tr class=row<%=(rowN++)&1%>>
  <th><%=getLocString("users.roles")%></th>
  <td>
<%
  //################################## roles #############################
  startSectionPre(out, "roles", "", false);
    startParams(out);
      paramCheck(out, "users.roles.apply",        "roles", "role01", "apply",             bean.isUserInRole("apply"), null);
      paramCheck(out, "users.roles.smscServ",     "roles", "role02", "smsc_service",      bean.isUserInRole("smsc_service"), null);
      paramCheck(out, "users.roles.locales",      "roles", "role03", "locale_resources",  bean.isUserInRole("locale_resources"), null);
      paramCheck(out, "users.roles.dl",           "roles", "role04", "dl",                bean.isUserInRole("dl"), null);
      paramCheck(out, "users.roles.perfmon",      "roles", "role05", "perfmon",           bean.isUserInRole("perfmon"), null);
      paramCheck(out, "users.roles.topmon",       "roles", "role06", "topmon",            bean.isUserInRole("topmon"), null);
      paramCheck(out, "users.roles.smsView",      "roles", "role07", "smsView",           bean.isUserInRole("smsView"), null);
      paramCheck(out, "users.roles.smsViewArch",  "roles", "role08", "smsView_archive",   bean.isUserInRole("smsView_archive"), null);
      paramCheck(out, "users.roles.smsViewOper",  "roles", "role09", "smsView_operative", bean.isUserInRole("smsView_operative"), null);
      paramCheck(out, "users.roles.smsStat",      "roles", "role10", "smsstat",           bean.isUserInRole("smsstat"), null);
      paramCheck(out, "users.roles.mscman",       "roles", "role11", "mscman",            bean.isUserInRole("mscman"), null);
      paramCheck(out, "users.roles.users",        "roles", "role12", "users",             bean.isUserInRole("users"), null);
      paramCheck(out, "users.roles.acls",         "roles", "role13", "acls",              bean.isUserInRole("acls"), null);
      paramCheck(out, "users.roles.routes",       "roles", "role14", "routes",            bean.isUserInRole("routes"), null);
      paramCheck(out, "users.roles.subjects",     "roles", "role15", "subjects",          bean.isUserInRole("subjects"), null);
      paramCheck(out, "users.roles.aliases",      "roles", "role16", "aliases",           bean.isUserInRole("aliases"), null);
      paramCheck(out, "users.roles.profiles",     "roles", "role17", "profiles",          bean.isUserInRole("profiles"), null);
      paramCheck(out, "users.roles.hosts",        "roles", "role18", "hosts",             bean.isUserInRole("hosts"), null);
      paramCheck(out, "users.roles.servOperator", "roles", "role19", "service_operator",  bean.isUserInRole("service_operator"), null);
      paramCheck(out, "users.roles.services",     "roles", "role20", "services",          bean.isUserInRole("services"), null);
      paramCheck(out, "users.roles.providers",    "roles", "role21", "providers",         bean.isUserInRole("providers"), null);
      paramCheck(out, "users.roles.categories",   "roles", "role22", "categories",        bean.isUserInRole("categories"), null);
      paramCheck(out, "users.roles.infosme-admin", "roles", "role23", "infosme-admin",    bean.isUserInRole("infosme-admin"), null);
      paramCheck(out, "users.roles.infosme-market","roles", "role24", "infosme-market",   bean.isUserInRole("infosme-market"), null);
      paramCheck(out, "users.roles.dbsme-admin",   "roles", "role25", "dbsme-admin",      bean.isUserInRole("dbsme-admin"), null);
      paramCheck(out, "users.roles.mcisme-admin",  "roles", "role26", "mcisme-admin",     bean.isUserInRole("mcisme-admin"), null);
      paramCheck(out, "users.roles.mtsmsme-admin", "roles", "role27", "mtsmsme-admin",    bean.isUserInRole("mtsmsme-admin"), null);

      if (bean.getServiceRoles().size() > 0)
      for (Iterator i = bean.getServiceRoles().iterator(); i.hasNext();)
      {
        String roleName = (String) i.next();
        String roleNameEnc = StringEncoderDecoder.encode(roleName);
        paramCheckPre(out, getLocString("users.roles.externalSme") + " " + StringEncoderDecoder.encode(bean.getServiceIdFromRole(roleName)), "roles", "role_check_"+roleNameEnc, roleNameEnc, bean.isUserInRole(roleName), null);
      }
    finishParams(out);
  finishSection(out);
%>
</td>
</tr>

<tr class=row<%=(rowN++)&1%>>
  <th><%=getLocString("users.prefs")%></th>
  <td>
<%
  //################################## roles #############################
  startSectionPre(out, "prefs", "", false);
    startParams(out);
        paramHidden(out, "prefsNames", "locale");
        param(out, "users.prefs.locale", "prefsValues", bean.getPref("locale"), null, null, false, "language");
    finishParams(out);
        startSection(out, "prefs.topmon", "users.prefs.topmon", false);
          startParams(out);
            paramHidden(out, "prefsNames", "topmon.graph.scale");
            param(out, "users.prefs.topmon.graphScale", "prefsValues", bean.getPref("topmon.graph.scale"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "topmon.graph.grid");
            param(out, "users.prefs.topmon.graphGrid", "prefsValues", bean.getPref("topmon.graph.grid"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "topmon.graph.higrid");
            param(out, "users.prefs.topmon.graphHiGrid", "prefsValues", bean.getPref("topmon.graph.higrid"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "topmon.graph.head");
            param(out, "users.prefs.topmon.graphHead", "prefsValues", bean.getPref("topmon.graph.head"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "topmon.max.speed");
            param(out, "users.prefs.topmon.maxSpeed", "prefsValues", bean.getPref("topmon.max.speed"), null, null, false, "positive");
          finishParams(out);
        finishSection(out);

        startSection(out, "prefs.perfmon", "users.prefs.perfmon", false);
          startParams(out);
            paramHidden(out, "prefsNames", "perfmon.pixPerSecond");
            param(out, "users.prefs.perfmon.pixPerSecond", "prefsValues", bean.getPref("perfmon.pixPerSecond"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "perfmon.scale");
            param(out, "users.prefs.perfmon.scale", "prefsValues", bean.getPref("perfmon.scale"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "perfmon.block");
            param(out, "users.prefs.perfmon.block", "prefsValues", bean.getPref("perfmon.block"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "perfmon.vLightGrid");
            param(out, "users.prefs.perfmon.vLightGrid", "prefsValues", bean.getPref("perfmon.vLightGrid"), null, null, false, "positive");
            paramHidden(out, "prefsNames", "perfmon.vMinuteGrid");
            param(out, "users.prefs.perfmon.vMinuteGrid", "prefsValues", bean.getPref("perfmon.vMinuteGrid"), null, null, false, "positive");
          finishParams(out);
        finishSection(out);
  finishSection(out);
%>
</td>
</tr>

<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("common.sortmodes.firstName")%>:</th>
	<td><input class=txt name=firstName value="<%=bean.getFirstName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("common.sortmodes.lastName")%>:</th>
	<td><input class=txt name=lastName value="<%=bean.getLastName()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("common.sortmodes.department")%>:</th>
	<td><input class=txt name=dept value="<%=bean.getDept()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.workPhone")%>:</th>
	<td><input class=txt name=workPhone value="<%=bean.getWorkPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.homePhone")%>:</th>
	<td><input class=txt name=homePhone value="<%=bean.getHomePhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.cellularPhone")%>:</th>
	<td><input class=txt name=cellPhone value="<%=bean.getCellPhone()%>" validation="nonEmpty" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("users.email")%>:</th>
	<td><input class=txt name=email value="<%=bean.getEmail()%>" validation="email" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div>