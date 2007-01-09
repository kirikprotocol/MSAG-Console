<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.smsc.aliases.Index,
                 ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter"%>
<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.smsc.aliases.Index"/>
<jsp:setProperty name="bean" property="*"/>

<%
    TITLE = getLocString("aliases.title");

    switch (bean.process(request)) {
        case PageBean.RESULT_OK :
        case PageBean.RESULT_ERROR :
        case PageBean.RESULT_DONE:
            break;
        case Index.RESULT_ADD :
            response.sendRedirect("aliasesAdd.jsp");
            return;
        case Index.RESULT_EDIT :
            response.sendRedirect("aliasesEdit.jsp?alias=" + URLEncoder.encode((String)bean.getSelectedAlias().getAlias().getMask()) + "&address=" +
                    URLEncoder.encode((String)bean.getSelectedAlias().getAddress().getMask()) + "&hide=" + URLEncoder.encode((String.valueOf(bean.getSelectedAlias().isHide()))));
            return;
        default :
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
  int rowN=0;
%>


<%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
    MENU0_SELECTION = "MENU0_ALIASES";
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>

<div class=content>

    <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <div class=page_subtitle><%=getLocString("common.titles.aliases")%></div>
    <table class=properties_list cellspacing=0 cellspadding=0>
        <col width=1px>
        <col align=left>
        <%
            for (int i = 0; i < bean.getAliases().length; i++) {
        %>
        <tr class=row<%=(rowN++) & 1%>>
            <td><input class=txt name=aliases value="<%=bean.getAliases()[i]%>" validation="mask"
                       onkeyup="resetValidation(this)"></td>
            <td>&nbsp;</td>
        </tr>
        <%}%>
        <tr class=row<%=(rowN++) & 1%>>
            <td><input class=txt name=aliases validation="mask" onkeyup="resetValidation(this)"></td>
            <td><%addButton(out, "mbAddAlias", "Add", "aliases.addAliasHint");%></td>
        </tr>
    </table>
    <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Addresses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <div class=page_subtitle><%=getLocString("common.titles.addresses")%></div>
    <table class=properties_list cellspacing=0 cellspadding=0>
        <col width=1px>
        <col align=left>
        <%
            rowN = 0;
            for (int i = 0; i < bean.getAddresses().length; i++) {
        %>
        <tr class=row<%=(rowN++) & 1%>>
            <td><input class=txt name=addresses value="<%=bean.getAddresses()[i]%>" validation="mask"
                       onkeyup="resetValidation(this)"></td>
            <td></td>
        </tr>
        <%}%>
        <tr class=row<%=(rowN++) & 1%>>
            <td><input class=txt name=addresses validation="mask" onkeyup="resetValidation(this)"></td>
            <td><%addButton(out, "mbAddAddress", "Add", "aliases.addAddressHint");%></td>
        </tr>
    </table>
    <%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
    <div class=page_subtitle><%=getLocString("common.titles.options")%></div>
    <table class=properties_list cellspacing=0 cellspadding=0>
        <col width=1px>
        <%rowN = 0;%>
        <tr class=row<%=(rowN++) & 1%>>
            <th><%=getLocString("aliases.hideOption")%>:</th>
            <td><div class=select><select class=txt name=hide>
                <option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.all")%></option>
                <option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.hide")%></option>
                <option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.noHide")%></option>
            </select></div></td>
        </tr>
    </table>
</div>

<input type=hidden name=initialized value="<%=bean.isInitialized()%>">

<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbAdd", "aliases.add", "aliases.add");
    page_menu_button(session, out, "mbQuery", "common.buttons.query", "common.buttons.query");
    if (bean.isInitialized())
      page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clear");
    page_menu_space(out);
    page_menu_end(out);

    if (bean.isInitialized()) {
%>

<div class=content>
  <%@ include file="/WEB-INF/inc/paged_table.jsp"%>
</div>
<%
      page_menu_begin(out);
      page_menu_button(session, out, "mbAdd", "aliases.add", "aliases.add");
      page_menu_button(session, out, "mbDelete", "aliases.delete", "aliases.deleteHint");
      page_menu_space(out);
      page_menu_end(out);
    }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>

