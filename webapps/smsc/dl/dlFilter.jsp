<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.jsp.dl.dlFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
    TITLE="Distribution lists: Filter";
    MENU0_SELECTION = "MENU0_DL";

    int beanResult = bean.RESULT_OK;
    switch(beanResult = bean.process(appContext, errorMessages))
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
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Names ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Names</div></td></tr>
<%
int rowN = 0;
for (int i=0; i<bean.getNames().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label></th>
	<td><input class=txtW name=names value="<%=bean.getNames()[i]%>"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new distribution list name to filter.\nUses java regexp format"></th>
	<td><input class=txtW name=names></td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply  value="Apply" title="Apply filter to distribution lists list">
<input class=btn type=submit name=mbClear  value="Clear" title="Clear filter">
<input class=btn type=submit name=mbCancel value="Cancel" title="Cancel filter editing">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>