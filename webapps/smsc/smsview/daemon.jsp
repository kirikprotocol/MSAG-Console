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
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%!
  void printAddParam(JspWriter out, String section) throws IOException
  {
    out.print("<tr class=row" + ((row++) & 1) + ">");
    out.print("<th><input class=txt id=\"newParamName_"+section+"\" name=\"newParamName_"+section+"\"></th>");
    out.print("<td width=100% ><input class=txtW id=\"newParamValue_"+section+"\" name=\"newParamValue_"+section+"\"></td>");
    out.print("<td><img src=\"/images/but_add.gif\" onclick=\"addParam('"+section+"')\" title='Add new parameter'></td>");
    out.print("</tr>");
  }
  void printDelParam(JspWriter out, String section, String param, String value) throws IOException
  {
    String fullParam = section + Section.NAME_DELIMETER + param;
    out.print("<tr class=row" + ((row++) & 1) + " id=\"paramRow_"+fullParam+"\">");
    out.print("<th nowrap>"+param+"</th>");
    out.print("<td width=100% ><input class=txtW id=\""+fullParam+"\" name=\""+fullParam+"\" value=\""+StringEncoderDecoder.encode(value)+"\"></td>");
    out.print("<td><img src=\"/images/but_del.gif\" onclick=\"delParam('"+section+"', '"+param+"')\" title='Remove parameter'></td>");
    out.print("</tr>");
  }
%>
<%
  page_menu_begin(out);
  page_menu_button(out, "mbSave",  "Save",  "Save config");
  page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
  page_menu_space(out);
  page_menu_button(out, "mbStart", "Start", "Start SMSC", bean.getStatus() == ServiceInfo.STATUS_STOPPED);
  page_menu_button(out, "mbStop",  "Stop",  "Stop SMSC", bean.getStatus() == ServiceInfo.STATUS_RUNNING);
  page_menu_end(out);
%>
<script language="JavaScript">
function refreshStartStopButtonsStatus()
{
	document.all.mbStart.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "stopped");
	document.all.mbStop.disabled = (document.all.RUNNING_STATUSERVICE_<%=Constants.ARCHIVE_DAEMON_SVC_ID%>.innerText != "running");
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
      startSection(out, "sources", "sources", true);
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
    startSection(out, "Indexator", "Indexator", true);
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
  page_menu_button(out, "mbSave",  "Save",  "Save config");
  page_menu_button(out, "mbReset", "Reset", "Reset", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
