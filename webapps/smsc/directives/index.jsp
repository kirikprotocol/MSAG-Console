<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.smsc.directives.Index,
                 java.util.Map" %>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.directives.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("directives.title");
    switch (bean.process(request)) {
        case PageBean.RESULT_OK:
        case PageBean.RESULT_ERROR:
            break;
        case Index.RESULT_DONE:
            response.sendRedirect(CPATH + "/smsc_service/status.jsp");
            return;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_SMSC_Directives";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<div class=content>
<table class=list id=aliases>
<col width="20%">
<col width="20%">
<col width="1%">
<col width="59%">
<tr><th><%=getLocString("common.sortmodes.alias")%></th><th><%=getLocString("common.sortmodes.directive")%></th><th>
    &nbsp;</th><th></th></tr>
<script>
    function removeDirectiveAlias(rowId)
    {
        var tbl = document.getElementById('aliases');
        var r = tbl.rows[rowId];
        tbl.deleteRow(r.rowIndex);
    }
    function createInput(id, value)
    {
        var i = document.createElement("input");
        i.className = "txt";
        i.id = i.name = id;
        i.value = value;
        return i;
    }
    function addSelectOption(select, text, value)
    {
        var o = document.createElement("option");
        try {
          select.add(o, null); // For Firefox, does not work in MSIE
        } catch (ex) {
          select.add(o); // For MSIE
        }
        o.value = o.innerText = o.text = text;
        if (text == value)
            o.selected = true;
    }
    function createSelect(id, value)
    {
        var s = document.createElement("select");
        s.className = "selectW";
        s.setAttribute("class", "selectW");
        s.id = s.name = id;
        addSelectOption(s, "def", value);
        addSelectOption(s, "template", value);
        addSelectOption(s, "ack", value);
        addSelectOption(s, "noack", value);
        addSelectOption(s, "hide", value);
        addSelectOption(s, "unhide", value);
        addSelectOption(s, "flash", value);
        return s;
    }
    function addDirectiveAlias()
    {
        var tbl = document.getElementById('aliases');
        var aElem = document.getElementById('newAlias');
        var dElem = document.getElementById('newDirective');
        var a = aElem.value;
        var d = dElem.value;
        var aHex = encodeHEX(a);

        if (a == null || a.length == 0) {
            alert("<%=getLocString("directives.alerts.nameNotSpecified")%>");
            return false;
        }
        if (tbl.rows['row_' + aHex] != null) {
            alert("<%=getLocString("directives.alerts.aliasAlreadyExists")%>: " + a);
            return false;
        }

        var r = tbl.insertRow(tbl.rows.length - 1);
        //r.className = "row" + ((tbl.rows.length+1) & 1);
        r.id = "row_" + aHex;
        var aCell = document.createElement("td");
        aCell.appendChild(createInput("alias", a));
        r.appendChild(aCell);
        var dCell = document.createElement("td");
        dCell.appendChild(createSelect("directive_" + aHex, d));
        r.appendChild(dCell);
        var bCell = document.createElement("td");
        bCell.innerHTML = "<img src=\"/images/but_del.gif\" onclick=\"removeDirectiveAlias('row_" + aHex + "')\" style=\"cursor:pointer;\">";
        r.appendChild(bCell);
        aElem.value = "";

        return false;
    }
</script>
<%
    int row = 0;
    for (Iterator i = bean.getAliases().entrySet().iterator(); i.hasNext();) {
        final Map.Entry entry = (Map.Entry) i.next();
        final String alias = (String) entry.getKey();
        final String directive = (String) entry.getValue();
        final String aliasHex = StringEncoderDecoder.encodeHEX(alias);
%><tr id="row_<%=aliasHex%>">
    <td><input class=txt name="alias" value="<%=alias%>" validation="id" onkeyup="resetValidation(this)"></td>
    <td><select class=selectW name="directive_<%=aliasHex%>">
        <option value="def"      <%="def"     .equals(directive) ? "selected" : ""%>>def</option>
        <option value="template" <%="template".equals(directive) ? "selected" : ""%>>template</option>
        <option value="ack"      <%="ack"     .equals(directive) ? "selected" : ""%>>ack</option>
        <option value="noack"    <%="noack"   .equals(directive) ? "selected" : ""%>>noack</option>
        <option value="hide"     <%="hide"    .equals(directive) ? "selected" : ""%>>hide</option>
        <option value="unhide"   <%="unhide"  .equals(directive) ? "selected" : ""%>>unhide</option>
        <option value="flash"    <%="flash"   .equals(directive) ? "selected" : ""%>>flash</option>
    </select></td>
    <td><img src="/images/but_del.gif" onclick="removeDirectiveAlias('row_<%=aliasHex%>')" style="cursor:pointer;"></td>
</tr><%
    }
%>
<tr>
    <td><input class=txt id=newAlias name=newAlias validation="id" onkeyup="resetValidation(this)"></td>
    <td><select class=selectW id=newDirective name=newDirective>
        <option value="def">def</option>
        <option value="template">template</option>
        <option value="ack">ack</option>
        <option value="noack">noack</option>
        <option value="hide">hide</option>
        <option value="unhide">unhide</option>
        <option value="flash">flash</option>
    </select></td>
    <td><img src="/images/but_add.gif"
             onclick="return validateField(document.getElementById('newAlias')) && addDirectiveAlias();"
             style="cursor:pointer;"></td>
</tr>
</table>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbDone", "common.buttons.done", "common.buttons.done");
    page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "directives.cancelHint", "clickCancel()");
    page_menu_space(out);
    page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>