<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.Section,
                 ru.novosoft.smsc.jsp.smsc.smscenters.SMSCenters" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.timezones.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smscenters.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
  FORM_METHOD = "POST";
  TITLE = getLocString("smscenters.table");
  MENU0_SELECTION = "MENU0_SMSC_Reshedule";
  switch (bean.process(request)) {
    case Index.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case Index.RESULT_OK:
      break;
    case Index.RESULT_ERROR:
      break;
    default:
      errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "timezones.saveHint");
  page_menu_space(out);
  page_menu_end(out);
%>

<%!
  void printAddParamSubj(JspWriter out, String section, String selectedValue, Collection values) throws IOException {
    out.print("<tr class=row" + ((row++) & 1) + ">");
    out.print("<th><select id=\"newParamName_" + section + "\" name=\"newParamName_" + section + "\" class=\"txt\">");
    for (Iterator i = values.iterator(); i.hasNext();) {
      String name = (String) i.next();
      String encName = StringEncoderDecoder.encode(name);
      out.print("<option value=\""+encName + "\">" + encName + "</option>");
    }
    out.print("</select></th>");
    out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + section + "\" name=\"newParamValue_" + section + "\" value=\""+selectedValue+"\"></td>");
    out.print("<td><img class=button src=\"/images/but_add.gif\" onclick=\"addParam('" + section + "')\" title='" + getLocString("common.hints.addParam") + "'></td>");
    out.print("</tr>");
  }

  void printAddParam(JspWriter out, String section, String selectedValue) throws IOException {
    out.print("<tr class=row" + ((row++) & 1) + ">");
    out.print("<th><input class=txt id=\"newParamName_" + section + "\" name=\"newParamName_" + section + "\" validation=\"mask\" onkeyup=\"resetValidation(this)\"></th>");
    out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + section + "\" name=\"newParamValue_" + section + "\" value=\""+selectedValue+"\" ></td>");
    out.print("<td><img class=button src=\"/images/but_add.gif\" onclick=\"addParam('" + section + "')\" title='" + getLocString("common.hints.addParam") + "'></td>");
    out.print("</tr>");
  }

  void printDelParam(JspWriter out, String section, String param, String value) throws IOException {
    String fullParam = section + Section.NAME_DELIMETER + param;
    out.print("<tr class=row" + ((row++) & 1) + " id=\"paramRow_" + fullParam + "\">");
    out.print("<th nowrap>" + param + "</th>");
    out.print("<td width=100% ><input class=txtW id=\"" + fullParam + "\" name=\"" + fullParam + "\" value=\"" + StringEncoderDecoder.encode(value) + "\" validation=\"nonEmpty\" onkeyup=\"resetValidation(this)\"></td>");
    out.print("<td><img class=button src=\"/images/but_del.gif\" onclick=\"delParam('" + section + "', '" + param + "')\" title='" + getLocString("common.hints.delParam") + "'></td>");
    out.print("</tr>");
  }
%>
<script language="JavaScript">
  function addParam(sectionName) {
    tableElem = document.getElementById("paramTable_" + sectionName);

    paramNameElem = document.getElementById("newParamName_" + sectionName);
    resetValidation(paramNameElem);
    if (!validateField(paramNameElem) || paramNameElem.value == "")
      return;

    paramValueElem = document.getElementById("newParamValue_" + sectionName);
    resetValidation(paramValueElem);
    if (!validateField(paramValueElem) || paramNameElem.value == "")
      return;

    test = document.getElementById("paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value);
    if (test == null)
    {
      newRow = tableElem.insertRow(tableElem.rows.length - 1);
      newRow.className = "row" + (tableElem.rows.length & 1);
      newRow.id = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
      newCell = document.createElement("th");
      newCell.className = "label";
      newCell.innerText = newCell.innerHTML = paramNameElem.value;
      newRow.appendChild(newCell);

      inputElement = document.createElement("input");
      inputElement.name = sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
      inputElement.value = paramValueElem.value;
      inputElement.setAttribute("validation", "nonEmpty");
      inputElement.className = "txtW";
      newCell = newRow.insertCell(1);
      newCell.appendChild(inputElement);

      imgElement = document.createElement("img");
      imgElement.src = "/images/but_del.gif";
      imgElement.setAttribute('sectionName', sectionName);
      imgElement.setAttribute('paramName', paramNameElem.value);
      imgElement.className = "button";
      imgElement.onclick = removeParam_Event;
      // imgElement.attachEvent("onclick", removeParam_Event);
      newCell = newRow.insertCell(2);
      newCell.appendChild(imgElement);

      paramNameElem.value = "";
      paramValueElem.value = "";
    }
  }

  function removeParam_Event(e) {
    if (!e)
      e = event;
    var a = e.target || e.srcElement;

    delParam(a.attributes.sectionName.nodeValue, a.attributes.paramName.nodeValue);
  }

  function delParam(sectionName, paramName) {
    tableElem = document.getElementById("paramTable_" + sectionName);
    rowId = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramName;
    rowElem = tableElem.rows[rowId];
    tableElem.deleteRow(rowElem.rowIndex);
  }
</script>
<div class=content>
  <div class=secSmsc><%=getLocString("smscenters.table")%></div>
    <%
      startSection(out, "smscentersMask", "smscenters.masks", true);
      startParams(out, "paramTable_" + SMSCenters.SMS_CENTERS_MASKS_PREFIX);
      java.util.Collection masks = bean.getMasks();
      for (Iterator i = masks.iterator(); i.hasNext();) {
        String mask = (String) i.next();
        String smscNumber = bean.getMasksSMSCNumber(mask);
        if (smscNumber != null)
          printDelParam(out, SMSCenters.SMS_CENTERS_MASKS_PREFIX, mask, smscNumber);
      }
      printAddParam(out, SMSCenters.SMS_CENTERS_MASKS_PREFIX, "");
      finishParams(out);
      finishSection(out);

      startSection(out, "smscentersSubjects", "smscenters.subjects", true);
      startParams(out, "paramTable_" + SMSCenters.SMS_CENTERS_SUBJECTS_PREFIX);
      java.util.Collection routes = bean.getSubjects();
      for (Iterator i = routes.iterator(); i.hasNext();) {
        String route = (String) i.next();
        String smscNumber = bean.getRoutesSMSCNumber(route);
        if (smscNumber != null)
          printDelParam(out, SMSCenters.SMS_CENTERS_SUBJECTS_PREFIX, route, smscNumber);
      }
      routes = bean.getAllRoutes();
      printAddParamSubj(out, SMSCenters.SMS_CENTERS_SUBJECTS_PREFIX, "", routes);
      finishParams(out);
      finishSection(out);
    %>
</div>
<%
  page_menu_begin(out);
  page_menu_button(session, out, "mbSave", "common.buttons.save", "smscenters.saveHint");
  page_menu_space(out);
  page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>