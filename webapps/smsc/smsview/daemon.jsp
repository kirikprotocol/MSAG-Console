<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*, java.util.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.admin.Constants,
                 ru.novosoft.smsc.admin.service.ServiceInfo"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsview.SmsDaemonFormBean"
/><jsp:setProperty name="bean" property="*"/><%
  ServiceIDForShowStatus = Constants.ARCHIVE_DAEMON_SVC_ID;
  FORM_METHOD = "POST";
  TITLE = "SMS Archive Daemon";
  MENU0_SELECTION = "MENU0_SMSDAEMON";
  switch(bean.process(request))
  {
    case SmsDaemonFormBean.RESULT_DONE:
      response.sendRedirect("daemon.jsp");
      return;
    case SmsDaemonFormBean.RESULT_OK:
      break;
    case SmsDaemonFormBean.RESULT_ERROR:
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
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "running");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc>Archive Daemon Configuration</div>
<%
    //################################## Global #############################
    startParams(out);
      param(out, "Start interval", "ArchiveDaemon.interval", bean.getIntParam("ArchiveDaemon.interval"));
    finishParams(out);
    //################################## View #############################
    startSection(out, "View", "SMS View", false);
      startParams(out);
        param(out, "host", "ArchiveDaemon.View.host", bean.getStringParam("ArchiveDaemon.View.host"));
        param(out, "port", "ArchiveDaemon.View.port", bean.getIntParam("ArchiveDaemon.View.port"));
      finishParams(out);
    finishSection(out);
    //################################## Queries #############################
    startSection(out, "Queries", "SMS Queries", false);
      startParams(out);
        param(out, "maximum", "ArchiveDaemon.Queries.max", bean.getIntParam("ArchiveDaemon.Queries.max"));
        param(out, "initial", "ArchiveDaemon.Queries.init", bean.getIntParam("ArchiveDaemon.Queries.init"));
      finishParams(out);
    finishSection(out);
    //################################## Locations #############################
    startSection(out, "Locations", "Locations", true);
      startParams(out);
        param(out, "destination", "ArchiveDaemon.Locations.destination", bean.getStringParam("ArchiveDaemon.Locations.destination"));
      finishParams(out);
      // TODO: support for entries adding/deleting
      startSection(out, "Source", "Source", true);
        startParams(out);
          param(out, "enabled", "ArchiveDaemon.Locations.Source.enabled", bean.getBoolParam("ArchiveDaemon.Locations.Source.enabled"));
          param(out, "dir",     "ArchiveDaemon.Locations.Source.dir",     bean.getStringParam("ArchiveDaemon.Locations.Source.dir"));
        finishParams(out);
      finishSection(out);
    finishSection(out);
    //################################## Indexator #############################
    startSection(out, "Indexator", "Indexator", false);
      startParams(out);
        param(out, "smsIdHashSize",    "ArchiveDaemon.Indexator.smsIdHashSize",    bean.getIntParam("ArchiveDaemon.Indexator.smsIdHashSize"));
        param(out, "smeIdHashSize",    "ArchiveDaemon.Indexator.smeIdHashSize",    bean.getIntParam("ArchiveDaemon.Indexator.smeIdHashSize"));
        param(out, "routeIdHashSize",  "ArchiveDaemon.Indexator.routeIdHashSize",  bean.getIntParam("ArchiveDaemon.Indexator.routeIdHashSize"));
        param(out, "addrHashSize",     "ArchiveDaemon.Indexator.addrHashSize",     bean.getIntParam("ArchiveDaemon.Indexator.addrHashSize"));
        param(out, "smeIdRootSize",    "ArchiveDaemon.Indexator.smeIdRootSize",    bean.getIntParam("ArchiveDaemon.Indexator.smeIdRootSize"));
        param(out, "smeIdChunkSize",   "ArchiveDaemon.Indexator.smeIdChunkSize",   bean.getIntParam("ArchiveDaemon.Indexator.smeIdChunkSize"));
        param(out, "routeIdRootSize",  "ArchiveDaemon.Indexator.routeIdRootSize",  bean.getIntParam("ArchiveDaemon.Indexator.routeIdRootSize"));
        param(out, "routeIdChunkSize", "ArchiveDaemon.Indexator.routeIdChunkSize", bean.getIntParam("ArchiveDaemon.Indexator.routeIdChunkSize"));
        param(out, "addrRootSize",     "ArchiveDaemon.Indexator.addrRootSize",     bean.getIntParam("ArchiveDaemon.Indexator.addrRootSize"));
        param(out, "defAddrChunkSize", "ArchiveDaemon.Indexator.defAddrChunkSize", bean.getIntParam("ArchiveDaemon.Indexator.defAddrChunkSize"));
      finishParams(out);
      startSection(out, "smeAddrChunkSize", "smeAddrChunkSize", true);
        startParams(out);
          // TODO: support for smeAddrChunkSize entries adding/deleting
          param(out, "MAP_PROXY", "ArchiveDaemon.Indexator.smeAddrChunkSize.MAP_PROXY", bean.getIntParam("ArchiveDaemon.Indexator.smeAddrChunkSize.MAP_PROXY"));
          param(out, "hello", "ArchiveDaemon.Indexator.smeAddrChunkSize.hello", bean.getIntParam("ArchiveDaemon.Indexator.smeAddrChunkSize.hello"));
          param(out, "world", "ArchiveDaemon.Indexator.smeAddrChunkSize.world", bean.getIntParam("ArchiveDaemon.Indexator.smeAddrChunkSize.world"));
        finishParams(out);
      finishSection(out);
    finishSection(out);
%></div><%
  page_menu_begin(out);
  page_menu_button(out, "mbSave",  "Save",  "Save config");
  page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
