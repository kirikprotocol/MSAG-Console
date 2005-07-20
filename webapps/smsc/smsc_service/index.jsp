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
TITLE = getLocString("smsc.title");
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
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveConfig");
page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
page_menu_space(out);
page_menu_button(session, out, "mbStart", "common.buttons.start", "smsc.start", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsc.stop", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
page_menu_end(out);
%><script>
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.SMSC_SME_ID%>.innerText != "<%=getLocString("common.statuses.stopped")%>");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.SMSC_SME_ID%>.innerText != "<%=getLocString("common.statuses.running")%>");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc><%=getLocString("smsc.configuration")%></div>
<%
  //################################## admin #############################
   startSection(out, "admin", "smsc.admin", false);
    startParams(out);
      param(out, "common.util.host", "admin.host", bean.getStringParam("admin.host"));
      param(out, "common.util.port", "admin.port", bean.getIntParam("admin.port"));
    finishParams(out);
  finishSection(out);
  //################################## core #############################
  startSection(out, "core", "smsc.core", false);
    startParams(out);
      param(out, "smsc.core.stateCount", "core.state_machines_count",   bean.getIntParam(   "core.state_machines_count"));
      param(out, "smsc.core.queueLimit", "core.eventQueueLimit",        bean.getIntParam("core.eventQueueLimit"));
      param(out, "smsc.core.reschTable", "core.reschedule_table",       bean.getStringParam("core.reschedule_table"));
      param(out, "smsc.core.servAddr",   "core.service_center_address", bean.getStringParam("core.service_center_address"));
      param(out, "smsc.core.ussdAddr",   "core.ussd_center_address",    bean.getStringParam("core.ussd_center_address"));
      param(out, "smsc.core.ussdSSN",    "core.ussd_ssn",               bean.getIntParam(   "core.ussd_ssn"));
      param(out, "smsc.core.systemId",   "core.systemId",               bean.getStringParam("core.systemId"));
      param(out, "smsc.core.servType",   "core.service_type",           bean.getStringParam("core.service_type"));
      param(out, "smsc.core.protId",     "core.protocol_id",            bean.getIntParam(   "core.protocol_id"));
      param(out, "smsc.core.locales",    "core.locales",                bean.getStringParam("core.locales"));
      param(out, "smsc.core.defLocale",  "core.default_locale",         bean.getStringParam("core.default_locale"));
      param(out, "smsc.core.mergeTime",  "core.mergeTimeout",           bean.getIntParam(   "core.mergeTimeout"));
      param(out, "smsc.core.validTime",  "sms.max_valid_time",          bean.getIntParam("sms.max_valid_time"));
      param(out, "smsc.core.loggerFile", "logger.initFile",             bean.getStringParam("logger.initFile"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.perfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "core.performance", "smsc.core.perfmon", false);
      startParams(out);
        param(out, "common.util.host", "core.performance.host", bean.getStringParam("core.performance.host"));
        param(out, "common.util.port", "core.performance.port", bean.getIntParam(   "core.performance.port"));
      finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.smeperfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "core.smeperformance", "smsc.core.topmon", false);
      startParams(out);
        param(out, "common.util.host", "core.smeperformance.host", bean.getStringParam("core.smeperformance.host"));
        param(out, "common.util.port", "core.smeperformance.port", bean.getIntParam(   "core.smeperformance.port"));
      finishParams(out);
    finishSection(out);
  finishSection(out);
  //################################## map #############################
  startSection(out, "map", "smsc.map", false);
    startParams(out);
      param(out, "smsc.map.busyMTdelay", "map.busyMTDelay", bean.getIntParam("map.busyMTDelay"));
      param(out, "smsc.map.lockedMOdelay", "map.lockedByMODelay", bean.getIntParam("map.lockedByMODelay"));
      param(out, "smsc.map.MOlockTime", "map.MOLockTimeout", bean.getIntParam("map.MOLockTimeout"));
      param(out, "smsc.map.allowCallBarred", "map.allowCallBarred", bean.getBoolParam("map.allowCallBarred"));
    finishParams(out);
  finishSection(out);

  //################################## traffic control #############################
  startSection(out, "trafficControl", "smsc.trControl", false);
    startParams(out);
      param(out, "smsc.trControl.maxSmsPerSecond",         "trafficControl.maxSmsPerSecond",         bean.getIntParam("trafficControl.maxSmsPerSecond"));
      param(out, "smsc.trControl.shapeTimeFrame",          "trafficControl.shapeTimeFrame",          bean.getIntParam("trafficControl.shapeTimeFrame"));
      param(out, "smsc.trControl.startTimeFrame",          "trafficControl.startTimeFrame",          bean.getIntParam("trafficControl.startTimeFrame"));
      param(out, "smsc.trControl.protectTimeFrame",        "trafficControl.protectTimeFrame",        bean.getIntParam("trafficControl.protectTimeFrame"));
      param(out, "smsc.trControl.protectThreshold",        "trafficControl.protectThreshold",        bean.getIntParam("trafficControl.protectThreshold"));
      param(out, "smsc.trControl.allowedDeliveryFailures", "trafficControl.allowedDeliveryFailures", bean.getIntParam("trafficControl.allowedDeliveryFailures"));
      param(out, "smsc.trControl.lookAheadTime",           "trafficControl.lookAheadTime",           bean.getIntParam("trafficControl.lookAheadTime"));

    finishParams(out);
  finishSection(out);
  //################################## MessageStore #############################
  startSection(out, "MessageStore", "smsc.mesStor", false);
    startParams(out);
      param(out, "smsc.mesStor.maxTriesCount",   "MessageStore.maxTriesCount",   bean.getIntParam("MessageStore.maxTriesCount"));
      param(out, "smsc.mesStor.billingDir",      "MessageStore.billingDir",      bean.getStringParam("MessageStore.billingDir"));
      param(out, "smsc.mesStor.billingInterval", "MessageStore.billingInterval", bean.getIntParam("MessageStore.billingInterval"));
      param(out, "smsc.mesStor.archiveDir",      "MessageStore.archiveDir",      bean.getStringParam("MessageStore.archiveDir"));
      param(out, "smsc.mesStor.archiveInterval", "MessageStore.archiveInterval", bean.getIntParam("MessageStore.archiveInterval"));
      param(out, "smsc.mesStor.statDir",         "MessageStore.statisticsDir", bean.getStringParam("MessageStore.statisticsDir"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Storage ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.Storage", "smsc.mesStor.storage", false);
      startParams(out);
        param(out, "smsc.mesStor.storage.dbInstance",     "MessageStore.Storage.dbInstance",     bean.getStringParam("MessageStore.Storage.dbInstance"));
        param(out, "smsc.mesStor.storage.dbUserName",     "MessageStore.Storage.dbUserName",     bean.getStringParam("MessageStore.Storage.dbUserName"));
        param(out, "smsc.mesStor.storage.dbUserPassword", "MessageStore.Storage.dbUserPassword", bean.getStringParam("MessageStore.Storage.dbUserPassword"));
      finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Cache ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.Cache", "smsc.mesStor.cache", false);
      startParams(out);
        param(out, "smsc.mesStor.cache.enabled", "MessageStore.Cache.enabled", bean.getBoolParam("MessageStore.Cache.enabled"));
        param(out, "smsc.mesStor.cache.capacity", "MessageStore.Cache.capacity", bean.getIntParam("MessageStore.Cache.capacity"));
      finishParams(out);
    finishSection(out);
  //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.LocalStore ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.LocalStore", "smsc.mesStor.localStore", false);
      startParams(out);
        param(out, "smsc.mesStor.localStore.filename", "MessageStore.LocalStore.filename", bean.getStringParam("MessageStore.LocalStore.filename"));
        param(out, "smsc.mesStor.localStore.maxStoreSize", "MessageStore.LocalStore.maxStoreSize", bean.getIntParam("MessageStore.LocalStore.maxStoreSize"));
        param(out, "smsc.mesStor.localStore.minRollTime", "MessageStore.LocalStore.minRollTime", bean.getIntParam("MessageStore.LocalStore.minRollTime"));
        param(out, "smsc.mesStor.localStore.msgidfile", "MessageStore.LocalStore.msgidfile", bean.getStringParam("MessageStore.LocalStore.msgidfile"));
      finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.Connections ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.Connections", "smsc.mesStor.connections", false);
      startParams(out);
        param(out, "smsc.mesStor.connections.max", "MessageStore.Connections.max", bean.getIntParam("MessageStore.Connections.max"));
        param(out, "smsc.mesStor.connections.init", "MessageStore.Connections.init", bean.getIntParam("MessageStore.Connections.init"));
      finishParams(out);
    finishSection(out);
  finishSection(out);
  //################################## DataSource #############################
  startSection(out, "DataSource", "smsc.dataSource", false);
    startParams(out);
      param(out, "smsc.dataSource.type",           "DataSource.type",           bean.getStringParam("DataSource.type"));
      param(out, "smsc.dataSource.connections",    "DataSource.connections",    bean.getIntParam(   "DataSource.connections"));
      param(out, "smsc.dataSource.dbInstance",     "DataSource.dbInstance",     bean.getStringParam("DataSource.dbInstance"));
      param(out, "smsc.dataSource.dbUserName",     "DataSource.dbUserName",     bean.getStringParam("DataSource.dbUserName"));
      param(out, "smsc.dataSource.dbUserPassword", "DataSource.dbUserPassword", bean.getStringParam("DataSource.dbUserPassword"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ StartupLoader.DataSourceDrivers ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "StartupLoader.DataSourceDrivers", "smsc.dataSource.drivers", false);
      for (Iterator i = bean.getDatasourceDrivers().iterator(); i.hasNext();)
      {
        String name = (String) i.next();
        String encName = StringEncoderDecoder.encode(name);
        //---------------------------------- StartupLoader.DataSourceDrivers.%name% -----------------------------
        startSectionPre(out, "StartupLoader.DataSourceDrivers." + encName, encName, false);
          startParams(out);
            param(out, "smsc.dataSource.drivers.type",   "StartupLoader.DataSourceDrivers." + encName + ".type",   bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".type"));
            param(out, "smsc.dataSource.drivers.loadup", "StartupLoader.DataSourceDrivers." + encName + ".loadup", bean.getStringParam("StartupLoader.DataSourceDrivers." + name + ".loadup"));
          finishParams(out);
        finishSection(out);
      }
    finishSection(out);
  finishSection(out);
  //################################## smpp #############################
  startSection(out, "smpp", "smsc.smpp", false);
    startParams(out);
      param(out, "common.util.host",                  "smpp.host",                          bean.getStringParam("smpp.host"));
      param(out, "common.util.port",                  "smpp.port",                          bean.getIntParam("smpp.port"));
      param(out, "smsc.smpp.readTimeout",             "smpp.readTimeout",                   bean.getIntParam("smpp.readTimeout"));
      param(out, "smsc.smpp.inactivityTime",          "smpp.inactivityTime",                bean.getIntParam("smpp.inactivityTime"));
      param(out, "smsc.smpp.inactivityTimeOut",       "smpp.inactivityTimeOut",             bean.getIntParam("smpp.inactivityTimeOut"));
      param(out, "smsc.smpp.bindTimeout",             "smpp.bindTimeout",                   bean.getIntParam("smpp.bindTimeout"));
      param(out, "smsc.smpp.defaultConnectionsLimit", "smpp.defaultConnectionsLimit",       bean.getIntParam("smpp.defaultConnectionsLimit"));
      finishParams(out);
      startSection(out, "connectionsLimitsForIps",    "smsc.smpp.connectionsLimitsForIps", false);
      if ( bean.getConnectionsLimitsForIps().size()>0)
      { startParams(out);
    //---------------------------------- smpp.connectionsLimitsForIps.%name% -----------------------------
      for (Iterator i = bean.getConnectionsLimitsForIps().iterator(); i.hasNext();)
      {
            String name = (String) i.next();
            param(out, "smsc.smpp.connectionsLimitsForIps.Ip",   "smpp.connectionsLimitsForIps.Ip" ,   bean.getStringParam("smpp.connectionsLimitsForIps." + name ));
      }
        finishParams(out);
      }
      finishSection(out);
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
  startSection(out, "profiler", "smsc.profiler", false);
    startParams(out);
      param(out, "smsc.profiler.systemId",                "profiler.systemId",                bean.getStringParam("profiler.systemId"));
      param(out, "smsc.profiler.service_type",            "profiler.service_type",            bean.getStringParam("profiler.service_type"));
      param(out, "smsc.profiler.protocol_id",             "profiler.protocol_id",             bean.getIntParam(   "profiler.protocol_id"));
      paramSelect(out,"smsc.profiler.defaultDataCoding",  "profiler.defaultDataCoding",       profilerCodepages,      bean.getStringParam("profiler.defaultDataCoding"));
      paramSelect(out,"smsc.profiler.defaultReport",      "profiler.defaultReport",           profilerReportOptions,  bean.getStringParam("profiler.defaultReport"));
      param(out, "smsc.profiler.defaultHide",             "profiler.defaultHide",             bean.getBoolParam(  "profiler.defaultHide"));
      param(out, "smsc.profiler.defaultHideModifiable",   "profiler.defaultHideModifiable",   bean.getBoolParam(  "profiler.defaultHideModifiable"));
      param(out, "smsc.profiler.defaultDivertModifiable", "profiler.defaultDivertModifiable", bean.getBoolParam(  "profiler.defaultDivertModifiable"));
      param(out, "smsc.profiler.defaultUssdIn7Bit",       "profiler.defaultUssdIn7Bit",       bean.getBoolParam(  "profiler.defaultUssdIn7Bit"));
      param(out, "smsc.profiler.defaultUdhConcat",        "profiler.defaultUdhConcat",        bean.getBoolParam(  "profiler.defaultUdhConcat"));

    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.ussdOpsMapping ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "profiler.ussdOpsMapping", "smsc.profiler.ussdOpsMapping", false);
      startParams(out);
        param(out, "smsc.profiler.ussdOpsMapping.reportNone",        "profiler.ussdOpsMapping.REPORT NONE",         bean.getIntParam("profiler.ussdOpsMapping.REPORT NONE"));
        param(out, "smsc.profiler.ussdOpsMapping.reportFull",        "profiler.ussdOpsMapping.REPORT FULL",         bean.getIntParam("profiler.ussdOpsMapping.REPORT FULL"));
        param(out, "smsc.profiler.ussdOpsMapping.reportFinal",       "profiler.ussdOpsMapping.REPORT FINAL",        bean.getIntParam("profiler.ussdOpsMapping.REPORT FINAL"));
        param(out, "smsc.profiler.ussdOpsMapping.localeRuRu",        "profiler.ussdOpsMapping.LOCALE RU_RU",        bean.getIntParam("profiler.ussdOpsMapping.LOCALE RU_RU"));
        param(out, "smsc.profiler.ussdOpsMapping.localeEnEn",        "profiler.ussdOpsMapping.LOCALE EN_EN",        bean.getIntParam("profiler.ussdOpsMapping.LOCALE EN_EN"));
        param(out, "smsc.profiler.ussdOpsMapping.default",           "profiler.ussdOpsMapping.DEFAULT",             bean.getIntParam("profiler.ussdOpsMapping.DEFAULT"));
        param(out, "smsc.profiler.ussdOpsMapping.ucs2",              "profiler.ussdOpsMapping.UCS2",                bean.getIntParam("profiler.ussdOpsMapping.UCS2"));
        param(out, "smsc.profiler.ussdOpsMapping.hide",              "profiler.ussdOpsMapping.HIDE",                bean.getIntParam("profiler.ussdOpsMapping.HIDE"));
        param(out, "smsc.profiler.ussdOpsMapping.unhide",            "profiler.ussdOpsMapping.UNHIDE",              bean.getIntParam("profiler.ussdOpsMapping.UNHIDE"));
        param(out, "smsc.profiler.ussdOpsMapping.ussd7bitOn",        "profiler.ussdOpsMapping.USSD7BIT ON",         bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT ON"));
        param(out, "smsc.profiler.ussdOpsMapping.ussd7bitOff",       "profiler.ussdOpsMapping.USSD7BIT OFF",        bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertOn",          "profiler.ussdOpsMapping.DIVERT ON",           bean.getIntParam("profiler.ussdOpsMapping.DIVERT ON"));
        param(out, "smsc.profiler.ussdOpsMapping.divertOff",         "profiler.ussdOpsMapping.DIVERT OFF",          bean.getIntParam("profiler.ussdOpsMapping.DIVERT OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertAbsentOn",    "profiler.ussdOpsMapping.DIVERT ABSENT ON",    bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT ON"));
        param(out, "smsc.profiler.ussdOpsMapping.divertAbsentOff",   "profiler.ussdOpsMapping.DIVERT ABSENT OFF",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertBlockedOn",   "profiler.ussdOpsMapping.DIVERT BLOCKED ON",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED ON"));
        param(out, "smsc.profiler.ussdOpsMapping.divertBlockedOff",  "profiler.ussdOpsMapping.DIVERT BLOCKED OFF",  bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertBarredOn",    "profiler.ussdOpsMapping.DIVERT BARRED ON",    bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED ON"));
        param(out, "smsc.profiler.ussdOpsMapping.divertBarredOff",   "profiler.ussdOpsMapping.DIVERT BARRED OFF",   bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertCapacityOn",  "profiler.ussdOpsMapping.DIVERT CAPACITY ON",  bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY ON"));
        param(out, "smsc.profiler.ussdOpsMapping.divertCapacityOff", "profiler.ussdOpsMapping.DIVERT CAPACITY OFF", bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.divertStatus",      "profiler.ussdOpsMapping.DIVERT STATUS",       bean.getIntParam("profiler.ussdOpsMapping.DIVERT STATUS"));
        param(out, "smsc.profiler.ussdOpsMapping.divertTo",          "profiler.ussdOpsMapping.DIVERT TO",           bean.getIntParam("profiler.ussdOpsMapping.DIVERT TO"));
        param(out, "smsc.profiler.ussdOpsMapping.concatOn",          "profiler.ussdOpsMapping.CONCAT ON",           bean.getIntParam("profiler.ussdOpsMapping.CONCAT ON"));
        param(out, "smsc.profiler.ussdOpsMapping.concatOff",         "profiler.ussdOpsMapping.CONCAT OFF",          bean.getIntParam("profiler.ussdOpsMapping.CONCAT OFF"));
        param(out, "smsc.profiler.ussdOpsMapping.translitOn",        "profiler.ussdOpsMapping.TRANSLIT ON",         bean.getIntParam("profiler.ussdOpsMapping.TRANSLIT ON"));
        param(out, "smsc.profiler.ussdOpsMapping.translitOff",       "profiler.ussdOpsMapping.TRANSLIT OFF",        bean.getIntParam("profiler.ussdOpsMapping.TRANSLIT OFF"));
      finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.notify ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "profiler.notify", "smsc.profiler.notify", false);
      startParams(out);
        param(out, "smsc.profiler.notify.host",     "profiler.notify.host", bean.getStringParam("profiler.notify.host"));
        param(out, "smsc.profiler.notify.port",     "profiler.notify.port", bean.getIntParam("profiler.notify.port"));
        param(out, "smsc.profiler.notify.dir",      "profiler.notify.dir",  bean.getStringParam("profiler.notify.dir"));
      finishParams(out);
    finishSection(out);
  finishSection(out);
  //################################## abonentinfo #############################
  startSection(out, "abonentinfo", "smsc.abonentInfo", false);
    startParams(out);
      param(out, "smsc.abonentInfo.systemId",         "abonentinfo.systemId",              bean.getStringParam("abonentinfo.systemId"));
      param(out, "smsc.abonentInfo.serviceType",      "abonentinfo.service_type",          bean.getStringParam("abonentinfo.service_type"));
      param(out, "smsc.abonentInfo.protocolId",       "abonentinfo.protocol_id",           bean.getIntParam(   "abonentinfo.protocol_id"));
      param(out, "smsc.abonentInfo.mobileAccessAddr", "abonentinfo.mobile_access_address", bean.getStringParam("abonentinfo.mobile_access_address"));
      param(out, "smsc.abonentInfo.smppAccessAddr",   "abonentinfo.smpp_access_address",   bean.getStringParam("abonentinfo.smpp_access_address"));
    finishParams(out);
  finishSection(out);
   //################################## distrList #############################
  startSection(out, "distrList", "smsc.distrList", false);
    startParams(out);
      param(out, "smsc.distrList.storeDir",      "distrList.storeDir", bean.getStringParam("distrList.storeDir"));
    finishParams(out);
  finishSection(out);
   //################################## Acl #############################
  startSection(out, "acl", "smsc.acl", false);
    startParams(out);
      param(out, "smsc.acl.preCreateSize", "acl.preCreateSize", bean.getIntParam("acl.preCreateSize"));
      param(out, "smsc.acl.storeDir",      "acl.storeDir",      bean.getStringParam("acl.storeDir"));
    finishParams(out);
  finishSection(out);
    //################################## MscManager #############################
  startSection(out, "MscManager", "smsc.MscManager", false);
    startParams(out);
      param(out, "smsc.MscManager.automaticRegistration", "MscManager.automaticRegistration", bean.getBoolParam("MscManager.automaticRegistration"));
      param(out, "smsc.MscManager.failureLimit", "MscManager.failureLimit", bean.getIntParam("MscManager.failureLimit"));
      param(out, "smsc.MscManager.storeFile",      "MscManager.storeFile",    bean.getStringParam("MscManager.storeFile"));
    finishParams(out);
  finishSection(out);

%></div><%
page_menu_begin(out);
page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveConfig");
page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>
