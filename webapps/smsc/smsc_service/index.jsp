<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.admin.profiler.Profile,
                 ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
                 java.util.LinkedList,
                 java.util.List" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
    ServiceIDForShowStatus = Constants.SMSC_SME_ID;
    FORM_METHOD = "POST";
    TITLE = getLocString("smsc.title");
    MENU0_SELECTION = "MENU0_SMSC";

    switch (bean.process(request)) {
        case Index.RESULT_DONE :
            response.sendRedirect("index.jsp");
            return;
        case Index.RESULT_OK :
            break;
        case Index.RESULT_ERROR :
            break;
        default :
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
    page_menu_space(out);
    page_menu_button(session, out, "mbStart", "common.buttons.start", "smsc.start", false);
    page_menu_button(session, out, "mbStop", "common.buttons.stop", "smsc.stop", false);

    //page_menu_button(session, out, "mbStart", "common.buttons.start", "smsc.start", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
    //page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsc.stop", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
    page_menu_end(out);
%>
<script>
    function refreshStartStopButtonsStatus() {
        document.getElementById('mbStart').disabled = (document.getElementById(
                'RUNNING_STATUSERVICE_<%= Constants.SMSC_SME_ID %>').innerText != "<%= getLocString("common.statuses.offline") %>");
        document.getElementById('mbStop').disabled = (document.getElementById(
                'RUNNING_STATUSERVICE_<%= Constants.SMSC_SME_ID %>').innerText == "<%= getLocString("common.statuses.offline") %>");

        window.setTimeout(refreshStartStopButtonsStatus, 10000);
    }

    refreshStartStopButtonsStatus();
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc>
    <%= getLocString("smsc.configuration") %>
</div>
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
    param(out, "smsc.core.stateCount", "core.state_machines_count", bean.getIntParam("core.state_machines_count"));
    param(out, "smsc.core.mainLoopsCount", "core.mainLoopsCount", bean.getIntParam("core.mainLoopsCount"));
    param(out, "smsc.core.queueLimit", "core.eventQueueLimit", bean.getIntParam("core.eventQueueLimit"));
    param(out, "smsc.core.schedConfig", "core.reschedule_config", bean.getStringParam("core.reschedule_config"));
    param(out, "smsc.core.servAddr", "core.service_center_address", bean.getStringParam("core.service_center_address"));
    param(out, "smsc.core.ussdAddr", "core.ussd_center_address", bean.getStringParam("core.ussd_center_address"));
    param(out, "smsc.core.ussdSSN", "core.ussd_ssn", bean.getIntParam("core.ussd_ssn"));
    param(out, "smsc.core.addUssdSSN", "core.add_ussd_ssn", bean.getStringParam("core.add_ussd_ssn"));
    param(out, "smsc.core.receiptOrigAddress", "core.receipt_orig_address", bean.getStringParam("core.receipt_orig_address"));
    param(out, "smsc.core.systemId", "core.systemId", bean.getStringParam("core.systemId"));
    param(out, "smsc.core.servType", "core.service_type", bean.getStringParam("core.service_type"));
    param(out, "smsc.core.protId", "core.protocol_id", bean.getIntParam("core.protocol_id"));
    param(out, "smsc.core.locales", "core.locales", bean.getStringParam("core.locales"));
    param(out, "smsc.core.defLocale", "core.default_locale", bean.getStringParam("core.default_locale"));
    param(out, "smsc.core.mergeTime", "core.mergeTimeout", bean.getIntParam("core.mergeTimeout"));
    param(out, "smsc.core.tzConfig", "core.timezones_config", bean.getStringParam("core.timezones_config"));
    param(out, "smsc.core.smartMultipart", "core.smartMultipartForward", bean.getBoolParam("core.smartMultipartForward"));
    param(out, "smsc.core.srcSmeSeparateRouting", "core.srcSmeSeparateRouting", bean.getBoolParam("core.srcSmeSeparateRouting"));
    param(out, "smsc.core.schedulerSoftLimit", "core.schedulerSoftLimit", bean.getIntParam("core.schedulerSoftLimit"));
    param(out, "smsc.core.schedulerHardLimit", "core.schedulerHardLimit", bean.getIntParam("core.schedulerHardLimit"));
    
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.perfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "core.performance", "smsc.core.perfmon", false);
    startParams(out);
    param(out, "common.util.host", "core.performance.host", bean.getStringParam("core.performance.host"));
    param(out, "common.util.port", "core.performance.port", bean.getIntParam("core.performance.port"));
    finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ core.smeperfomance ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "core.smeperformance", "smsc.core.topmon", false);
    startParams(out);
    param(out, "common.util.host", "core.smeperformance.host", bean.getStringParam("core.smeperformance.host"));
    param(out, "common.util.port", "core.smeperformance.port", bean.getIntParam("core.smeperformance.port"));
    finishParams(out);
    finishSection(out);
    finishSection(out);
    //################################## sms #############################
    startSection(out, "sms", "smsc.sms", false);
    startParams(out);
    param(out, "smsc.sms.validTime", "sms.max_valid_time", bean.getIntParam("sms.max_valid_time"));
    finishParams(out);
    finishSection(out);
    //################################## map #############################
    startSection(out, "map", "smsc.map", false);
    startParams(out);
    param(out, "smsc.map.busyMTdelay", "map.busyMTDelay", bean.getIntParam("map.busyMTDelay"));
    param(out, "smsc.map.lockedMOdelay", "map.lockedByMODelay", bean.getIntParam("map.lockedByMODelay"));
    param(out, "smsc.map.MOlockTime", "map.MOLockTimeout", bean.getIntParam("map.MOLockTimeout"));
    param(out, "smsc.map.allowCallBarred", "map.allowCallBarred", bean.getBoolParam("map.allowCallBarred"));
    param(out, "smsc.map.ussdV1Enabled", "map.ussdV1Enabled", bean.getBoolParam("map.ussdV1Enabled"));
    param(out, "smsc.map.ussdV1UseOrigEntityNumber", "map.ussdV1UseOrigEntityNumber", bean.getBoolParam("map.ussdV1UseOrigEntityNumber"));
    param(out, "smsc.map.mapIOTasksCount", "map.mapIOTasksCount", bean.getIntParam("map.mapIOTasksCount"));
    finishParams(out);
    finishSection(out);
    //################################## traffic control #############################
    startSection(out, "trafficControl", "smsc.trControl", false);
    startParams(out);
    param(out, "smsc.trControl.maxSmsPerSecond", "trafficControl.maxSmsPerSecond", bean.getIntParam("trafficControl.maxSmsPerSecond"));
    param(out, "smsc.trControl.shapeTimeFrame", "trafficControl.shapeTimeFrame", bean.getIntParam("trafficControl.shapeTimeFrame"));
    param(out, "smsc.trControl.statTimeFrame", "trafficControl.statTimeFrame", bean.getIntParam("trafficControl.statTimeFrame"));
    finishParams(out);
    finishSection(out);
    //################################## MessageStore #############################
    startSection(out, "MessageStore", "smsc.mesStor", false);
    startParams(out);
    param(out, "smsc.mesStor.statDir", "MessageStore.statisticsDir", bean.getStringParam("MessageStore.statisticsDir"));
    param(out, "smsc.mesStor.archiveDir", "MessageStore.archiveDir", bean.getStringParam("MessageStore.archiveDir"));
    param(out, "smsc.mesStor.archiveInterval", "MessageStore.archiveInterval", bean.getIntParam("MessageStore.archiveInterval"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~ MessageStore.LocalStore ~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "MessageStore.LocalStore", "smsc.mesStor.localStore", false);
    startParams(out);
    param(out, "smsc.mesStor.localStore.filename", "MessageStore.LocalStore.filename", bean.getStringParam("MessageStore.LocalStore.filename"));
    if( bean.getStringParam("MessageStore.LocalStore.maxStoreSize") == null ) { // it means old style of this param as int
      param(out, "smsc.mesStor.localStore.maxStoreSize", "MessageStore.LocalStore.maxStoreSize", bean.getIntParam("MessageStore.LocalStore.maxStoreSize"));
    } else {
      param(out, "smsc.mesStor.localStore.maxStoreSize", "MessageStore.LocalStore.maxStoreSize", bean.getStringParam("MessageStore.LocalStore.maxStoreSize"));
    }
    param(out, "smsc.mesStor.localStore.minRollTime", "MessageStore.LocalStore.minRollTime", bean.getIntParam("MessageStore.LocalStore.minRollTime"));
    param(out, "smsc.mesStor.localStore.msgidfile", "MessageStore.LocalStore.msgidfile", bean.getStringParam("MessageStore.LocalStore.msgidfile"));
    finishParams(out);
    finishSection(out);
    finishSection(out);
    //################################## smpp #############################
    startSection(out, "smpp", "smsc.smpp", false);
    startParams(out);
    param(out, "common.util.host", "smpp.host", bean.getStringParam("smpp.host"));
    param(out, "common.util.port", "smpp.port", bean.getIntParam("smpp.port"));
    param(out, "smsc.smpp.readTimeout", "smpp.readTimeout", bean.getIntParam("smpp.readTimeout"));
    param(out, "smsc.smpp.inactivityTime", "smpp.inactivityTime", bean.getIntParam("smpp.inactivityTime"));
    param(out, "smsc.smpp.inactivityTimeOut", "smpp.inactivityTimeOut", bean.getIntParam("smpp.inactivityTimeOut"));
    param(out, "smsc.smpp.bindTimeout", "smpp.bindTimeout", bean.getIntParam("smpp.bindTimeout"));
    param(out, "smsc.smpp.defaultConnectionsLimit", "smpp.defaultConnectionsLimit", bean.getIntParam("smpp.defaultConnectionsLimit"));
    finishParams(out);
    startSection(out, "connectionsLimitsForIps", "smsc.smpp.connectionsLimitsForIps", false);

    if (bean.getConnectionsLimitsForIps().size() > 0) {
        startParams(out);

        //---------------------------------- smpp.connectionsLimitsForIps.%name% -----------------------------
        for (Iterator i = bean.getConnectionsLimitsForIps().iterator(); i.hasNext();) {
            String name = (String) i.next();

            param(out, "smsc.smpp.connectionsLimitsForIps.Ip", "smpp.connectionsLimitsForIps.Ip", bean.getStringParam(
                    "smpp.connectionsLimitsForIps." + name));
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
    param(out, "smsc.profiler.systemId", "profiler.systemId", bean.getStringParam("profiler.systemId"));
    param(out, "smsc.profiler.service_type", "profiler.service_type", bean.getStringParam("profiler.service_type"));
    param(out, "smsc.profiler.protocol_id", "profiler.protocol_id", bean.getIntParam("profiler.protocol_id"));
    param(out, "smsc.profiler.storeFile", "profiler.storeFile", bean.getStringParam("profiler.storeFile"));
    paramSelect(out, "smsc.profiler.defaultDataCoding", "profiler.defaultDataCoding", profilerCodepages, bean.getStringParam("profiler.defaultDataCoding"));
    paramSelect(out, "smsc.profiler.defaultReport", "profiler.defaultReport", profilerReportOptions, bean.getStringParam("profiler.defaultReport"));
    param(out, "smsc.profiler.defaultHide", "profiler.defaultHide", bean.getBoolParam("profiler.defaultHide"));
    param(out, "smsc.profiler.defaultHideModifiable", "profiler.defaultHideModifiable", bean.getBoolParam("profiler.defaultHideModifiable"));
    param(out, "smsc.profiler.defaultDivertModifiable", "profiler.defaultDivertModifiable", bean.getBoolParam("profiler.defaultDivertModifiable"));
    param(out, "smsc.profiler.defaultUssdIn7Bit", "profiler.defaultUssdIn7Bit", bean.getBoolParam("profiler.defaultUssdIn7Bit"));
    param(out, "smsc.profiler.defaultUdhConcat", "profiler.defaultUdhConcat", bean.getBoolParam("profiler.defaultUdhConcat"));
    param(out, "smsc.profiler.defaultAccessMaskIn", "profiler.defaultAccessMaskIn", bean.getIntParam("profiler.defaultAccessMaskIn"));
    param(out, "smsc.profiler.defaultAccessMaskOut", "profiler.defaultAccessMaskOut", bean.getIntParam("profiler.defaultAccessMaskOut"));
    finishParams(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.ussdOpsMapping ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "profiler.ussdOpsMapping", "smsc.profiler.ussdOpsMapping", false);
    startParams(out);
    param(out, "smsc.profiler.ussdOpsMapping.reportNone", "profiler.ussdOpsMapping.REPORT NONE", bean.getIntParam("profiler.ussdOpsMapping.REPORT NONE"));
    param(out, "smsc.profiler.ussdOpsMapping.reportFull", "profiler.ussdOpsMapping.REPORT FULL", bean.getIntParam("profiler.ussdOpsMapping.REPORT FULL"));
    param(out, "smsc.profiler.ussdOpsMapping.reportFinal", "profiler.ussdOpsMapping.REPORT FINAL", bean.getIntParam("profiler.ussdOpsMapping.REPORT FINAL"));
    param(out, "smsc.profiler.ussdOpsMapping.localeRuRu", "profiler.ussdOpsMapping.LOCALE RU_RU", bean.getIntParam("profiler.ussdOpsMapping.LOCALE RU_RU"));
    param(out, "smsc.profiler.ussdOpsMapping.localeEnEn", "profiler.ussdOpsMapping.LOCALE EN_EN", bean.getIntParam("profiler.ussdOpsMapping.LOCALE EN_EN"));
    param(out, "smsc.profiler.ussdOpsMapping.default", "profiler.ussdOpsMapping.DEFAULT", bean.getIntParam("profiler.ussdOpsMapping.DEFAULT"));
    param(out, "smsc.profiler.ussdOpsMapping.ucs2", "profiler.ussdOpsMapping.UCS2", bean.getIntParam("profiler.ussdOpsMapping.UCS2"));
    param(out, "smsc.profiler.ussdOpsMapping.hide", "profiler.ussdOpsMapping.HIDE", bean.getIntParam("profiler.ussdOpsMapping.HIDE"));
    param(out, "smsc.profiler.ussdOpsMapping.unhide", "profiler.ussdOpsMapping.UNHIDE", bean.getIntParam("profiler.ussdOpsMapping.UNHIDE"));
    param(out, "smsc.profiler.ussdOpsMapping.ussd7bitOn", "profiler.ussdOpsMapping.USSD7BIT ON", bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT ON"));
    param(out, "smsc.profiler.ussdOpsMapping.ussd7bitOff", "profiler.ussdOpsMapping.USSD7BIT OFF", bean.getIntParam("profiler.ussdOpsMapping.USSD7BIT OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertOn", "profiler.ussdOpsMapping.DIVERT ON", bean.getIntParam("profiler.ussdOpsMapping.DIVERT ON"));
    param(out, "smsc.profiler.ussdOpsMapping.divertOff", "profiler.ussdOpsMapping.DIVERT OFF", bean.getIntParam("profiler.ussdOpsMapping.DIVERT OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertAbsentOn", "profiler.ussdOpsMapping.DIVERT ABSENT ON", bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT ON"));
    param(out, "smsc.profiler.ussdOpsMapping.divertAbsentOff", "profiler.ussdOpsMapping.DIVERT ABSENT OFF", bean.getIntParam("profiler.ussdOpsMapping.DIVERT ABSENT OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertBlockedOn", "profiler.ussdOpsMapping.DIVERT BLOCKED ON", bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED ON"));
    param(out, "smsc.profiler.ussdOpsMapping.divertBlockedOff", "profiler.ussdOpsMapping.DIVERT BLOCKED OFF",bean.getIntParam("profiler.ussdOpsMapping.DIVERT BLOCKED OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertBarredOn", "profiler.ussdOpsMapping.DIVERT BARRED ON", bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED ON"));
    param(out, "smsc.profiler.ussdOpsMapping.divertBarredOff", "profiler.ussdOpsMapping.DIVERT BARRED OFF", bean.getIntParam("profiler.ussdOpsMapping.DIVERT BARRED OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertCapacityOn", "profiler.ussdOpsMapping.DIVERT CAPACITY ON",bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY ON"));
    param(out, "smsc.profiler.ussdOpsMapping.divertCapacityOff", "profiler.ussdOpsMapping.DIVERT CAPACITY OFF",bean.getIntParam("profiler.ussdOpsMapping.DIVERT CAPACITY OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.divertStatus", "profiler.ussdOpsMapping.DIVERT STATUS", bean.getIntParam("profiler.ussdOpsMapping.DIVERT STATUS"));
    param(out, "smsc.profiler.ussdOpsMapping.divertTo", "profiler.ussdOpsMapping.DIVERT TO", bean.getIntParam("profiler.ussdOpsMapping.DIVERT TO"));
    param(out, "smsc.profiler.ussdOpsMapping.concatOn", "profiler.ussdOpsMapping.CONCAT ON", bean.getIntParam("profiler.ussdOpsMapping.CONCAT ON"));
    param(out, "smsc.profiler.ussdOpsMapping.concatOff", "profiler.ussdOpsMapping.CONCAT OFF", bean.getIntParam("profiler.ussdOpsMapping.CONCAT OFF"));
    param(out, "smsc.profiler.ussdOpsMapping.translitOn", "profiler.ussdOpsMapping.TRANSLIT ON", bean.getIntParam("profiler.ussdOpsMapping.TRANSLIT ON"));
    param(out, "smsc.profiler.ussdOpsMapping.translitOff", "profiler.ussdOpsMapping.TRANSLIT OFF", bean.getIntParam("profiler.ussdOpsMapping.TRANSLIT OFF"));
    finishParams(out);
    finishSection(out);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ profiler.notify ~~~~~~~~~~~~~~~~~~~~~~~~~
    startSection(out, "profiler.notify", "smsc.profiler.notify", false);
    startParams(out);
    param(out, "smsc.profiler.notify.host", "profiler.notify.host", bean.getStringParam("profiler.notify.host"));
    param(out, "smsc.profiler.notify.port", "profiler.notify.port", bean.getIntParam("profiler.notify.port"));
    param(out, "smsc.profiler.notify.dir", "profiler.notify.dir", bean.getStringParam("profiler.notify.dir"));
    finishParams(out);
    finishSection(out);
    finishSection(out);
    //################################## abonentinfo #############################
    startSection(out, "abonentinfo", "smsc.abonentInfo", false);
    startParams(out);
    param(out, "smsc.abonentInfo.systemId", "abonentinfo.systemId", bean.getStringParam("abonentinfo.systemId"));
    param(out, "smsc.abonentInfo.serviceType", "abonentinfo.service_type", bean.getStringParam("abonentinfo.service_type"));
    param(out, "smsc.abonentInfo.protocolId", "abonentinfo.protocol_id", bean.getIntParam("abonentinfo.protocol_id"));
    param(out, "smsc.abonentInfo.mobileAccessAddr", "abonentinfo.mobile_access_address", bean.getStringParam("abonentinfo.mobile_access_address"));
    param(out, "smsc.abonentInfo.smppAccessAddr", "abonentinfo.smpp_access_address", bean.getStringParam("abonentinfo.smpp_access_address"));
    finishParams(out);
    finishSection(out);
    //################################## MscManager #############################
    startSection(out, "MscManager", "smsc.MscManager", false);
    startParams(out);
    param(out, "smsc.MscManager.automaticRegistration", "MscManager.automaticRegistration", bean.getBoolParam("MscManager.automaticRegistration"));
    param(out, "smsc.MscManager.failureLimit", "MscManager.failureLimit", bean.getIntParam("MscManager.failureLimit"));
    param(out, "smsc.MscManager.storeFile", "MscManager.storeFile", bean.getStringParam("MscManager.storeFile"));
    param(out, "smsc.MscManager.singleAttemptTimeout", "MscManager.singleAttemptTimeout", bean.getIntParam("MscManager.singleAttemptTimeout"));
    finishParams(out);
    finishSection(out);
    //################################## distrList #############################
    startSection(out, "distrList", "smsc.distrList", false);
    startParams(out);
    param(out, "smsc.distrList.storeDir", "distrList.storeDir", bean.getStringParam("distrList.storeDir"));
    param(out, "smsc.distrList.originatingAddress", "distrList.originatingAddress", bean.getStringParam("distrList.originatingAddress"));
    finishParams(out);
    finishSection(out);
    //################################## Acl #############################
    startSection(out, "acl", "smsc.acl", false);
    startParams(out);
    param(out, "smsc.acl.storeDir", "acl.storeDir", bean.getStringParam("acl.storeDir"));
    param(out, "smsc.acl.preCreateSize", "acl.preCreateSize", bean.getIntParam("acl.preCreateSize"));
    finishParams(out);
    finishSection(out);
    //################################## INMAN #############################
    List chargingPolicyOptions = new LinkedList();
    chargingPolicyOptions.add("deliver"); // do not localize it - it's option name that understood by SMSC
    chargingPolicyOptions.add("submit");

    startSection(out, "inman", "smsc.inman", false);
    startParams(out);
    param(out, "common.util.host", "inman.host", bean.getStringParam("inman.host"));
    param(out, "common.util.port", "inman.port", bean.getIntParam("inman.port"));
    finishParams(out);
    startSection(out, "inman.chargingPolicy", "smsc.inman.chargingPolicy", false);
    startParams(out);
    paramSelect(out, "smsc.inman.peer2peer", "inman.chargingPolicy.peer2peer", chargingPolicyOptions, bean.getStringParam("inman.chargingPolicy.peer2peer"));
    paramSelect(out, "smsc.inman.other", "inman.chargingPolicy.other", chargingPolicyOptions, bean.getStringParam("inman.chargingPolicy.other"));
    finishParams(out);
    finishSection(out);
    finishSection(out);
    //############################## Alias manager ###########################
    startSection(out, "aliasman", "smsc.aliasman", false);
    startParams(out);
    param(out, "smsc.aliasman.storeFile", "aliasman.storeFile", bean.getStringParam("aliasman.storeFile"));
    finishParams(out);
    finishSection(out);
    //################################## Cluster #############################
    List clusterModeOptions = new LinkedList();
    clusterModeOptions.add(getLocString("cluster.mode.ha"));
    clusterModeOptions.add(getLocString("cluster.mode.hs"));
    clusterModeOptions.add(getLocString("cluster.mode.single"));
    startSection(out, "cluster", "smsc.cluster", false);
    startParams(out);
    paramSelect(out, "smsc.cluster.mode", "cluster.mode", clusterModeOptions, bean.getStringParam("cluster.mode"));
    param(out, "smsc.cluster.agentHost", "cluster.agentHost", bean.getStringParam("cluster.agentHost"));
    param(out, "smsc.cluster.agentPort", "cluster.agentPort", bean.getIntParam("cluster.agentPort"));
    param(out, "smsc.cluster.host1", "cluster.host1", bean.getStringParam("cluster.host1"));
    param(out, "smsc.cluster.port1", "cluster.port1", bean.getIntParam("cluster.port1"));
    param(out, "smsc.cluster.host2", "cluster.host2", bean.getStringParam("cluster.host2"));
    param(out, "smsc.cluster.port2", "cluster.port2", bean.getIntParam("cluster.port2"));
    finishParams(out);
    finishSection(out);
    //############################## Alias manager ###########################
    startSection(out, "snmp", "smsc.snmp", false);
    startParams(out);
    param(out, "smsc.snmp.csvFileDir", "snmp.csvFileDir", bean.getStringParam("snmp.csvFileDir"));
    param(out, "smsc.snmp.csvFileRollInterval", "snmp.csvFileRollInterval", bean.getIntParam("snmp.csvFileRollInterval"));
    param(out, "smsc.snmp.cacheTimeout", "snmp.cacheTimeout", bean.getIntParam("cacheTimeout"));
    finishParams(out);
    finishSection(out);
%>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
