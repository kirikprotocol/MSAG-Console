<%@ page import="ru.novosoft.smsc.admin.profiler.SupportExtProfile"%>
<%@ page import="ru.novosoft.smsc.admin.region.Region" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%@ include file="/WEB-INF/inc/time.jsp"%>
<div class=content>
<div class=page_subtitle><%=getLocString("users.subTitle")%></div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%int rowN = 0;%>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("common.sortmodes.login")%>:</th>
    <td><input class=txt name=login value="<%=bean.getLogin()%>" <%=bean.isNew() ? "" : "readonly"%>></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.password")%>:</th>
    <td><input class=txt type=password name=password value=""></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.confirmPassword")%>:</th>
    <td><input class=txt type=password name=confirmPassword value=""></td>
</tr>

<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.roles")%></th>
    <td>
        <%
            //################################## roles #############################
            startSectionPre(out, "roles", "", false);
            startParams(out);
            paramCheck(out, "users.roles.apply", "roles", "role01", "apply", bean.isUserInRole("apply"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.smscServ", "roles", "role02", "smsc_service", bean.isUserInRole("smsc_service"), null);
              paramCheck(out, "users.roles.locales", "roles", "role03", "locale_resources", bean.isUserInRole("locale_resources"), null);
              paramCheck(out, "users.roles.dl", "roles", "role04", "dl", bean.isUserInRole("dl"), null);
              paramCheck(out, "users.roles.perfmon", "roles", "role05", "perfmon", bean.isUserInRole("perfmon"), null);
              paramCheck(out, "users.roles.topmon", "roles", "role06", "topmon", bean.isUserInRole("topmon"), null);
              paramCheck(out, "users.roles.smsView", "roles", "role07", "smsView", bean.isUserInRole("smsView"), null);
              paramCheck(out, "users.roles.smsViewArch", "roles", "role08", "smsView_archive", bean.isUserInRole("smsView_archive"), null);
              paramCheck(out, "users.roles.smsViewOper", "roles", "role09", "smsView_operative", bean.isUserInRole("smsView_operative"), null);
              paramCheck(out, "users.roles.smsViewSmsTextP2P", "roles", "role35", "smsView_smstext_p2p", bean.isUserInRole("smsView_smstext_p2p"), null);
              paramCheck(out, "users.roles.smsViewSmsTextContent", "roles", "role35", "smsView_smstext_content", bean.isUserInRole("smsView_smstext_content"), null);
              paramCheck(out, "users.roles.smsStat", "roles", "role10", "smsstat", bean.isUserInRole("smsstat"), null);
              paramCheck(out, "users.roles.mscman", "roles", "role11", "mscman", bean.isUserInRole("mscman"), null);
            }
            paramCheck(out, "users.roles.users", "roles", "role12", "users", bean.isUserInRole("users"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.acls", "roles", "role13", "acls", bean.isUserInRole("acls"), null);
              paramCheck(out, "users.roles.routes", "roles", "role14", "routes", bean.isUserInRole("routes"), null);
            }
            paramCheck(out, "users.roles.subjects", "roles", "role15", "subjects", bean.isUserInRole("subjects"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.aliases", "roles", "role16", "aliases", bean.isUserInRole("aliases"), null);
              paramCheck(out, "users.roles.profiles", "roles", "role17", "profiles", bean.isUserInRole("profiles"), null);
            }
            paramCheck(out, "users.roles.hosts", "roles", "role18", "hosts", bean.isUserInRole("hosts"), null);
            paramCheck(out, "users.roles.servOperator", "roles", "role19", "service_operator", bean.isUserInRole("service_operator"), null);
            paramCheck(out, "users.roles.services", "roles", "role20", "services", bean.isUserInRole("services"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.providers", "roles", "role21", "providers", bean.isUserInRole("providers"), null);
              paramCheck(out, "users.roles.categories", "roles", "role22", "categories", bean.isUserInRole("categories"), null);
            }
            paramCheck(out, "users.roles.infosme-admin", "roles", "role23", "infosme-admin", bean.isUserInRole("infosme-admin"), null);
            paramCheck(out, "users.roles.infosme-market", "roles", "role24", "infosme-market", bean.isUserInRole("infosme-market"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.dbsme-admin", "roles", "role25", "dbsme-admin", bean.isUserInRole("dbsme-admin"), null);
              paramCheck(out, "users.roles.mcisme-admin", "roles", "role26", "mcisme-admin", bean.isUserInRole("mcisme-admin"), null);
              paramCheck(out, "users.roles.mtsmsme-admin", "roles", "role27", "mtsmsme-admin", bean.isUserInRole("mtsmsme-admin"), null);
              paramCheck(out, "users.roles.closed_groups", "roles", "role29", "closed_groups", bean.isUserInRole("closed_groups"), null);
              paramCheck(out, "users.roles.timezones", "roles", "role30", "timezones", bean.isUserInRole("timezones"), null);
            }
            paramCheck(out, "users.roles.journal", "roles", "role28", "journal", bean.isUserInRole("journal"), null);
            paramCheck(out, "users.roles.regions", "roles", "role31", "regions", bean.isUserInRole("regions"), null);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
              paramCheck(out, "users.roles.emailsme", "roles", "role32", "emailsme", bean.isUserInRole("emailsme"), null);
              paramCheck(out, "users.roles.snmp", "roles", "role33", "snmp", bean.isUserInRole("snmp"), null);
              if (SupportExtProfile.enabled)
                paramCheck(out, "users.roles.smscenters", "roles", "role34", "smscenters", bean.isUserInRole("smscenters"), null);
              paramCheck(out, "users.roles.fraud", "roles", "role35", "fraud", bean.isUserInRole("fraud"), null);
            }
            paramCheck(out, "users.roles.smsquiz-admin", "roles", "role36", "smsquiz-admin", bean.isUserInRole("smsquiz-admin"), null);
            if (bean.getServiceRoles().size() > 0)
                for (Iterator i = bean.getServiceRoles().iterator(); i.hasNext();) {
                    String roleName = (String) i.next();
                    String roleNameEnc = StringEncoderDecoder.encode(roleName);
                    paramCheckPre(out, getLocString("users.roles.externalSme") + " " + StringEncoderDecoder.encode(bean.getServiceIdFromRole(roleName)), "roles", "role_check_" + roleNameEnc, roleNameEnc, bean.isUserInRole(roleName), null);
                }
            finishParams(out);
            finishSection(out);
        %>
    </td>
</tr>

<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.prefs")%></th>
    <td>
        <%
            //################################## roles #############################
            startSectionPre(out, "prefs", "", false);
            startParams(out);
            paramHidden(out, "prefsNames", "locale");
            param(out, "users.prefs.locale", "prefsValues", bean.getPref("locale"), null, null, false, "language");
            finishParams(out);
            if (Constants.instMode == Constants.INST_MODE_SMSC) {
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
            }
            if (Constants.instMode == Constants.INST_MODE_INFO_SME) {
              startParams(out);
              paramSelect(out, "users.prefs.timezone", "timezone", bean.getTimezones(), bean.getTimezone(), null, null, null);
              finishParams(out);
            }

            if (bean.isInfoSmeEmbeded()) {
              startSection(out, "prefs.infosme.regions", "users.prefs.infosme.regions", false);
              startParams(out);
              for (Iterator iter = bean.getRegions().iterator(); iter.hasNext();) {
                Region r = (Region)iter.next();
                param(out, r.getName(), "infosme.region." + r.getId(), bean.isInfoSmeRegionAllowed(String.valueOf(r.getId())), null);
              }
              finishParams(out);
              finishSection(out);

                    startSection(out, "infosme", "infosme.label.task_config", false);  %>
                    <table class=properties_list cellspacing=0>
                    <col width="1%">
                    <col width="99%">
                    <tr class=row<%=rowN++&1%>>
                      <th><label for=infosmeTransactionMode><%= getLocString("infosme.label.transaction_mode")%></label></th>
                      <td>
                        <input class=check type=checkbox id=infosmeTransactionMode name=infosmeTransactionMode <%=bean.isInfosmeTransactionMode() ? "checked" : ""%>>
                      </td>
                    </tr>

                    <tr class=row<%=rowN++&1%>>
                      <th><%= getLocString("infosme.label.active_period")%></th>
                      <td>
                        <input class=timeField id=infosmeActivePeriodStart name=infosmeActivePeriodStart value="<%=StringEncoderDecoder.encode(bean.getInfosmeActivePeriodStart())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(infosmeActivePeriodStart, false, true);">...</button>
                        &nbsp;<%= getLocString("infosme.label.active_period_to")%>&nbsp;
                        <input class=timeField id=infosmeActivePeriodEnd name=infosmeActivePeriodEnd value="<%=StringEncoderDecoder.encode(bean.getInfosmeActivePeriodEnd())%>" maxlength=20 style="z-index:22;"><button class=timeButton type=button onclick="return showTime(infosmeActivePeriodEnd, false, true);">...</button>
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%= getLocString("infosme.label.validity_period")%></th>
                      <td>
                        <input class=txt id=infosmeValidityPeriod name=infosmeValidityPeriod value="<%=StringEncoderDecoder.encode(bean.getInfosmeValidityPeriod())%>" maxlength=20 style="z-index:22;">
                       </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%= getLocString("infosme.label.active_weekdays")%></th>
                      <td>
                        <table>
                        <col width="1%"><col width="32%">
                        <col width="1%"><col width="32%">
                        <col width="1%"><col width="32%">
                        <tr>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Mon value=Mon <%=bean.isWeekDayActive("Mon") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Mon><%= getLocString("infosme.label.monday")%></label></td>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Thu value=Thu <%=bean.isWeekDayActive("Thu") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Thu><%= getLocString("infosme.label.thursday")%></label></td>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Sat value=Sat <%=bean.isWeekDayActive("Sat") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sat><%= getLocString("infosme.label.saturday")%></label></td>
                        <tr>
                        <tr>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Tue value=Tue <%=bean.isWeekDayActive("Tue") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Tue><%= getLocString("infosme.label.tuesday")%></label></td>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Fri value=Fri <%=bean.isWeekDayActive("Fri") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Fri><%= getLocString("infosme.label.friday")%></label></td>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Sun value=Sun <%=bean.isWeekDayActive("Sun") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Sun><%= getLocString("infosme.label.sunday")%></label></td>
                        <tr>
                        <tr>
                          <td style="border:none;"><input class=check type=checkbox name=infosmeActiveWeekDays id=activeWeekDays_Wed value=Wed <%=bean.isWeekDayActive("Wed") ? "checked" : ""%>></td><td style="border:none;"><label for=activeWeekDays_Wed><%= getLocString("infosme.label.wednesday")%></label></td>
                        <tr>
                        </table>
                      </td>
                    </tr>

                    <tr class=row<%=rowN++&1%>>
                      <th><label for=infosmeReplaceMessage><%= getLocString("infosme.label.replace_messages")%></label></th>
                      <td>
                        <input class=check type=checkbox id=infosmeReplaceMessage name=infosmeReplaceMessage <%=bean.isInfosmeReplaceMessage() ? "checked" : ""%> onClick="document.getElementById('infosmeSvcType').disabled = !this.checked;">
                        <input class=txt id=infosmeSvcType name=infosmeSvcType value="<%=StringEncoderDecoder.encode(bean.getInfosmeSvcType())%>" maxlength=5 validation="id" onkeyup="resetValidation(this)">
                        <script>document.getElementById('infosmeSvcType').disabled = !document.getElementById('infosmeReplaceMessage').checked;</script>
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><label for=infosmeTrackIntegrity><%= getLocString("infosme.label.integrity")%></label></th>
                      <td><input class=check type=checkbox id=infosmeTrackIntegrity name=infosmeTrackIntegrity <%=bean.isInfosmeTrackIntegrity() ? "checked" : ""%>>
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><label for=infosmeKeepHistory><%= getLocString("infosme.label.keep_history")%></label></th>
                      <td><input class=check type=checkbox id=infosmeKeepHistory name=infosmeKeepHistory <%=bean.isInfosmeKeepHistory() ? "checked" : ""%>>
                      </td>
                    </tr>

                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.cacheSize")%></th>
                      <td><input class=txt name=infosmeMessagesCacheSize value="<%=StringEncoderDecoder.encode(bean.getInfosmeMessagesCacheSize())%>" validation="positive">
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.cacheSleep")%></th>
                      <td>
                         <input class=txt name=infosmeMessagesCacheSleep value="<%=StringEncoderDecoder.encode(bean.getInfosmeMessagesCacheSleep())%>" validation="positive">secs
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.uncommitedGeneration")%></th>
                      <td><input class=txt name=infosmeUncommitedInGeneration value="<%=StringEncoderDecoder.encode(bean.getInfosmeUncommitedInGeneration())%>" validation="positive">
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.uncommitedProcess")%></th>
                      <td>
                          <input class=txt name=infosmeUncommitedInProcess value="<%=StringEncoderDecoder.encode(bean.getInfosmeUncommitedInProcess())%>" validation="positive">
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.priority")%></th>
                      <td>
                          <input class=txt name=infosmePriority value="<%=StringEncoderDecoder.encode(bean.getInfosmePriority())%>" validation="positive">
                      </td>
                    </tr>
                    <tr class=row<%=rowN++&1%>>
                      <th><%=getLocString("infosme.label.originating_address")%></th>
                      <td>
                          <input class=txt name=infosmeSourceAddress value="<%=StringEncoderDecoder.encode(bean.getInfosmeSourceAddress())%>">
                      </td>
                    </tr>
                    <%if(bean.isUssdPushFeature()){ %>
                      <tr class=row<%=rowN++&1%>>
                        <th><%=getLocString("infosme.label.delivery_mode")%></th>
                        <td>
                          <select id="deliveryMode" name="deliveryMode">
                            <option value="<%=0%>" <%=bean.getDeliveryMode() == 0 ? "selected" : ""%>> <%= getLocString("infosme.label.delivery_mode_sms")%></option>
                            <option value="<%=1%>" <%=bean.getDeliveryMode() == 1 ? "selected" : ""%>><%= getLocString("infosme.label.delivery_mode_ussd_push")%></option>
                            <option value="<%=2%>" <%=bean.getDeliveryMode() == 2 ? "selected" : ""%>><%= getLocString("infosme.label.delivery_mode_ussd_push_vlr")%></option>
                          </select>
                        </td>
                      </tr>
                    <%} %>
                    </table>

                    <%finishSection(out);

                    startSection(out, "infosmeArchive", "infosme.label.tasks_archive", false);  %>

                    <table class=properties_list cellspacing=0>
                      <col width="1%">
                      <col width="99%">
                      <tr class=row<%=rowN++&1%>>
                        <th><%=getLocString("infosme.label.forced_archiving")%></th>
                        <td>
                          <input class=check type=checkbox id=infosmeArchive name=infosmeArchive <%=bean.isInfosmeArchive() ? "checked" : ""%> onClick="document.getElementById('infosmeArchiveTimeout').disabled = !this.checked;">
                          <input class=txt id=infosmeArchiveTimeout name=infosmeArchiveTimeout value="<%=bean.getInfosmeArchiveTimeout()%>" maxlength="5" validation="positive" onkeyup="resetValidation(this)">
                          <script>document.getElementById('infosmeArchiveTimeout').disabled = !document.getElementById('infosmeArchive').checked;</script>
                        </td>
                      </tr>
                    </table>

                    <%finishSection(out);
                    }

          finishSection(out);
        %>
    </td>
</tr>

<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("common.sortmodes.firstName")%>:</th>
    <td><input class=txt name=firstName value="<%=bean.getFirstName()%>" validation="nonEmpty"
               onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("common.sortmodes.lastName")%>:</th>
    <td><input class=txt name=lastName value="<%=bean.getLastName()%>" validation="nonEmpty"
               onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("common.sortmodes.department")%>:</th>
    <td><input class=txt name=dept value="<%=bean.getDept()%>" validation="nonEmpty" onkeyup="resetValidation(this)">
    </td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.workPhone")%>:</th>
    <td><input class=txt name=workPhone value="<%=bean.getWorkPhone()%>" validation="nonEmpty"
               onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.homePhone")%>:</th>
    <td><input class=txt name=homePhone value="<%=bean.getHomePhone()%>" validation="nonEmpty"
               onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.cellularPhone")%>:</th>
    <td><input class=txt name=cellPhone value="<%=bean.getCellPhone()%>" validation="nonEmpty"
               onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++) & 1%>>
    <th><%=getLocString("users.email")%>:</th>
    <td><input class=txt name=email value="<%=bean.getEmail()%>" validation="email" onkeyup="resetValidation(this)">
    </td>
</tr>
</table>
</div>