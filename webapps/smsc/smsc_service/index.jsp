<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="/WEB-INF/inc/service_status.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
				 ru.novosoft.smsc.admin.Constants"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
FORM_METHOD = "POST";
TITLE = "SMSC";
switch(bean.process(appContext, errorMessages, request))
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
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SMSC";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
SMSC is <%=serviceStatus(Constants.SMSC_SME_ID, bean.getStatus(), "SMSC_STATUS_ELEM_ID")%>
<div class=secButtons>
<input class=btn type=submit id=mbStart name=mbStart value="Start" title="Start SMSC" <%=bean.getStatus() != ServiceInfo.STATUS_STOPPED ? "disabled" : ""%>>
<input class=btn type=submit id=mbStop name=mbStop value="Stop" title="Stop  SMSC" <%=bean.getStatus() != ServiceInfo.STATUS_RUNNING ? "disabled" : ""%>>
<script>
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.SMSC_STATUS_ELEM_ID.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.SMSC_STATUS_ELEM_ID.innerText != "running");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
</div>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<br><h1>SMSC Configuration</h1>
<script>
function showhide(elem)
{
	var st = elem.nextSibling.style;
	if (st.display != "none")
		st.display="none";
	else
		st.display = "";
}
</script>
<dl>
<dt class=secTitle onclick='showhide(this)'>Logger</dt>
<dd style="display:none">
<%int row = 0;%>
<table class=secRep cellspacing=1 width="100%">
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>init file:</th>
	<td><input class=txtW name="logger.initFile" value="<%=StringEncoderDecoder.encode(bean.getStringParam("logger.initFile"))%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>Administartion</dt>
<dd style="display:none">
<%row=0;%>
<table class=secRep cellspacing=1 width="100%">
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>host:</th>
	<td><input class=txtW name="admin.host" value="<%=StringEncoderDecoder.encode(bean.getStringParam("admin.host"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>port:</th>
	<td><input class=txtW name="admin.port" value="<%=bean.getIntParam("admin.port")%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>Message store</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>maxTriesCount:</th>
	<td><input class=txtW name="MessageStore.maxTriesCount" value="<%=bean.getIntParam("MessageStore.maxTriesCount")%>"></td>
</tr>
</table>
<dl>
<dt class=secTitle onclick='showhide(this)'>Storage</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbInstance:</th>
	<td><input class=txtW name="MessageStore.Storage.dbInstance" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Storage.dbInstance"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserName:</th>
	<td><input class=txtW name="MessageStore.Storage.dbUserName" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Storage.dbUserName"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserPassword:</th>
	<td><input class=txtW name="MessageStore.Storage.dbUserPassword" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Storage.dbUserPassword"))%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>Archive</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>enabled:</th>
	<td><input class=check type=checkbox name="MessageStore.Archive.enabled" <%=bean.getBoolParam("MessageStore.Archive.enabled") ? "checked" : ""%> value=true></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>interval:</th>
	<td><input class=txtW name="MessageStore.Archive.interval" value="<%=bean.getIntParam("MessageStore.Archive.interval")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>finalized:</th>
	<td><input class=txtW name="MessageStore.Archive.finalized" value="<%=bean.getIntParam("MessageStore.Archive.finalized")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>uncommited:</th>
	<td><input class=txtW name="MessageStore.Archive.uncommited" value="<%=bean.getIntParam("MessageStore.Archive.uncommited")%>"></td>
</tr>
</table>
<dl>
<dt class=secTitle onclick='showhide(this)'>Cleaner</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>enabled:</th>
	<td><input class=check type=checkbox name="MessageStore.Archive.Cleaner.enabled" <%=bean.getBoolParam("MessageStore.Archive.Cleaner.enabled") ? "checked" : ""%> value=true></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>interval:</th>
	<td><input class=txtW name="MessageStore.Archive.Cleaner.interval" value="<%=bean.getIntParam("MessageStore.Archive.Cleaner.interval")%>"></td>
</tr>
</table>
</dd>
</dl>
</dd>
<dt class=secTitle onclick='showhide(this)'>Billing</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbInstance:</th>
	<td><input class=txtW name="MessageStore.Billing.dbInstance" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Billing.dbInstance"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserName:</th>
	<td><input class=txtW name="MessageStore.Billing.dbUserName" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Billing.dbUserName"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserPassword:</th>
	<td><input class=txtW name="MessageStore.Billing.dbUserPassword" value="<%=StringEncoderDecoder.encode(bean.getStringParam("MessageStore.Billing.dbUserPassword"))%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>Cache</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>enabled:</th>
	<td><input class=check type=checkbox name="MessageStore.Cache.enabled" <%=bean.getBoolParam("MessageStore.Cache.enabled") ? "checked" : ""%> value=true></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>capacity:</th>
	<td><input class=txtW name="MessageStore.Cache.capacity" value="<%=bean.getIntParam("MessageStore.Cache.capacity")%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>Connections</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>max:</th>
	<td><input class=txtW name="MessageStore.Connections.max" value="<%=bean.getIntParam("MessageStore.Connections.max")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>init:</th>
	<td><input class=txtW name="MessageStore.Connections.init" value="<%=bean.getIntParam("MessageStore.Connections.init")%>"></td>
</tr>
</table>
</dd>
</dl>
</dd>
<dt class=secTitle onclick='showhide(this)'>map</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>hlr:</th>
	<td><input class=txtW name="map.hlr" value="<%=StringEncoderDecoder.encode(bean.getStringParam("map.hlr"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>vlr:</th>
	<td><input class=txtW name="map.vlr" value="<%=StringEncoderDecoder.encode(bean.getStringParam("map.vlr"))%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>smpp</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>host:</th>
	<td><input class=txtW name="smpp.host" value="<%=StringEncoderDecoder.encode(bean.getStringParam("smpp.host"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>port:</th>
	<td><input class=txtW name="smpp.port" value="<%=bean.getIntParam("smpp.port")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>readTimeout:</th>
	<td><input class=txtW name="smpp.readTimeout" value="<%=bean.getIntParam("smpp.readTimeout")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>inactivityTime:</th>
	<td><input class=txtW name="smpp.inactivityTime" value="<%=bean.getIntParam("smpp.inactivityTime")%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>core</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>state_machines_count:</th>
	<td><input class=txtW name="core.state_machines_count" value="<%=bean.getIntParam("core.state_machines_count")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>reschedule_table:</th>
	<td><input class=txtW name="core.reschedule_table" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.reschedule_table"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>receipt_delivered:</th>
	<td><input class=txtW name="core.receipt_delivered" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.receipt_delivered"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>receipt_failed:</th>
	<td><input class=txtW name="core.receipt_failed" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.receipt_failed"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>receipt_notify:</th>
	<td><input class=txtW name="core.receipt_notify" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.receipt_notify"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>service_center_address:</th>
	<td><input class=txtW name="core.service_center_address" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.service_center_address"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>systemId:</th>
	<td><input class=txtW name="core.systemId" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.systemId"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>service_type:</th>
	<td><input class=txtW name="core.service_type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.service_type"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>protocol_id:</th>
	<td><input class=txtW name="core.protocol_id" value="<%=bean.getIntParam("core.protocol_id")%>"></td>
</tr>
</table>
<dl>
<dt class=secTitle onclick='showhide(this)'>performance</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>host:</th>
	<td><input class=txtW name="core.performance.host" value="<%=StringEncoderDecoder.encode(bean.getStringParam("core.performance.host"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>port:</th>
	<td><input class=txtW name="core.performance.port" value="<%=bean.getIntParam("core.performance.port")%>"></td>
</tr>
</table>
</dd>
</dl>
</dd>
<dt class=secTitle onclick='showhide(this)'>Abonent Info</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>systemId:</th>
	<td><input class=txtW name="abonentinfo.systemId" value="<%=StringEncoderDecoder.encode(bean.getStringParam("abonentinfo.systemId"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>service_type:</th>
	<td><input class=txtW name="abonentinfo.service_type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("abonentinfo.service_type"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>protocol_id:</th>
	<td><input class=txtW name="abonentinfo.protocol_id" value="<%=bean.getIntParam("abonentinfo.protocol_id")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>mobile_access_address:</th>
	<td><input class=txtW name="abonentinfo.mobile_access_address" value="<%=bean.getStringParam("abonentinfo.mobile_access_address")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>smpp_access_address:</th>
	<td><input class=txtW name="abonentinfo.smpp_access_address" value="<%=bean.getStringParam("abonentinfo.smpp_access_address")%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>profiler</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>systemId:</th>
	<td><input class=txtW name="profiler.systemId" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.systemId"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>service_type:</th>
	<td><input class=txtW name="profiler.service_type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.service_type"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>protocol_id:</th>
	<td><input class=txtW name="profiler.protocol_id" value="<%=bean.getIntParam("profiler.protocol_id")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>defaultReport:</th>
	<td><input class=txtW name="profiler.defaultReport" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.defaultReport"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>defaultDataCoding:</th>
	<td><input class=txtW name="profiler.defaultDataCoding" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.defaultDataCoding"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>msgReportNone:</th>
	<td><input class=txtW name="profiler.msgReportNone" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.msgReportNone"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>msgReportFull:</th>
	<td><input class=txtW name="profiler.msgReportFull" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.msgReportFull"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>msgDataCodingDefault:</th>
	<td><input class=txtW name="profiler.msgDataCodingDefault" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.msgDataCodingDefault"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>msgDataCodingUCS2:</th>
	<td><input class=txtW name="profiler.msgDataCodingUCS2" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.msgDataCodingUCS2"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>msgError:</th>
	<td><input class=txtW name="profiler.msgError" value="<%=StringEncoderDecoder.encode(bean.getStringParam("profiler.msgError"))%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>sms</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>max_valid_time:</th>
	<td><input class=txtW name="sms.max_valid_time" value="<%=bean.getIntParam("sms.max_valid_time")%>"></td>
</tr>
</table>
</dd>
<dt class=secTitle onclick='showhide(this)'>StartupLoader</dt>
<dd style="display:none">
	<dl>
	<dt class=secTitle onclick='showhide(this)'>DataSourceDrivers</dt>
	<dd style="display:none">
	<%
		for (Iterator i = bean.getDatasourceDrivers().iterator(); i.hasNext();)
		{
			String name = (String) i.next();
			%>
		<dl>
		<dt class=secTitle onclick='showhide(this)'><%=name%></dt>
		<dd style="display:none">
			<table class=secRep cellspacing=1 width="100%">
			<%row=0;%>
			<col width=180px>
			<tr class=row<%=(row++)&1%>>
				<th class=label>type:</th>
				<td><input class=txtW name="StartupLoader.DataSourceDrivers.<%=StringEncoderDecoder.encode(name)%>.type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".type"))%>"></td>
			</tr>
			<tr class=row<%=(row++)&1%>>
				<th class=label>loadup:</th>
				<td><input class=txtW name="StartupLoader.DataSourceDrivers.<%=StringEncoderDecoder.encode(name)%>.loadup" value="<%=StringEncoderDecoder.encode(bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".loadup"))%>"></td>
			</tr>
			</table>
		</dd>
		</dl><%
		}%>
	</dd>
	</dl>
</dd>
<dt class=secTitle onclick='showhide(this)'>DataSource</dt>
<dd style="display:none">
<table class=secRep cellspacing=1 width="100%">
<%row=0;%>
<col width=180px>
<tr class=row<%=(row++)&1%>>
	<th class=label>type:</th>
	<td><input class=txtW name="DataSource.type" value="<%=StringEncoderDecoder.encode(bean.getStringParam("DataSource.type"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>connections:</th>
	<td><input class=txtW name="DataSource.connections" value="<%=bean.getIntParam("DataSource.connections")%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbInstance:</th>
	<td><input class=txtW name="DataSource.dbInstance" value="<%=StringEncoderDecoder.encode(bean.getStringParam("DataSource.dbInstance"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserName:</th>
	<td><input class=txtW name="DataSource.dbUserName" value="<%=StringEncoderDecoder.encode(bean.getStringParam("DataSource.dbUserName"))%>"></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th class=label>dbUserPassword:</th>
	<td><input class=txtW name="DataSource.dbUserPassword" value="<%=StringEncoderDecoder.encode(bean.getStringParam("DataSource.dbUserPassword"))%>"></td>
</tr>
</table>
</dd>
</dl>
<div class=secButtons>
<%
if (bean.getStatus() != ServiceInfo.STATUS_RUNNING)
{
%><input class=btn type=submit name=mbSave value="Save" title="Save config"><%
}
else
{
%><input class=btn type=submit name=mbSave value="Save" title="Save new config"><%
}%>
<input class=btn type=submit name=mbReset value="Reset" title="Reset" onclick='clickCancel()'>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

