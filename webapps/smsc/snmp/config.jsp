<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.localeResources.Section,
                 ru.novosoft.smsc.jsp.smsc.snmp.ConfigBean" %>
<%@ page import="java.util.*"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.snmp.SnmpObject"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.snmp.ConfigBean"/>
<jsp:setProperty name="bean" property="*"/>
<%
    FORM_METHOD = "POST";
    TITLE = getLocString("snmp.configTitle");
    MENU0_SELECTION = "MENU0_SNMPCONFIG";
    switch (bean.process(request)) {
        case ConfigBean.RESULT_DONE:
            response.sendRedirect("config.jsp");
            return;
        case ConfigBean.RESULT_OK:
            break;
        case ConfigBean.RESULT_ERROR:
            break;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp" %>
<%!
        void printAddParam(JspWriter out, Section section) throws IOException {
        String sectionFullName = section.getFullName();
        out.print("<tr class=row" + ((row++) & 1) + ">");
        out.print("<th><input class=txt id=\"newParamName_" + sectionFullName + "\" name=\"newParamName_" + sectionFullName + "\"></th>");
        out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + sectionFullName + "\" name=\"newParamValue_" + sectionFullName + "\"></td>");
        out.print("<td><img src=\"/images/but_add.gif\" onclick=\"addParam('" + sectionFullName + "')\" title='" + getLocString("common.hints.addParam") + "'></td>");
        out.print("</tr>");
    }

    void printAddSection(JspWriter out, String string) throws IOException {
        out.print("<div>");
        out.print("<input class=txt id=\"newSectionInput_" + string + "\" name=\"newSectionInput_" + string + "\">");
        out.print("<img src=\"/images/but_add.gif\" onclick=\"addSection('" + string + "')\" title='" + getLocString("common.hints.addSection") + "' style='position:relative;top:4px'></div>");
    }

    void printAddSectionCounter(JspWriter out, String string) throws IOException {
        out.print("<div>");
        out.print("<select class=select id=\"newSectionCounterInput_" + string + "\" name=\"newSectionCounterInput_" + string + "\">");
        out.print("<option value=\"0x00d\" SELECTED>0x00d</option>");
        out.print("<option value=\"0x014\">0x014</option>");
        out.print("<option value=\"0x058\">0x058</option>");
        out.print("<option value=\"accepted\">accepted</option>");
        out.print("<option value=\"delivered\">delivered</option>");
        out.print("<option value=\"failed\">failed</option>");
        out.print("<option value=\"other\">other</option>");
        out.print("<option value=\"rejected\">rejected</option>");
        out.print("<option value=\"retried\">retried</option>");
        out.print("<option value=\"SDP\">SDP</option>");
        out.print("<option value=\"temperror\">temperror</option>");
        out.print("</select>");
//        out.print("<input class=txt id=\"newSectionCounterInput_" + string + "\" name=\"newSectionCounterInput_" + string + "\">");
        out.print("<img src=\"/images/but_add.gif\" onclick=\"addSectionCounter('" + string + "')\" title='" + getLocString("common.hints.addSection") + "' style='position:relative;top:4px'></div>");
    }
%>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<script language="JavaScript">
function createImgButton(imgUrl, onclickT, tooltipText)
{
    return "<img src=\"" + imgUrl + "\" onclick=\"" + onclickT + "\" title=\"" + tooltipText + "\">";
}
function createImgButton2(imgUrl, onclickT, tooltipText, styleText)
{
    return "<img src=\"" + imgUrl + "\" onclick=\"" + onclickT + "\" title=\"" + tooltipText + "\" style=\"" + styleText + "\">";
}
function removeSection(sectionName)
{
    sectionElem = document.getElementById("sectionHeader_" + sectionName);
    if (sectionElem.removeNode)
      sectionElem.removeNode(true);
    else
      sectionElem.parentNode.removeChild(sectionElem);
    sectionElem = document.getElementById("sectionValue_" + sectionName);
    if (sectionElem.removeNode)
      sectionElem.removeNode(true);
    else
      sectionElem.parentNode.removeChild(sectionElem);
}
function sectionHeader(sectionName, fullName)
{
    var sectionHeader_ = "sectionHeader_" + fullName;
    return ""
            + "<div class=collapsing_tree_opened id=\"" + sectionHeader_ + "\" onclick=\"collasping_tree_showhide_section('" + fullName + "')\">"
            + "<table cellspacing=0><tr><td width=100%>"
            + sectionName
            + "</td>"
            + "<td>"
            + createImgButton("/images/but_del.gif", "removeSection('" + fullName + "')", "<%=getLocString("common.hints.delSection")%>")
            + "</td>"
            + "</tr></table></div>";
}
function createInput(id_, className)
{
    return "<input class=" + className + " name=\"" + id_ + "\" id=\"" + id_ + "\">";
}
function addSectionField(sectionFN)
{
    var newSectionInput = "newSectionInput_" + sectionFN;
    return ""
            + "<div>"
            + createInput(newSectionInput, "txt")
            + createImgButton2("/images/but_add.gif", "addSection('" + sectionFN + "')", "<%=getLocString("common.hints.addSection")%>", "position:relative;top:4px")
            + "</div>";
}
function addParamField(sectionFN)
{
    var name = sectionFN + ".enabled";

    return ""
            + "<table cellspacing=0>"
            + "<col width=150px>"
            + "<tr class=row0>"
            + "<th nowrap><label for=\"" + name + "\"><%=getLocString("snmp.enabled")%></label></th>"
            + "<td><input class=check type=checkbox name=\"" + name + "\" id=\"" + name + "\" value=true checked></td>"
            + "</tr>"
            + "</table>";

}
function sectionValue(sectionName, fullName)
{
    var sectionValue_ = "sectionValue_" + fullName;
    return ""
            + "<table cellspacing=0 sellpadding=0 id=\"" + sectionValue_ + "\">"
            + "<col width='56px'/>"
            + "<tr><th/><td>"
            + addParamField(fullName)
            + "<div class=collapsing_tree_closed  id=\"sectionHeader_" + fullName + "_counters\""
            + " onclick=\"collasping_tree_showhide_section('" + fullName + "_counters')\"><%=getLocString("snmp.counters")%></div>"
            + "<table cellspacing=0 cellpadding=0 id=\"sectionValue_" + fullName + "_counters\"><col width='56px'/><tr><th/><td>"
            + "<select class=select id=\"newSectionCounterInput_" + fullName + "_counters\" name=\"newSectionCounterInput_" + fullName + "_counters\">"
            + "<option value=\"0x00d\" SELECTED>0x00d</option>"
            + "<option value=\"0x014\">0x014</option>"
            + "<option value=\"0x058\">0x058</option>"
            + "<option value=\"accepted\">accepted</option>"
            + "<option value=\"delivered\">delivered</option>"
            + "<option value=\"failed\">failed</option>"
            + "<option value=\"other\">other</option>"
            + "<option value=\"rejected\">rejected</option>"
            + "<option value=\"retried\">retried</option>"
            + "<option value=\"SDP\">SDP</option>"
            + "<option value=\"temperror\">temperror</option>"
            + "</select>"
//            + "<div><input class=txt id=\"newSectionCounterInput_" + fullName + "_counters\""
//            + " name=\"newSectionCounterInput_" + fullName + "_counters\">"
            + "<img src=\"/images/but_add.gif\" onclick=\"addSectionCounter('" + fullName + "_counters')\" title='<%=getLocString("common.hints.addSection")%>'>"
            + "</div></td></tr></table>"
            + "</td></tr></table>"
            + "</td></tr>"
            + "</table>";
}
function sectionValueCounter(sectionName, fullName)
{
    var sectionValue_ = "sectionValue_" + fullName;
    return ""
            + "<table cellspacing=0 id=\"" + sectionValue_ + "\">"
            + "<col width='56px'/>"
            + "<tr><th/><td>"
            + "<table cellspacing=0 id=\"" + sectionValue_ + "\">"
            + "<col width='150px'>"
            + "<tr class=row0><th nowrap><%=getLocString("snmp.limits.warning")%></th>"
            + "<td nowrap><input class=txt name=" + fullName + ".warning id=" + fullName + ".warning value=0></td></tr>"
            + "<tr class=row1><th nowrap><%=getLocString("snmp.limits.minor")%></th>"
            + "<td nowrap><input class=txt name=" + fullName + ".minor id=" + fullName + ".minor value=0></td></tr>"
            + "<tr class=row0><th nowrap><%=getLocString("snmp.limits.major")%></th>"
            + "<td nowrap><input class=txt name=" + fullName + ".major id=" + fullName + ".major value=0></td></tr>"
            + "<tr class=row1><th nowrap><%=getLocString("snmp.limits.critical")%></th>"
            + "<td nowrap><input class=txt name=" + fullName + ".critical id=" + fullName + ".critical value=0></td></tr>"
            + "</table>";
            + "</td></tr>"
            + "</table>";

}
function addSection(parentSectionName)
{
    newSectionNameElem = document.getElementById("newSectionInput_" + parentSectionName);
    newSectionName = newSectionNameElem.value;
    fullName = newSectionName;

    check = document.getElementById(fullName);
    if (check != null) return;

    parentSectionTable = document.getElementById("sectionValue_" + parentSectionName);

    parentNewRow = parentSectionTable.insertRow(parentSectionTable.rows.length);
    parentNewRow.insertCell(0);
    nestCell = parentNewRow.insertCell(0);
    nestCell.innerHTML = ""
            + sectionHeader(newSectionName, fullName)
            + sectionValue(newSectionName, fullName);

    newSectionNameElem.value = "";
}
function addSectionCounter(parentSectionName)
{
    newSectionNameElem = document.getElementById("newSectionCounterInput_" + parentSectionName);
    newSectionName = newSectionNameElem.value;
    fullName = parentSectionName + "<%=Section.NAME_DELIMETER%>" + newSectionName;

    check = document.getElementById(fullName);
    if (check != null) return;

    parentSectionTable = document.getElementById("sectionValue_" + parentSectionName);

    parentNewRow = parentSectionTable.insertRow(0);
    parentNewRow.insertCell(0);
    nestCell = parentNewRow.insertCell(0);
    nestCell.innerHTML = ""
            + sectionHeader(newSectionName, fullName)
            + sectionValueCounter(newSectionName, fullName);

    newSectionNameElem.value = "";
}
</script>
<%-- ~~~~~~~~~~~~~~~~~~~~~~~~~~~ SNMP Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=content>
    <div class=secSmsc><%=getLocString("snmp.configSubTitle")%></div>
    <%
        //################################## View #############################
        startSection(out, "main", "snmp.configTitle", true);

        startSection(out, "counterInterval", "snmp.counterInterval", true);
        startParams(out);
        param(out, "snmp.counterInterval", "counterInterval", bean.getCounterInterval());
        finishParams(out);
        finishSection(out);

        startSection(out, "Default", "snmp.default", false);
        startParams(out);
        param(out, "snmp.default.enabled", "defaultEnabled", bean.isDefaultEnabled());
        finishParams(out);
        java.util.Map defMap = bean.defaultCounters;
        for (Iterator i = defMap.keySet().iterator(); i.hasNext();) {
            String name = (String) i.next();
            startSectionPre(out, "default."+name, name, false);
            Object value = defMap.get(name);
            if (value != null && value instanceof Vector && ((Vector) value).size() == 4) {
                Vector vct = (Vector) value;
                startParams(out);
                param(out, "snmp.limits.warning", "default."+name+".warning", (String) vct.elementAt(0));
                param(out, "snmp.limits.minor", "default."+name+".minor", (String) vct.elementAt(1));
                param(out, "snmp.limits.major", "default."+name+".major", (String) vct.elementAt(2));
                param(out, "snmp.limits.critical", "default."+name+".critical", (String) vct.elementAt(3));
                finishParams(out);
            }
            finishSection(out);
        }
        finishSection(out);

        defMap = bean.objects;
        for (Iterator i = defMap.keySet().iterator(); i.hasNext();) {
            String name = (String) i.next();
            SnmpObject obj = (SnmpObject) defMap.get(name);
            startSectionPre(out, name, name, false, "<img src=\"/images/but_del.gif\" onclick=\"removeSection('" + name + "')\" title='" + getLocString("common.hints.delSection") + "'>");
                startParams(out);
                param(out, "snmp.enabled", obj.id+"."+"enabled", obj.enabled);
                finishParams(out);
                startSection(out, obj.id+"_counters", "snmp.counters", false);
                    Map map = obj.counters;
                    for (Iterator j = map.keySet().iterator(); j.hasNext();) {
                        name = (String) j.next();
                        startSectionPre(out, obj.id+"_"+name, name, false, "<img src=\"/images/but_del.gif\" onclick=\"removeSection('" + obj.id+"_"+name + "')\" title='" + getLocString("common.hints.delSection") + "'>");
                        Object value = map.get(name);
                        if (value != null && value instanceof Vector && ((Vector) value).size() == 4) {
                            Vector vct = (Vector) value;
                            startParams(out);
                            param(out, "snmp.limits.warning", obj.id+"."+name+".warning", (String) vct.elementAt(0));
                            param(out, "snmp.limits.minor", obj.id+"."+name+".minor", (String) vct.elementAt(1));
                            param(out, "snmp.limits.major", obj.id+"."+name+".major", (String) vct.elementAt(2));
                            param(out, "snmp.limits.critical", obj.id+"."+name+".critical", (String) vct.elementAt(3));
                            finishParams(out);
                        }
                        finishSection(out);
                    }
                printAddSectionCounter(out,obj.id+"_counters");
                finishSection(out);
            finishSection(out);
        }
        finishSection(out);
        printAddSection(out, "main");
    %>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbSave", "common.buttons.save", "common.buttons.saveConfig");
    page_menu_button(session, out, "mbReset", "common.buttons.reset", "common.buttons.reset"/*, "clickCancel()"*/);
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
