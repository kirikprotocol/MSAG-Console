<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.dl.dlFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE="Distribution lists filter";
    MENU0_SELECTION = "MENU0_DL";

    int beanResult = bean.RESULT_OK;
    switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
    {
        case dlFilter.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case dlFilter.RESULT_OK:
            STATUS.append("Ok");
            break;
        case dlFilter.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<table class=list cellspacing=0>
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Names ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=page_subtitle>Names</div></td></tr>
<%
int rowN = 0;
for (int i=0; i<bean.getNames().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td>&nbsp;</td>
	<td><input class=txt name=names value="<%=bean.getNames()[i]%>"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new distribution list name to filter.\nUses java regexp format"></td>
	<td><input class=txt name=names></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter to distribution lists list");
page_menu_button(out, "mbClear", "Clear", "Clear filter");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>