<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.mcisme.beans.Options,
		java.util.Iterator, java.util.Collection,
		ru.novosoft.smsc.util.StringEncoderDecoder, 
		ru.novosoft.smsc.mcisme.beans.MCISmeBean"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.mcisme.beans.Options" />
<jsp:setProperty name="bean" property="*"/>
<%
	TITLE=getLocString("mcisme.title");
	MENU0_SELECTION = "MENU0_SERVICES";
    int beanResult = bean.process(request);
%><%@ include file="inc/menu_switch.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="inc/header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<div class=content>
<input type=hidden name=initialized value=true>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.gen_options")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<% int rowN = 0;%>
<tr class=row<%=rowN++&1%>>
  <th>Service address</th>
  <td><input class=txt name=address value="<%=StringEncoderDecoder.encode(bean.getAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Redirection address</th>
  <td><input class=txt name=redirectionAddress value="<%=StringEncoderDecoder.encode(bean.getRedirectionAddress())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Service type</th>
  <td><input class=txt name=svcType value="<%=StringEncoderDecoder.encode(bean.getSvcType())%>" maxlength="5"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Protocol ID</th>
  <td><input class=txt name=protocolId value="<%=StringEncoderDecoder.encode(bean.getProtocolId())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Validity period (days)</th>
  <td><input class=txt name=daysValid value="<%=StringEncoderDecoder.encode(bean.getDaysValid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Calling mask (regexp)</th>
  <td><input class=txt name=callingMask value="<%=StringEncoderDecoder.encode(bean.getCallingMask())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Called mask (regexp)</th>
  <td><input class=txt name=calledMask value="<%=StringEncoderDecoder.encode(bean.getCalledMask())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>PDU dispatchers count</th>
  <td><input class=txt name=pduDispatchersCount value="<%=StringEncoderDecoder.encode(bean.getPduDispatchersCount())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Max DataSm registry size</th>
  <td><input class=txt name=maxDataSmRegistrySize value="<%=StringEncoderDecoder.encode(bean.getMaxDataSmRegistrySize())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages max</th>
  <td><input class=txt name=unrespondedMessagesMax value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unresponded messages sleep</th>
  <td><input class=txt name=unrespondedMessagesSleep value="<%=StringEncoderDecoder.encode(bean.getUnrespondedMessagesSleep())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Outgoing speed max</th>
  <td><input class=txt name=outgoingSpeedMax value="<%=StringEncoderDecoder.encode(bean.getOutgoingSpeedMax())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Response wait time</th>
  <td><input class=txt name=responceWaitTime value="<%=StringEncoderDecoder.encode(bean.getResponceWaitTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Input queue size</th>
  <td><input class=txt name=inputQueueSize value="<%=StringEncoderDecoder.encode(bean.getInputQueueSize())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Inform abonents</th>
  <td>
    <label for=informDefault>On in default profile</label>
    <input class=check type=checkbox name=defaultInform id=informDefault value=true <%=bean.isDefaultInform() ? "checked" : ""%>>
    <label for=informForce>&nbsp;&nbsp;Force</label>
    <input class=check type=checkbox name=forceInform id=informForce value=true <%=bean.isForceInform() ? "checked" : ""%>>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Notify callers</th>
  <td>
    <label for=notifyDefault>On in default profile</label>
    <input class=check type=checkbox name=defaultNotify id=notifyDefault value=true <%=bean.isDefaultNotify() ? "checked" : ""%>>
    <label for=wantNotifyDefault>Caller want notify</label>
    <input class=check type=checkbox name=defaultWantNotifyMe id=wantNotifyDefault value=true <%=bean.isDefaultWantNotifyMe() ? "checked" : ""%>>
    <label for=wantNotifyUse>Want notify policy enable</label>
    <input class=check type=checkbox name=useWantNotifyPolicy id=wantNotifyUse value=true <%=bean.isUseWantNotifyPolicy() ? "checked" : ""%>>
    <label for=informForce>&nbsp;&nbsp;Force</label>
    <input class=check type=checkbox name=forceNotify id=notifyForce value=true <%=bean.isForceNotify() ? "checked" : ""%>>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Use abonent address as source</th>
  <td>
    <input class=check type=checkbox name=groupSmsByCallingAbonent id=groupSmsByCallingAbonent value=true <%=bean.isGroupSmsByCallingAbonent() ? "checked" : ""%>>
  </td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin host</th>
  <td><input class=txt name=adminHost value="<%=StringEncoderDecoder.encode(bean.getAdminHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Admin port</th>
  <td><input class=txt name=adminPort value="<%=StringEncoderDecoder.encode(bean.getAdminPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC host</th>
  <td><input class=txt name=smscHost value="<%=StringEncoderDecoder.encode(bean.getSmscHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC port</th>
  <td><input class=txt name=smscPort value="<%=StringEncoderDecoder.encode(bean.getSmscPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC SID</th>
  <td><input class=txt name=smscSid value="<%=StringEncoderDecoder.encode(bean.getSmscSid())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection timeout</th>
  <td><input class=txt name=smscTimeout value="<%=StringEncoderDecoder.encode(bean.getSmscTimeout())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>SMSC connection password</th>
  <td><input class=txt name=smscPassword value="<%=StringEncoderDecoder.encode(bean.getSmscPassword())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Country code</th>
  <td><input class=txt name=countryCode value="<%=StringEncoderDecoder.encode(bean.getCountryCode())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Time zone file location</th>
  <td><input class=txt name=timeZoneFileLocation value="<%=StringEncoderDecoder.encode(bean.getTimeZoneFileLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Routes file location</th>
  <td><input class=txt name=routesFileLocation value="<%=StringEncoderDecoder.encode(bean.getRoutesFileLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Schedule delay on alert notification</th>
  <td><input class=txt name=schedDelay value="<%=StringEncoderDecoder.encode(bean.getSchedDelay())%>"></td>
</tr>
</table>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.rr_settings")%></div>
<table class=properties_list cellspacing=0 width="100%" <%rowN=0;%>>
<col width="40%">
<col width="60%">
<tr class=row<%=rowN++&1%>>
  <th><label for=skipCaller>Skip calls from unknown abonents</label></th>
  <td><input class=check type=checkbox name=skipUnknownCaller id=skipCaller value=true <%=bean.isSkipUnknownCaller() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Release calls strategy</th>
  <td>
  <select name=releaseStrategy>
    <option value="<%= MCISmeBean.RELEASE_REDIRECT_STRATEGY%>" <%=
    (bean.getReleaseStrategyInt() == MCISmeBean.RELEASE_REDIRECT_STRATEGY) ? "selected":""%>>REDIRECT CALLS ON MSC</option>
    <option value="<%= MCISmeBean.RELEASE_PREFIXED_STRATEGY%>" <%=
    (bean.getReleaseStrategyInt() == MCISmeBean.RELEASE_PREFIXED_STRATEGY) ? "selected":""%>>USE PREFIXED B-NUMBERS</option>
    <option value="<%= MCISmeBean.RELEASE_MIXED_STRATEGY   %>" <%=
    (bean.getReleaseStrategyInt() == MCISmeBean.RELEASE_MIXED_STRATEGY   ) ? "selected":""%>>USE REDIRECT AND B-NUMBERS</option>
    <option value="<%= MCISmeBean.RELEASE_REDIRECT_RULES_STRATEGY %>" <%=
    (bean.getReleaseStrategyInt() == MCISmeBean.RELEASE_REDIRECT_RULES_STRATEGY) ? "selected":""%>>USE REDIRECT RULES</option>
  </select>
  </td>
</tr>
</table>
<table class=properties_list cellspacing=0  width="100%">
<col width="38%">
<col width="1%">
<col width="1%">
<col width="60%">
<tr class=row<%=rowN++&1%>>
  <th>Reason</th>
  <th>Responce code</th>
  <th>Inform</th>
  <th>Default profile</th>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Busy</th>
  <td><input class=txt name=causeBusy value="<%=StringEncoderDecoder.encode(bean.getCauseBusy())%>" validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informBusy value=true <%=bean.isInformBusy() ? "checked" : ""%>></td>
  <td><input class=check type=checkbox name=defaultBusy value=true <%=bean.isDefaultBusy() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>No Reply</th>
  <td><input class=txt name=causeNoReply value="<%=StringEncoderDecoder.encode(bean.getCauseNoReply())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informNoReply value=true <%=bean.isInformNoReply() ? "checked" : ""%>></td>
  <td><input class=check type=checkbox name=defaultNoReply value=true <%=bean.isDefaultNoReply() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Unconditional</th>
  <td><input class=txt name=causeUnconditional value="<%=StringEncoderDecoder.encode(bean.getCauseUnconditional())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informUnconditional value=true <%=bean.isInformUnconditional() ? "checked" : ""%>></td>
  <td><input class=check type=checkbox name=defaultUnconditional value=true <%=bean.isDefaultUnconditional() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Absent</th>
  <td><input class=txt name=causeAbsent value="<%=StringEncoderDecoder.encode(bean.getCauseAbsent())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informAbsent value=true <%=bean.isInformAbsent() ? "checked" : ""%>></td>
  <td><input class=check type=checkbox name=defaultAbsent value=true <%=bean.isDefaultAbsent() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Detach</th>
  <td><input class=txt name=causeDetach value="<%=StringEncoderDecoder.encode(bean.getCauseDetach())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informDetach value=true <%=bean.isInformDetach() ? "checked" : ""%>></td>
  <td><input class=check type=checkbox name=defaultDetach value=true <%=bean.isDefaultDetach() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Other</th>
  <td><input class=txt name=causeOther value="<%=StringEncoderDecoder.encode(bean.getCauseOther())%>"  validation="release_cause" onkeyup="resetValidation(this)"></td>
  <td><input class=check type=checkbox name=informOther value=true <%=bean.isInformOther() ? "checked" : ""%>></td>
  <td>&nbsp;</td>
</tr>
</table>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.storage")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Storage path</th>
  <td><input class=txt name=eventStorageLocation value="<%=StringEncoderDecoder.encode(bean.getEventStorageLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Event life time</th>
  <td><input class=txt name=eventLifeTime value="<%=StringEncoderDecoder.encode(bean.getEventLifeTime())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Max evenets</th>
  <td><input class=txt name=maxEvents value="<%=StringEncoderDecoder.encode(bean.getMaxEvents())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Events registration policy</th>
  <td><input class=txt name=eventPolicyRegistration value="<%=StringEncoderDecoder.encode(bean.getEventPolicyRegistration())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Increment BD Files</th>
  <td><input class=txt name=bdFilesIncr value="<%=StringEncoderDecoder.encode(bean.getBdFilesIncr())%>"></td>
</tr>
</table>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.prof_stor")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Storage path</th>
  <td><input class=txt name=profStorageLocation value="<%=StringEncoderDecoder.encode(bean.getProfStorageLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Storage host</th>
  <td><input class=txt name=profStorageHost value="<%=StringEncoderDecoder.encode(bean.getProfStorageHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Storage port</th>
  <td><input class=txt name=profStoragePort value="<%=StringEncoderDecoder.encode(bean.getProfStoragePort())%>"></td>
</tr>
</table>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.statistics")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Directory</th>
  <td><input class=txt name=statDir value="<%=StringEncoderDecoder.encode(bean.getStatDir())%>"></td>
</tr>
</table>
<div class=page_subtitle><%= getLocString("mcisme.subtitle.scheduling")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Resending period</th>
  <td><input class=txt name=resendingPeriod value="<%=StringEncoderDecoder.encode(bean.getResendingPeriod())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Scheduling on busy</th>
  <td><input class=txt name=schedOnBusy value="<%=StringEncoderDecoder.encode(bean.getSchedOnBusy())%>"></td>
</tr>
</table>
<script type="text/javascript">
function clickAddErrorTimeout()
{
	var errElem   = document.getElementById('sched_new_err');
	var timeoutElem = document.getElementById('sched_new_timeout');
	var newRow = sched_table_id.insertRow(sched_table_id.rows.length-1);
	newRow.className = "row" + (sched_table_id.rows.length & 1);
	newRow.id = "sched_row_" + errElem.value;
	var newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txt name=\"err.to." + errElem.value +"\" value=\"" + errElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<input class=txtW name=\"to." + errElem.value + "\" value=\"" + timeoutElem.value + "\">";
	newRow.appendChild(newCell);
	newCell = document.createElement("td");
	newCell.innerHTML = "<img src=\"/images/but_del.gif\" class=button jbuttonName=\"mbRemove\" jbuttonValue=\"common.buttons.remove\" title=\"mcisme.hint.remove_ds_driver\" jbuttonOnclick=\"return clickRemoveErrorTimeout('" + newRow.id + "');\">";
	newRow.appendChild(newCell);

	errElem.value = "";
	timeoutElem.value = "";

	document.getElementById("").value = document.getElementById("").value + 1;

	return false;
}
function clickRemoveErrorTimeout(id_to_remove)
{
	var rowElem = sched_table_id.rows[id_to_remove];
	sched_table_id.deleteRow(rowElem.rowIndex);

	return false;
}
</script>
<table class=properties_list cellspacing=0 id=sched_table_id>
<col width=20%>
<col width=20%>
<col width=20%>
<tr>
	<th>Error Code</th>
	<th colspan=2>Resending Period</th>
</tr>

<%
	Collection errorNumbers = bean.getErrorNumbers();
	for (Iterator i = errorNumbers.iterator(); i.hasNext();)
	{
		String errorNumber = (String) i.next();
		final String onErrorTimout = bean.getOnErrorTimeout(errorNumber);
		%><tr class=row<%=(rowN++)&1%> id=sched_row_<%=StringEncoderDecoder.encode(errorNumber)%>>
			<td><input class=txt name="err.to.<%=errorNumber%>"    value="<%=errorNumber%>"></td>
			<td><input class=txtW name="to.<%=errorNumber%>" value="<%=onErrorTimout%>"></td>
			<td><%button(out, "/images/but_del.gif", "mbDel", "common.buttons.remove", "mcisme.hint.remove_error_timeout", "return clickRemoveErrorTimeout('sched_row_" + StringEncoderDecoder.encode(errorNumber) + "');");%></td>
		</tr><%

	}
%>
<tr id=sched_row_new class=row<%=(rowN+1)&1%>>
	<td><input class=txt  id="sched_new_err"   name="sched_new_err"  ></td>
	<td><input class=txtW id="sched_new_timeout" name="sched_new_timeout"></td>
	<td><%button(out, "/images/but_add.gif", "mbAdd", "common.buttons.add", "mcisme.hint.add_ds_driver", "return clickAddErrorTimeout();");%></td>
</tr>
</table>

<div class=page_subtitle><%= getLocString("mcisme.subtitle.advertising")%></div>
<table class=properties_list cellspacing=0 width="100%">
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Use advertising</th>
  <td><input class=check type=checkbox name=useAdvert value=true <%=bean.isUseAdvert() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Server</th>
  <td><input class=txt name=advertServer value="<%=StringEncoderDecoder.encode(bean.getAdvertServer())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Port</th>
  <td><input class=txt name=advertPort value="<%=StringEncoderDecoder.encode(bean.getAdvertPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>Timeout</th>
  <td><input class=txt name=advertTimeout value="<%=StringEncoderDecoder.encode(bean.getAdvertTimeout())%>"></td>
</tr>
</table>

<div class=page_subtitle><%= getLocString("mcisme.subtitle.mci_prof_options")%></div>
<table class=properties_list cellspacing=0  width="100%" <%rowN=0;%>>
<col width="20%">
<col width="80%">
<tr class=row<%=rowN++&1%>>
  <th>Config location</th>
  <td><input class=txt name=mciProfLocation value="<%=StringEncoderDecoder.encode(bean.getMciProfLocation())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MCISme address(es)</th>
  <td><input class=txt name=mciSmeAddresses value="<%=StringEncoderDecoder.encode(bean.getMciSmeAddresses())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>VoiceMail address(es)</th>
  <td><input class=txt name=voiceMailAddresses value="<%=StringEncoderDecoder.encode(bean.getVoiceMailAddresses())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MSC host</th>
  <td><input class=txt name=mciHost value="<%=StringEncoderDecoder.encode(bean.getMciHost())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MSC port</th>
  <td><input class=txt name=mciPort value="<%=StringEncoderDecoder.encode(bean.getMciPort())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MSC NVT IO DEVICE</th>
  <td><input class=txt name=mciNvtIODevice value="<%=StringEncoderDecoder.encode(bean.getMciNvtIODevice())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MSC usercode</th>
  <td><input class=txt name=mciUsercode value="<%=StringEncoderDecoder.encode(bean.getMciUsercode())%>"></td>
</tr>
<tr class=row<%=rowN++&1%>>
  <th>MSC userpassword</th>
  <td><input class=txt name=mciUserpassword value="<%=StringEncoderDecoder.encode(bean.getMciUserpassword())%>"></td>
</tr>
</table>
</div><%
page_menu_begin(out);
page_menu_button(session, out, "mbDone",   "common.buttons.done",   "mcisme.hint.done_edit");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "mcisme.hint.cancel_changes", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>