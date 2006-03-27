<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.jsp.smsc.closedgroups.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
                 ru.novosoft.smsc.jsp.util.tables.QueryResultSet" %>
<%@ page import="ru.novosoft.smsc.admin.route.MaskList"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.closedgroups.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%@ taglib uri="http://jakarta.apache.org/taglibs/input-1.0" prefix="input" %>

<%
    TITLE = getLocString("closedgroups.title");
    switch (bean.process(request)) {
        case Index.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case Index.RESULT_OK:

            break;
        case Index.RESULT_ERROR:

            break;
        case Index.RESULT_FILTER:
            response.sendRedirect("closedGroupsFilter.jsp");
            return;
        case Index.RESULT_ADD:
            response.sendRedirect("closedGroupAdd.jsp");
            return;
        case Index.RESULT_EDIT:
            response.sendRedirect("closedGroupEdit.jsp?id=" + URLEncoder.encode(bean.getEditId()));
            return;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
    MENU0_SELECTION = "MENU0_CLOSEDGROUPS";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAdd", "closedgroups.add", "closedgroups.addHint");
    page_menu_button(session, out, "mbDelete", "closedgroups.delete", "closedgroups.deleteHint");
    page_menu_space(out);
    page_menu_end(out);
%>
<div class=content>
<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=editId id=editId>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
    function edit(id_to_edit)
    {
        document.getElementById('jbutton').name = "mbEdit";
        opForm.editId.value = id_to_edit;
        opForm.submit();
        return false;
    }
    function setSort(sorting)
    {
        if (sorting == "<%=bean.getSort()%>")
            opForm.sort.value = "-<%=bean.getSort()%>";
        else
            opForm.sort.value = sorting;
        opForm.submit();
        return false;
    }
    function setFilter(filtering)
    {
        if (filtering == "<%=bean.getSort()%>")
            opForm.sort.value = "-<%=bean.getSort()%>";
        else
            opForm.sort.value = filtering;
        opForm.submit();
        return false;
    }
</script>
<hr>
<table class=list cellspacing=0 cellpadding=0 id=CLOSEDGROUPS_LIST_TABLE>
    <col width="1%">
    <col width="14%" align=left>
    <col width="20%" align=left>
    <col width="30%" align=left>
    <col width="35%" align=left>
    <thead>
        <tr>
            <th width="1%" class=ico><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
            <th width="14%" align=left><a
                    href="javascript:setSort('id')" <%=bean.getSort().endsWith("id") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                    title="<%=getLocString("common.sortmodes.idHint")%>"><%=getLocString("common.sortmodes.id")%></a>
            </th>
            <th width="20%" align=left><a
                    href="javascript:setSort('name')" <%=bean.getSort().endsWith("name") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                    title="<%=getLocString("common.sortmodes.nameHint")%>"><%=getLocString("common.sortmodes.name")%></a>
            </th>
            <th width="30%" align=left><%=getLocString("common.sortmodes.description")%></th>
            <th width="35%" align=left><%=getLocString("common.sortmodes.masks")%></th>
        </tr>
    </thead>
    <tbody>
        <%{
            int row = 0;
            boolean have = true;
            QueryResultSet closedgroups = (QueryResultSet) bean.getClosedGroups();
            try {
                Iterator i = closedgroups.iterator();
            } catch (Exception e) {
                have = false;
                e.printStackTrace();
            }
            if (have) {
                for (Iterator i = closedgroups.iterator(); i.hasNext(); row++) {
                    DataItem item = (DataItem) i.next();
                    String id = (String) item.getValue("id");
                    String descr = (String) item.getValue("def");
                    String encDescr = StringEncoderDecoder.encode(descr);
                    String encId = StringEncoderDecoder.encode(id);
                    String rowId = "CLOSEDGROUP_ID_" + StringEncoderDecoder.encodeHEX(id);
                    String name = (String) item.getValue("name");
                    String encName = StringEncoderDecoder.encode(name);
                    MaskList masks = (MaskList) item.getValue("masks");
        %>
        <tr class=row<%=row & 1%>>
            <td><input class=check type=checkbox name=checkedClosedGroupsNames
                       value="<%=encName%>" <%=bean.isClosedGroupChecked(name) ? "checked" : ""%>></td>
            <td><%=encId%>&nbsp;</td>
            <td><a href="javascript:edit('<%=encId%>')"
                   title="<%=getLocString("closedgroups.editSubTitle")%>"><%=encName%></a></td>
            <td><%=encDescr%></td>
            <td><%=masks.getFixedText(40)%></td>
        </tr>
        <tr class=row<%=row & 1%> id=<%=rowId%>_BODY style="display:none">
            <td>&nbsp;</td>
            <td colspan=4></td>
        </tr>
        <%}
        } //if (have) closed group
        }%>
    </tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp" %>
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAdd", "closedgroups.add", "closedgroups.addHint");
    page_menu_button(session, out, "mbDelete", "closedgroups.delete", "closedgroups.deleteHint");
    page_menu_space(out);
    page_menu_end(out);
%>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>
