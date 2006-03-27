<%@ include file="/WEB-INF/inc/code_header.jsp" %>
<%@ page import="ru.novosoft.smsc.admin.dl.Principal,
                 ru.novosoft.smsc.admin.route.Mask,
                 ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.dl.Principals" %>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.dl.Principals"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE = getLocString("dl.principalTitle");
    MENU0_SELECTION = "MENU0_DL_PRINCIPALS";

    int beanResult = PageBean.RESULT_OK;
    switch (beanResult = bean.process(request)) {
        case PageBean.RESULT_OK:
        case PageBean.RESULT_ERROR:
            break;
        case Principals.RESULT_DONE:
            response.sendRedirect("principals.jsp");
            return;
        case Principals.RESULT_EDIT:
            response.sendRedirect("principalsEdit.jsp?address=" + URLEncoder.encode(bean.getEdit()));
            return;
        case Principals.RESULT_ADD:
            response.sendRedirect("principalsEdit.jsp?create=true");
            return;
        case Principals.RESULT_FILTER:
            response.sendRedirect("principalsFilter.jsp");
            return;
        default:
            errorMessages.add(new SMSCJspException(ru.novosoft.smsc.jsp.SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp" %>
<div class=content>
    <input type=hidden name=initialized value=true>
    <input type=hidden id=startPosition name=startPosition value="<%=bean.getStartPosition()%>">
    <input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
    <input type=hidden name=sort>
    <input type=hidden id=edit name=edit>
    <%=getLocString("dl.addressStartsWith")%>
    <input class=txt name=filterAddress
           value="<%=bean.getFilterAddress() == null ? "" : StringEncoderDecoder.encode(bean.getFilterAddress())%>"
           validation="address_prefix" onkeyup="resetValidation(this)">
</div>
<%
    page_menu_begin(out);
    page_menu_button(session, out, "mbQuery", "common.buttons.query", "common.buttons.query");
    page_menu_space(out);
    page_menu_end(out);
    if (bean.isInitialized()) {
        if (bean.getTotalSizeInt() == 0) {
%><div class=content><%=getLocString("dl.noPrincipalsFound")%></div><%
} else {
%>
<div class=content>
    <script>
        function setSort(sorting)
        {
            if (sorting == "<%=bean.getSort()%>")
                opForm.sort.value = "-<%=bean.getSort()%>";
            else
                opForm.sort.value = sorting;
            opForm.submit();
            return false;
        }
    </script>

    <table class=list cellspacing=0>
        <col width="1%">
        <col width="99%" align=left>
        <thead>
            <tr>
                <th>&nbsp;</th>
                <th>
                    <a href="javascript:setSort('address')" <%=bean.getSort().endsWith("address") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("common.sortmodes.addressHint")%>"><%=getLocString("common.sortmodes.address")%></a>
                </th>
                <th>
                    <a href="javascript:setSort('max_lst')" <%=bean.getSort().endsWith("max_lst") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("common.sortmodes.maxListsHint")%>"><%=getLocString("common.sortmodes.maxLists")%></a>
                </th>
                <th>
                    <a href="javascript:setSort('max_el')" <%=bean.getSort().endsWith("max_el") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%>
                       title="<%=getLocString("common.sortmodes.maxElementsHint")%>"><%=getLocString("common.sortmodes.maxElements")%></a>
                </th>
            </tr>
        </thead>
        <tbody><%
            int row = 0;
            for (Iterator i = bean.getPrincipals().iterator(); i.hasNext();) {
                Principal principal = (Principal) i.next();
                String address = principal.getAddress();
                Mask mask = new Mask(address);
                String addressEnc = StringEncoderDecoder.encode(address);
                String addressToShowEnc = StringEncoderDecoder.encode(mask.getMask());
                int max_lst = principal.getMaxLists();
                int max_el = principal.getMaxElements();
        %>
            <tr class=row<%=(row++) & 1%>>
                <td><input class=check type=checkbox name=checked
                           value="<%=addressEnc%>" <%=bean.isChecked(address) ? "checked" : ""%>></td>
                <td><a href="javascript:editSomething('<%=addressEnc%>')"
                       title="<%=getLocString("dl.editPrincipalTitle")%>"><%=addressToShowEnc%></a></td>
                <td><%=max_lst%></td>
                <td><%=max_el%></td>
            </tr><%
            }
        %>
        </tbody>
    </table>
    <%@ include file="/WEB-INF/inc/navbar_nofilter.jsp" %>
</div>
<%
    }
    page_menu_begin(out);
    page_menu_button(session, out, "mbAdd", "dl.addPrincipal", "dl.addPrincipalHint");
    page_menu_confirm_button(session, out, "mbDelete", "dl.deletePrincipal", "dl.deletePrincipalHint", getLocString("dl.deletePrincipalConfirm"));
    page_menu_space(out);
    page_menu_end(out);
} else { %>
<table>
    <tr><td>&nbsp</td></tr>
</table>
<%
        page_menu_begin(out);
        page_menu_button(session, out, "mbAdd", "dl.addPrincipal", "dl.addPrincipalHint");
        page_menu_space(out);
        page_menu_end(out);
    }
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp" %>
<%@ include file="/WEB-INF/inc/code_footer.jsp" %>