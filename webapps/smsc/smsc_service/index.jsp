<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
				 ru.novosoft.smsc.admin.Constants,
					  java.io.IOException,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Iterator,
				  ru.novosoft.smsc.jsp.SMSCJspException,
				  ru.novosoft.smsc.jsp.SMSCErrors,
              java.util.Collection,
              java.util.List,
              java.util.LinkedList,
              ru.novosoft.smsc.admin.profiler.Profile"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Index"
/><jsp:setProperty name="bean" property="*"/><%
ServiceIDForShowStatus = Constants.SMSC_SME_ID;
FORM_METHOD = "POST";
TITLE = "SMSC";
MENU0_SELECTION = "MENU0_SMSC";
switch(bean.process(appContext, errorMessages, request, loginedUserPrincipal))
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
%><%@ 
include file="/WEB-INF/inc/html_3_header.jsp"%><%@ 
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save config");
page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
page_menu_space(out);
page_menu_button(out, "mbStart", "Start", "Start SMSC", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
page_menu_button(out, "mbStop",  "Stop",  "Stop SMSC", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
page_menu_end(out);
%><script>
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.SMSC_SME_ID%>.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.SMSC_SME_ID%>.innerText != "running");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc>SMSC Configuration</div>
<%
   //################################# logger #############################
	startSection(out, "logger", "Logger", false);
		startParams(out);
			param(out, "init file", "logger.initFile", bean.getStringParam("logger.initFile"));
		finishParams(out);
	finishSection(out);
	//################################## admin #############################
   startSection(out, "admin", "Administration", false);
		startParams(out);
			param(out, "host", "admin.host", bean.getStringParam("admin.host"));
			param(out, "port", "admin.port", bean.getIntParam("admin.port"));
		finishParams(out);
	finishSection(out);
	//################################## MessageStore #############################
	startSection(out, "MessageStore", "Message store", false);
		startParams(out);
			param(out, "max tries count", "MessageStore.maxTriesCount", bean.getIntParam("MessageStore.maxTriesCount"));
		finishParams(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Storage ~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "MessageStore.Storage", "Storage", false);
			startParams(out);
				param(out, "DB instance",      "MessageStore.Storage.dbInstance",     bean.getStringParam("MessageStore.Storage.dbInstance"));
				param(out, "DB user name",     "MessageStore.Storage.dbUserName",     bean.getStringParam("MessageStore.Storage.dbUserName"));
				param(out, "DB user password", "MessageStore.Storage.dbUserPassword", bean.getStringParam("MessageStore.Storage.dbUserPassword"));
			finishParams(out);
		finishSection(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Archive.Cleaner ~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "MessageStore.Cleaner", "Cleaner", false);
			startParams(out);
				param(out, "enabled",         "MessageStore.Cleaner.enabled",  bean.getBoolParam("MessageStore.Cleaner.enabled"));
				param(out, "age (days)",      "MessageStore.Cleaner.age",      bean.getIntParam( "MessageStore.Cleaner.age"));
				param(out, "awake (seconds)", "MessageStore.Cleaner.awake",    bean.getIntParam( "MessageStore.Cleaner.awake"));
				param(out, "interval",        "MessageStore.Cleaner.interval", bean.getIntParam( "MessageStore.Cleaner.interval"));
			finishParams(out);
		finishSection(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Cache ~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "MessageStore.Cache", "Cache", false);
			startParams(out);
				param(out, "enabled", "MessageStore.Cache.enabled", bean.getBoolParam("MessageStore.Cache.enabled"));
				param(out, "capacity", "MessageStore.Cache.capacity", bean.getIntParam("MessageStore.Cache.capacity"));
			finishParams(out);
		finishSection(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Connections ~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "MessageStore.Connections", "Connections", false);
			startParams(out);
				param(out, "maximum", "MessageStore.Connections.max", bean.getIntParam("MessageStore.Connections.max"));
				param(out, "initial", "MessageStore.Connections.init", bean.getIntParam("MessageStore.Connections.init"));
			finishParams(out);
		finishSection(out);
	finishSection(out);
	//################################## map #############################
/*	startSection(out, "map", "map", false);
		startParams(out);
			param(out, "hlr", "map.hlr", bean.getStringParam("map.hlr"));
			param(out, "vlr", "map.vlr", bean.getStringParam("map.vlr"));
		finishParams(out);
	finishSection(out);
*/	//################################## smpp #############################
	startSection(out, "smpp", "smpp", false);
		startParams(out);
			param(out, "host",               "smpp.host",              bean.getStringParam("smpp.host"));
			param(out, "port",               "smpp.port",              bean.getIntParam("smpp.port"));
			param(out, "read timeout",       "smpp.readTimeout",       bean.getIntParam("smpp.readTimeout"));
			param(out, "inactivity time",    "smpp.inactivityTime",    bean.getIntParam("smpp.inactivityTime"));
			param(out, "inactivity timeOut", "smpp.inactivityTimeOut", bean.getIntParam("smpp.inactivityTimeOut"));
		finishParams(out);
	finishSection(out);
	//################################## core #############################
	startSection(out, "core", "core", false);
		startParams(out);
			param(out, "state machines count",   "core.state_machines_count",   bean.getIntParam(   "core.state_machines_count"));
			param(out, "Event queue limit",       "core.eventQueueLimit",       bean.getIntParam("core.eventQueueLimit"));


			param(out, "reschedule table",       "core.reschedule_table",       bean.getStringParam("core.reschedule_table"));
			param(out, "service center address", "core.service_center_address", bean.getStringParam("core.service_center_address"));
			param(out, "system ID",              "core.systemId",               bean.getStringParam("core.systemId"));
			param(out, "service type",           "core.service_type",           bean.getStringParam("core.service_type"));
			param(out, "protocol ID",            "core.protocol_id",            bean.getIntParam(   "core.protocol_id"));
			param(out, "locales",                "core.locales",                bean.getStringParam("core.locales"));
			param(out, "default locale",         "core.default_locale",         bean.getStringParam("core.default_locale"));
		finishParams(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.perfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "core.performance", "performance", false);
			startParams(out);
				param(out, "host", "core.performance.host", bean.getStringParam("core.performance.host"));
				param(out, "port", "core.performance.port", bean.getIntParam(   "core.performance.port"));
			finishParams(out);
		finishSection(out);
	finishSection(out);
	//################################## abonentinfo #############################
	startSection(out, "abonentinfo", "Abonent info", false);
		startParams(out);
			param(out, "system ID",             "abonentinfo.systemId",              bean.getStringParam("abonentinfo.systemId"));
			param(out, "service type",          "abonentinfo.service_type",          bean.getStringParam("abonentinfo.service_type"));
			param(out, "protocol ID",           "abonentinfo.protocol_id",           bean.getIntParam(   "abonentinfo.protocol_id"));
			param(out, "mobile access address", "abonentinfo.mobile_access_address", bean.getStringParam("abonentinfo.mobile_access_address"));
			param(out, "smpp access address",   "abonentinfo.smpp_access_address",   bean.getStringParam("abonentinfo.smpp_access_address"));
		finishParams(out);
	finishSection(out);
	//################################## profiler #############################
  List profilerReportOptions = new LinkedList();
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_None));
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_Final));
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_Full));
	startSection(out, "profiler", "Profiler", false);
		startParams(out);
			param(out, "system ID",           "profiler.systemId",          bean.getStringParam("profiler.systemId"));
			param(out, "service type",        "profiler.service_type",      bean.getStringParam("profiler.service_type"));
			param(out, "protocol ID",         "profiler.protocol_id",       bean.getIntParam(   "profiler.protocol_id"));
      paramSelect(out,"default report", "profiler.defaultReport",     profilerReportOptions, bean.getStringParam("profiler.defaultReport"));
//			param(out, "default report",      "profiler.defaultReport",     bean.getStringParam("profiler.defaultReport"));
			param(out, "default data coding", "profiler.defaultDataCoding", bean.getStringParam("profiler.defaultDataCoding"));
			param(out, "default hide",        "profiler.defaultHide",       bean.getBoolParam(  "profiler.defaultHide"));
		finishParams(out);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.ussdOpsMapping ~~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "profiler.ussdOpsMapping", "USSD ops mapping", false);
			startParams(out);
				param(out, "report none",  "profiler.ussdOpsMapping.REPORT NONE",  bean.getIntParam("profiler.ussdOpsMapping.REPORT NONE"));
				param(out, "report full",  "profiler.ussdOpsMapping.REPORT FULL",  bean.getIntParam("profiler.ussdOpsMapping.REPORT FULL"));
				param(out, "report final", "profiler.ussdOpsMapping.REPORT FINAL", bean.getIntParam("profiler.ussdOpsMapping.REPORT FINAL"));
				param(out, "locale RU_RU", "profiler.ussdOpsMapping.LOCALE RU_RU", bean.getIntParam("profiler.ussdOpsMapping.LOCALE RU_RU"));
				param(out, "locale EN_EN", "profiler.ussdOpsMapping.LOCALE EN_EN", bean.getIntParam("profiler.ussdOpsMapping.LOCALE EN_EN"));
				param(out, "default",      "profiler.ussdOpsMapping.DEFAULT",      bean.getIntParam("profiler.ussdOpsMapping.DEFAULT"));
				param(out, "UCS2",         "profiler.ussdOpsMapping.UCS2",         bean.getIntParam("profiler.ussdOpsMapping.UCS2"));
				param(out, "hide",         "profiler.ussdOpsMapping.HIDE",         bean.getIntParam("profiler.ussdOpsMapping.HIDE"));
				param(out, "unhide",       "profiler.ussdOpsMapping.UNHIDE",       bean.getIntParam("profiler.ussdOpsMapping.UNHIDE"));
			finishParams(out);
		finishSection(out);
	finishSection(out);
	//################################## sms #############################
	startSection(out, "sms", "SMS", false);
		startParams(out);
			param(out, "max valid time", "sms.max_valid_time", bean.getIntParam("sms.max_valid_time"));
		finishParams(out);
	finishSection(out);
	//################################## StartupLoader #############################
	startSection(out, "StartupLoader", "Startup Loader", false);
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~ StartupLoader.DataSourceDrivers ~~~~~~~~~~~~~~~~~~~~~~~~~
		startSection(out, "StartupLoader.DataSourceDrivers", "Data Source Drivers", false);
			for (Iterator i = bean.getDatasourceDrivers().iterator(); i.hasNext();)
			{
				String name = (String) i.next();
				String encName = StringEncoderDecoder.encode(name);
				//---------------------------------- StartupLoader.DataSourceDrivers.%name% -----------------------------
				startSection(out, "StartupLoader.DataSourceDrivers." + encName, encName, false);
					startParams(out);
						param(out, "type",   "StartupLoader.DataSourceDrivers." + encName + ".type",   bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".type"));
						param(out, "loadup", "StartupLoader.DataSourceDrivers." + encName + ".loadup", bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".loadup"));
					finishParams(out);
				finishSection(out);
			}
		finishSection(out);
	finishSection(out);
	//################################## DataSource #############################
	startSection(out, "DataSource", "Data Source", false);
		startParams(out);
			param(out, "type",             "DataSource.type",           bean.getStringParam("DataSource.type"));
			param(out, "connections",      "DataSource.connections",    bean.getIntParam(   "DataSource.connections"));
			param(out, "DB instance",      "DataSource.dbInstance",     bean.getStringParam("DataSource.dbInstance"));
			param(out, "DB user name",     "DataSource.dbUserName",     bean.getStringParam("DataSource.dbUserName"));
			param(out, "DB user password", "DataSource.dbUserPassword", bean.getStringParam("DataSource.dbUserPassword"));
		finishParams(out);
	finishSection(out);
	//################################## MscManager #############################
	startSection(out, "MscManager", "MSC manager", false);
		startParams(out);
			param(out, "automatic registration", "MscManager.automaticRegistration", bean.getBoolParam("MscManager.automaticRegistration"));
			param(out, "failure limit",          "MscManager.failureLimit",          bean.getIntParam( "MscManager.failureLimit"));
		finishParams(out);
	finishSection(out);
	//################################## traffic control #############################

	startSection(out, "trafficControl", "Traffic Control", false);

		startParams(out);

			param(out, "Max SMS per second", "trafficControl.maxSmsPerSecond", bean.getIntParam("trafficControl.maxSmsPerSecond"));
			param(out, "Shape time frame (seconds)", "trafficControl.shapeTimeFrame", bean.getIntParam("trafficControl.shapeTimeFrame"));
			param(out, "Protect time frame (seconds)", "trafficControl.protectTimeFrame", bean.getIntParam("trafficControl.protectTimeFrame"));
			param(out, "Max unresponded delivers by SME", "trafficControl.protectThreshold", bean.getIntParam("trafficControl.protectThreshold"));
			param(out, "Delivery speed sensor min limit", "trafficControl.allowedDeliveryFailures", bean.getIntParam("trafficControl.allowedDeliveryFailures"));
			param(out, "Speed sensor scheduler look ahead (seconds)", "trafficControl.lookAheadTime", bean.getIntParam("trafficControl.lookAheadTime"));

		finishParams(out);

	finishSection(out);

%></div><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save config");
page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>