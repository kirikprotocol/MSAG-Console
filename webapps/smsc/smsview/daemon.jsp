<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.smsview.*,
                 ru.novosoft.smsc.util.*,
                 java.text.*, java.util.*,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.admin.Constants,
                 ru.novosoft.smsc.admin.service.ServiceInfo,
                 ru.novosoft.smsc.jsp.smsc.localeResources.Section,
                 java.io.IOException"%>
<%@ page import="ru.novosoft.smsc.jsp.smsview.*"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsview.SmsDaemonFormBean"
/><jsp:setProperty name="bean" property="*"/><%
  ServiceIDForShowStatus = Constants.ARCHIVE_DAEMON_SVC_ID;
  FORM_METHOD = "POST";
  TITLE = getLocString("smsview.daemonTitle");
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
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%!
  void printAddParam(JspWriter out, String section) throws IOException
  {
    out.print("<tr class=row" + ((row++) & 1) + ">");
    out.print("<th><input class=txt id=\"newParamName_"+section+"\" name=\"newParamName_"+section+"\"></th>");
    out.print("<td width=100% ><input class=txtW id=\"newParamValue_"+section+"\" name=\"newParamValue_"+section+"\"></td>");
    out.print("<td><img src=\"/images/but_add.gif\" onclick=\"addParam('"+section+"')\" title='" + getLocString("common.hints.addParam") + "'></td>");
    out.print("</tr>");
  }
  void printDelParam(JspWriter out, String section, String param, String value) throws IOException
  {
    String fullParam = section + Section.NAME_DELIMETER + param;
    out.print("<tr class=row" + ((row++) & 1) + " id=\"paramRow_"+fullParam+"\">");
    out.print("<th nowrap>"+param+"</th>");
    out.print("<td width=100% ><input class=txtW id=\""+fullParam+"\" name=\""+fullParam+"\" value=\""+StringEncoderDecoder.encode(value)+"\"></td>");
    out.print("<td><img src=\"/images/but_del.gif\" onclick=\"delParam('"+section+"', '"+param+"')\" title='" + getLocString("common.hints.delParam") + "'></td>");
    out.print("</tr>");
  }
%>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveConfig");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
  page_menu_space(out);
  page_menu_button(session, out, "mbStart", "common.buttons.start", "smsview.startDaemon", false);
  page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsview.stopDaemon", false);
//  page_menu_button(session, out, "mbStart", "common.buttons.start", "smsview.startDaemon", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
//  page_menu_button(session, out, "mbStop",  "common.buttons.stop",  "smsview.stopDaemon", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
  page_menu_end(out);
%>
<script language="JavaScript">
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "<%=getLocString("common.statuses.stopped")%>");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "<%=getLocString("common.statuses.running")%>");
	window.setTimeout(refreshStartStopButtonsStatus, 500);
}
refreshStartStopButtonsStatus();

function addParam(sectionName)
{
	tableElem = opForm.all("paramTable_" + sectionName);
	paramNameElem = opForm.all("newParamName_" + sectionName);
	paramValueElem = opForm.all("newParamValue_" + sectionName);

	newRow = tableElem.insertRow(tableElem.rows.length-1);
	newRow.className = "row" + (tableElem.rows.length & 1);
	newRow.id = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
	newCell = document.createElement("th");
	newCell.className = "label";
	newCell.innerText = paramNameElem.value;
	newRow.appendChild(newCell);

	inputElement = document.createElement("input");
	inputElement.name = sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
	inputElement.value = paramValueElem.value;
	inputElement.className = "txtW";
	newCell = newRow.insertCell();
	newCell.appendChild(inputElement);

	imgElement = document.createElement("img");
	imgElement.src = "/images/but_del.gif";
	imgElement.setAttribute('sectionName', sectionName);
	imgElement.setAttribute('paramName', paramNameElem.value);
	imgElement.attachEvent("onclick", removeParam_Event);
	newCell = newRow.insertCell();
	newCell.appendChild(imgElement);

	paramNameElem.value = "";
	paramValueElem.value = "";
}
function removeParam_Event()
{
  delParam(event.srcElement.attributes.sectionName.nodeValue, event.srcElement.attributes.paramName.nodeValue);
}
function delParam(sectionName, paramName)
{
  tableElem = opForm.all("paramTable_" + sectionName);
	rowId = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramName;
	rowElem = tableElem.rows(rowId);
	tableElem.deleteRow(rowElem.rowIndex);
}
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMSC Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
<div class=secSmsc><%=getLocString("smsview.daemonSubTitle")%></div>
<%
    //################################## Global #############################
    startParams(out);
      param(out, "smsview.startInterval", "ArchiveDaemon.interval", bean.getIntParam("ArchiveDaemon.interval"));
    finishParams(out);
    //################################## View #############################
    startSection(out, "View", "smsview.smsview", false);
      startParams(out);
        param(out, "common.util.host", "ArchiveDaemon.View.host", bean.getStringParam("ArchiveDaemon.View.host"));
        param(out, "common.util.port", "ArchiveDaemon.View.port", bean.getIntParam("ArchiveDaemon.View.port"));
      finishParams(out);
    finishSection(out);
    //################################## Queries #############################
    startSection(out, "Queries", "smsview.smsqueries", false);
      startParams(out);
        param(out, "smsview.maximum", "ArchiveDaemon.Queries.max", bean.getIntParam("ArchiveDaemon.Queries.max"));
        param(out, "smsview.initial", "ArchiveDaemon.Queries.init", bean.getIntParam("ArchiveDaemon.Queries.init"));
      finishParams(out);
    finishSection(out);
    //################################## Transactions ##########################
    startSection(out, "Transactions", "smsview.transactions", false);
      startParams(out);
        param(out, "smsview.maxTransSize", "ArchiveDaemon.Transactions.maxSmsCount",     bean.getIntParam("ArchiveDaemon.Transactions.maxSmsCount"));
        param(out, "smsview.maxTransTime", "ArchiveDaemon.Transactions.maxTimeInterval", bean.getIntParam("ArchiveDaemon.Transactions.maxTimeInterval"));
      finishParams(out);
    finishSection(out);
    //################################## Locations #############################
    startSection(out, "Locations", "smsview.locations", true);
      startParams(out);
        param(out, "smsview.archDest", "ArchiveDaemon.Locations.baseDestination", bean.getStringParam("ArchiveDaemon.Locations.baseDestination"));
        param(out, "smsview.textDest", "ArchiveDaemon.Locations.textDestination", bean.getStringParam("ArchiveDaemon.Locations.textDestination"));
      finishParams(out);
      startSection(out, "sources", "smsview.sources", true);
        final String baseSrcSection = SmsDaemonFormBean.LOC_SOURCES_SECTION;
        startParams(out, "paramTable_"+baseSrcSection);
          HashMap locationsMap = bean.getSubParams(baseSrcSection);
          for (Iterator i=locationsMap.keySet().iterator(); i.hasNext();) {
            String locName = (String)i.next();
            Object locDir = locationsMap.get(locName);
            if (locDir != null && locDir instanceof String)
              printDelParam(out, baseSrcSection, locName, (String)locDir);
          }
          printAddParam(out, baseSrcSection);
        finishParams(out);
      finishSection(out);
    finishSection(out);
    //################################## Indexator #############################
    startSection(out, "Indexator", "smsview.indexator", true);
      startParams(out);
        param(out, "smsview.smsIdHashSize",    "ArchiveDaemon.Indexator.smsIdHashSize",    bean.getIntParam("ArchiveDaemon.Indexator.smsIdHashSize"));
        param(out, "smsview.smeIdHashSize",    "ArchiveDaemon.Indexator.smeIdHashSize",    bean.getIntParam("ArchiveDaemon.Indexator.smeIdHashSize"));
        param(out, "smsview.routeIdHashSize",  "ArchiveDaemon.Indexator.routeIdHashSize",  bean.getIntParam("ArchiveDaemon.Indexator.routeIdHashSize"));
        param(out, "smsview.addrHashSize",     "ArchiveDaemon.Indexator.addrHashSize",     bean.getIntParam("ArchiveDaemon.Indexator.addrHashSize"));
        param(out, "smsview.smeIdRootSize",    "ArchiveDaemon.Indexator.smeIdRootSize",    bean.getIntParam("ArchiveDaemon.Indexator.smeIdRootSize"));
        param(out, "smsview.smeIdChunkSize",   "ArchiveDaemon.Indexator.smeIdChunkSize",   bean.getIntParam("ArchiveDaemon.Indexator.smeIdChunkSize"));
        param(out, "smsview.routeIdRootSize",  "ArchiveDaemon.Indexator.routeIdRootSize",  bean.getIntParam("ArchiveDaemon.Indexator.routeIdRootSize"));
        param(out, "smsview.routeIdChunkSize", "ArchiveDaemon.Indexator.routeIdChunkSize", bean.getIntParam("ArchiveDaemon.Indexator.routeIdChunkSize"));
        param(out, "smsview.addrRootSize",     "ArchiveDaemon.Indexator.addrRootSize",     bean.getIntParam("ArchiveDaemon.Indexator.addrRootSize"));
        param(out, "smsview.defAddrChunkSize", "ArchiveDaemon.Indexator.defAddrChunkSize", bean.getIntParam("ArchiveDaemon.Indexator.defAddrChunkSize"));
      finishParams(out);
      startSection(out, "smeAddrChunkSize", "smsview.smeAddrChunkSize", true);
        final String baseSmeSection = SmsDaemonFormBean.SME_PARAMS_SECTION;
        startParams(out, "paramTable_"+baseSmeSection);
          HashMap smeMap = bean.getSubParams(baseSmeSection);
          for (Iterator i=smeMap.keySet().iterator(); i.hasNext();) {
            String smeName = (String)i.next();
            Object smeValue = smeMap.get(smeName);
            if (smeValue != null && smeValue instanceof Integer)
              printDelParam(out, baseSmeSection, smeName, String.valueOf(smeValue));
          }
          printAddParam(out, baseSmeSection);
        finishParams(out);
      finishSection(out);
    finishSection(out);
%></div><%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave",  "common.buttons.save",  "common.buttons.saveConfig");
  page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
