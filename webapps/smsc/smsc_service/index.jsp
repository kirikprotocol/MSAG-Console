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
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:

		break;
	case Index.RESULT_ERROR:

		break;
	default:

		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
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
  //################################## admin #############################
   startSection(out, "admin", "Administration", false);
    startParams(out);
      param(out, "host", "admin.host", bean.getStringParam("admin.host"));
      param(out, "port", "admin.port", bean.getIntParam("admin.port"));
    finishParams(out);
  finishSection(out);
  //################################## core #############################
  startSection(out, "core", "Core", false);
    startParams(out);
      param(out, "state machines count",      "core.state_machines_count",   bean.getIntParam(   "core.state_machines_count"));
      param(out, "event queue limit",         "core.eventQueueLimit",        bean.getIntParam("core.eventQueueLimit"));
      param(out, "reschedule table",          "core.reschedule_table",       bean.getStringParam("core.reschedule_table"));
      param(out, "service center address",    "core.service_center_address", bean.getStringParam("core.service_center_address"));
      param(out, "USSD center address",       "core.ussd_center_address",    bean.getStringParam("core.ussd_center_address"));
      param(out, "USSD SSN",                  "core.ussd_ssn",               bean.getIntParam(   "core.ussd_ssn"));
      param(out, "system ID",                 "core.systemId",               bean.getStringParam("core.systemId"));
      param(out, "service type",              "core.service_type",           bean.getStringParam("core.service_type"));
      param(out, "protocol ID",               "core.protocol_id",            bean.getIntParam(   "core.protocol_id"));
      param(out, "locales",                   "core.locales",                bean.getStringParam("core.locales"));
      param(out, "default locale",            "core.default_locale",         bean.getStringParam("core.default_locale"));
      param(out, "merge timeout",             "core.mergeTimeout",           bean.getIntParam(   "core.mergeTimeout"));
      param(out, "SMS max valid time (secs)", "sms.max_valid_time",          bean.getIntParam("sms.max_valid_time"));
      param(out, "logger init file",          "logger.initFile",             bean.getStringParam("logger.initFile"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.perfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "core.performance", "Performance monitor", false);
      startParams(out);
        param(out, "host", "core.performance.host", bean.getStringParam("core.performance.host"));
        param(out, "port", "core.performance.port", bean.getIntParam(   "core.performance.port"));
      finishParams(out);
    finishSection(out);
  finishSection(out);
  //################################## traffic control #############################
  startSection(out, "trafficControl", "Traffic control", false);
    startParams(out);
      param(out, "max SMS per second",                       "trafficControl.maxSmsPerSecond",         bean.getIntParam("trafficControl.maxSmsPerSecond"));
      param(out, "shape time frame (secs)",                  "trafficControl.shapeTimeFrame",          bean.getIntParam("trafficControl.shapeTimeFrame"));
      param(out, "protect time frame (secs)",                "trafficControl.protectTimeFrame",        bean.getIntParam("trafficControl.protectTimeFrame"));
      param(out, "max unresponded delivers by SME",          "trafficControl.protectThreshold",        bean.getIntParam("trafficControl.protectThreshold"));
      param(out, "delivery speed sensor min limit",          "trafficControl.allowedDeliveryFailures", bean.getIntParam("trafficControl.allowedDeliveryFailures"));
      param(out, "speed sensor scheduler look ahead (secs)", "trafficControl.lookAheadTime",           bean.getIntParam("trafficControl.lookAheadTime"));
    finishParams(out);
  finishSection(out);
  //################################## MessageStore #############################
  startSection(out, "MessageStore", "Message store", false);
    startParams(out);
      param(out, "max tries count",         "MessageStore.maxTriesCount",   bean.getIntParam("MessageStore.maxTriesCount"));
      param(out, "billing directory",       "MessageStore.billingDir",      bean.getStringParam("MessageStore.billingDir"));
      param(out, "billing interval (secs)", "MessageStore.billingInterval", bean.getIntParam("MessageStore.billingInterval"));
      param(out, "archive directory",       "MessageStore.archiveDir",      bean.getStringParam("MessageStore.archiveDir"));
      param(out, "archive interval (secs)", "MessageStore.archiveInterval", bean.getIntParam("MessageStore.archiveInterval"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Storage ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.Storage", "Storage", false);
      startParams(out);
        param(out, "DB instance",      "MessageStore.Storage.dbInstance",     bean.getStringParam("MessageStore.Storage.dbInstance"));
        param(out, "DB user name",     "MessageStore.Storage.dbUserName",     bean.getStringParam("MessageStore.Storage.dbUserName"));
        param(out, "DB user password", "MessageStore.Storage.dbUserPassword", bean.getStringParam("MessageStore.Storage.dbUserPassword"));
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
  //################################## DataSource #############################
  startSection(out, "DataSource", "Data source", false);
    startParams(out);
      param(out, "type",              "DataSource.type",           bean.getStringParam("DataSource.type"));
      param(out, "connections count", "DataSource.connections",    bean.getIntParam(   "DataSource.connections"));
      param(out, "DB instance",       "DataSource.dbInstance",     bean.getStringParam("DataSource.dbInstance"));
      param(out, "DB user name",      "DataSource.dbUserName",     bean.getStringParam("DataSource.dbUserName"));
      param(out, "DB user password",  "DataSource.dbUserPassword", bean.getStringParam("DataSource.dbUserPassword"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ StartupLoader.DataSourceDrivers ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "StartupLoader.DataSourceDrivers", "Drivers", false);
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
  //################################## smpp #############################
  startSection(out, "smpp", "SMPP", false);
    startParams(out);
      param(out, "host",                      "smpp.host",              bean.getStringParam("smpp.host"));
      param(out, "port",                      "smpp.port",              bean.getIntParam("smpp.port"));
      param(out, "read timeout (secs)",       "smpp.readTimeout",       bean.getIntParam("smpp.readTimeout"));
      param(out, "inactivity time (secs)",    "smpp.inactivityTime",    bean.getIntParam("smpp.inactivityTime"));
      param(out, "inactivity timeout (secs)", "smpp.inactivityTimeOut", bean.getIntParam("smpp.inactivityTimeOut"));
    finishParams(out);
  finishSection(out);
  //################################## profiler #############################
  List profilerReportOptions = new LinkedList();
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_None));
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_Final));
  profilerReportOptions.add(Profile.getReportOptionsString(Profile.REPORT_OPTION_Full));
  List profilerCodepages = new LinkedList();
  profilerCodepages.add(Profile.getCodepageString(Profile.CODEPAGE_Default));
  profilerCodepages.add(Profile.getCodepageString(Profile.CODEPAGE_Latin1));
  profilerCodepages.add(Profile.getCodepageString(Profile.CODEPAGE_UCS2));
  profilerCodepages.add(Profile.getCodepageString(Profile.CODEPAGE_UCS2AndLatin1));
  startSection(out, "profiler", "Profiler", false);
    startParams(out);
      param(out, "system ID",                                 "profiler.systemId",                bean.getStringParam("profiler.systemId"));
      param(out, "service type",                              "profiler.service_type",            bean.getStringParam("profiler.service_type"));
      param(out, "protocol ID",                               "profiler.protocol_id",             bean.getIntParam(   "profiler.protocol_id"));
      paramSelect(out,"default data coding",                  "profiler.defaultDataCoding",       profilerCodepages,      bean.getStringParam("profiler.defaultDataCoding"));
      paramSelect(out,"default report",                       "profiler.defaultReport",           profilerReportOptions,  bean.getStringParam("profiler.defaultReport"));
      param(out, "default hide",                              "profiler.defaultHide",             bean.getBoolParam(  "profiler.defaultHide"));
      param(out, "default&nbsp;hide&nbsp;modifiable",         "profiler.defaultHideModifiable",   bean.getBoolParam(  "profiler.defaultHideModifiable"));
      param(out, "default&nbsp;divert&nbsp;modifiable",       "profiler.defaultDivertModifiable", bean.getBoolParam(  "profiler.defaultDivertModifiable"));
      param(out, "default&nbsp;USSD&nbsp;in&nbsp;7&nbsp;bit", "profiler.defaultUssdIn7Bit",       bean.getBoolParam(  "profiler.defaultUssdIn7Bit"));
      param(out, "default&nbsp;UDH&nbsp;concatenation",       "profiler.defaultUdhConcat",        bean.getBoolParam(  "profiler.defaultUdhConcat"));

    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.ussdOpsMapping ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "profiler.ussdOpsMapping", "USSD operations mapping", false);
      startParams(out);
        param(out, "report none",                               "profiler.ussdOpsMapping.REPORT NONE",         bean.getIntParam("profiler.ussdOpsMapping.REPORT NONE"));
        param(out, "report full",                               "profiler.ussdOpsMapping.REPORT FULL",         bean.getIntParam("profiler.ussdOpsMapping.REPORT FULL"));
        param(out, "report final",                              "profiler.ussdOpsMapping.REPORT FINAL",        bean.getIntParam("profiler.ussdOpsMapping.REPORT FINAL"));
        param(out, "locale RU_RU",                              "profiler.ussdOpsMapping.LOCALE RU_RU",        bean.getIntParam("profiler.ussdOpsMapping.LOCALE RU_RU"));
        param(out, "locale EN_EN",                              "profiler.ussdOpsMapping.LOCALE EN_EN",        bean.getIntParam("profiler.ussdOpsMapping.LOCALE EN_EN"));
        param(out, "default",                                   "profiler.ussdOpsMapping.DEFAULT",             bean.getIntParam("profiler.ussdOpsMapping.DEFAULT"));
        param(out, "UCS2",                                      "profiler.ussdOpsMapping.UCS2",                bean.getIntParam("profiler.ussdOpsMapping.UCS2"));
        param(out, "hide",                                      "profiler.ussdOpsMapping.HIDE",                bean.getIntParam("profiler.ussdOpsMapping.HIDE"));
        param(out, "unhide",                                    "profiler.ussdOpsMapping.UNHIDE",              bean.getIntParam("profiler.ussdOpsMapping.UNHIDE"));
        param(out, "USSD 7bit on",                              "profiler.ussdOpsMapping.USSD7BIT ON",         bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT ON"));
        param(out, "USSD 7bit off",                             "profiler.ussdOpsMapping.USSD7BIT OFF",        bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT OFF"));
        param(out, "divert on",                                 "profiler.ussdOpsMapping.DIVERT ON",           bean.getIntParam("profiler.ussdOpsMapping.DIVERT ON"));
        param(out, "divert off",                                "profiler.ussdOpsMapping.DIVERT OFF",          bean.getIntParam("profiler.ussdOpsMapping.DIVERT OFF"));
        param(out, "divert absent on",                          "profiler.ussdOpsMapping.DIVERT ABSENT ON",    bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT ON"));
        param(out, "divert absent off",                         "profiler.ussdOpsMapping.DIVERT ABSENT OFF",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT OFF"));
        param(out, "divert blocked on",                         "profiler.ussdOpsMapping.DIVERT BLOCKED ON",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED ON"));
        param(out, "divert blocked off",                        "profiler.ussdOpsMapping.DIVERT BLOCKED OFF",  bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED OFF"));
        param(out, "divert barred on",                          "profiler.ussdOpsMapping.DIVERT BARRED ON",    bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED ON"));
        param(out, "divert barred off",                         "profiler.ussdOpsMapping.DIVERT BARRED OFF",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED OFF"));
        param(out, "divert capacity on",                        "profiler.ussdOpsMapping.DIVERT CAPACITY ON",  bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY ON"));
        param(out, "divert capacity off",                       "profiler.ussdOpsMapping.DIVERT CAPACITY OFF", bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY OFF"));
        param(out, "divert to",                                 "profiler.ussdOpsMapping.DIVERT TO",           bean.getIntParam("profiler.ussdOpsMapping.DIVERT TO"));
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
      param(out, "SMPP access address",   "abonentinfo.smpp_access_address",   bean.getStringParam("abonentinfo.smpp_access_address"));
    finishParams(out);
  finishSection(out);
  //################################## MscManager #############################
  startSection(out, "MscManager", "MSC manager", false);
    startParams(out);
      param(out, "automatic registration", "MscManager.automaticRegistration", bean.getBoolParam("MscManager.automaticRegistration"));
      param(out, "failure limit",          "MscManager.failureLimit",          bean.getIntParam( "MscManager.failureLimit"));
    finishParams(out);
  finishSection(out);



	//################################## map #############################
/*	startSection(out, "map", "map", false);
		startParams(out);
			param(out, "hlr", "map.hlr", bean.getStringParam("map.hlr"));
			param(out, "vlr", "map.vlr", bean.getStringParam("map.vlr"));
		finishParams(out);
	finishSection(out);
*/
%></div><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save config");
page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>