<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.Section,
                 ru.novosoft.smsc.jsp.smsc.timezones.Index" %>
<%@ page import="java.util.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.timezones.TimeZones"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.timezones.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
    FORM_METHOD = "POST";
    TITLE = getLocString("timezones.table");
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
        out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + section + "\" name=\"newParamValue_" + section + "\" value="+selectedValue+"></td>");
        out.print("<td><img src=\"/images/but_add.gif\" onclick=\"addParam('" + section + "')\" title='" + getLocString("common.hints.addParam") + "'></td>");
        out.print("</tr>");
    }

    void printAddParam(JspWriter out, String section, String selectedValue) throws IOException {
        out.print("<tr class=row" + ((row++) & 1) + ">");
        out.print("<th><input class=txt id=\"newParamName_" + section + "\" name=\"newParamName_" + section + "\"></th>");
        out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + section + "\" name=\"newParamValue_" + section + "\" value="+selectedValue+"></td>");
        out.print("<td><img src=\"/images/but_add.gif\" onclick=\"addParam('" + section + "')\" title='" + getLocString("common.hints.addParam") + "'></td>");
        out.print("</tr>");
    }

    void printDelParam(JspWriter out, String section, String param, String value) throws IOException {
        String fullParam = section + Section.NAME_DELIMETER + param;
        out.print("<tr class=row" + ((row++) & 1) + " id=\"paramRow_" + fullParam + "\">");
        out.print("<th nowrap>" + param + "</th>");
        out.print("<td width=100% ><input class=txtW id=\"" + fullParam + "\" name=\"" + fullParam + "\" value=\"" + StringEncoderDecoder.encode(value) + "\"></td>");
        out.print("<td><img src=\"/images/but_del.gif\" onclick=\"delParam('" + section + "', '" + param + "')\" title='" + getLocString("common.hints.delParam") + "'></td>");
        out.print("</tr>");
    }
%>
<script language="JavaScript">
    function addParam(sectionName)
    {
        tableElem = opForm.all("paramTable_" + sectionName);
        paramNameElem = opForm.all("newParamName_" + sectionName);
        paramValueElem = opForm.all("newParamValue_" + sectionName);

        test = opForm.all("paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value);
        if (test == null)
        {
            newRow = tableElem.insertRow(tableElem.rows.length - 1);
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
            newCell = newRow.insertCell(1);
            newCell.appendChild(inputElement);

            imgElement = document.createElement("img");
            imgElement.src = "/images/but_del.gif";
            imgElement.setAttribute('sectionName', sectionName);
            imgElement.setAttribute('paramName', paramNameElem.value);
            imgElement.attachEvent("onclick", removeParam_Event);
            newCell = newRow.insertCell(2);
            newCell.appendChild(imgElement);

            paramNameElem.value = "";
            paramValueElem.value = "";
        }
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
<div class=content>
    <div class=secSmsc><%=getLocString("timezones.table")%></div>
    <%
        startSection(out, "timezonesTable", "timezones.masks", true);
        startParams(out, "paramTable_" + TimeZones.TIMEZONES_MASKS_PREFIX);
        java.util.Collection masks = bean.getMasks();
        for (Iterator i = masks.iterator(); i.hasNext();) {
            String mask = (String) i.next();
            String timezone = bean.getMaskTimeZone(mask);
            if (timezone != null)
                printDelParam(out, TimeZones.TIMEZONES_MASKS_PREFIX, mask, timezone);
        }
        printAddParam(out, TimeZones.TIMEZONES_MASKS_PREFIX, bean.getDefaultTimeZone());
        finishParams(out);
        finishSection(out);

        startSection(out, "timezonesTable", "timezones.subjects", true);
        startParams(out, "paramTable_" + TimeZones.TIMEZONES_SUBJECTS_PREFIX);
        java.util.Collection subjs = bean.getSubjects();
        for (Iterator i = subjs.iterator(); i.hasNext();) {
            String subj = (String) i.next();
            String timezone = bean.getSubjTimeZone(subj);
            if (timezone != null)
                printDelParam(out, TimeZones.TIMEZONES_SUBJECTS_PREFIX, subj, timezone);
        }
        subjs = bean.getAllSubjects();
        printAddParamSubj(out, TimeZones.TIMEZONES_SUBJECTS_PREFIX, bean.getDefaultTimeZone(), subjs);
        finishParams(out);
        finishSection(out);
    %>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "timezones.saveHint");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>