<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter,
					  ru.novosoft.smsc.jsp.util.tables.impl.route.RouteFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Filter routes";
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case RoutesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case RoutesFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%int rowN = 0;%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secList>Sources</div></td></tr>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input class=check id="src_chk_<%=encName%>" type=checkbox name=srcChks value="<%=encName%>" <%=bean.isSrcChecked(name) ? "checked" : ""%>>&nbsp;<label for="src_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}
for (int i=0; i<bean.getSrcMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to sources filter"></th>
	<td><input class=txtW name=srcMasks validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secList>Destinations</div></td></tr>
<%
rowN = 0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input class=check id="dst_chk_<%=encName%>" type=checkbox name=dstChks value="<%=encName%>" <%=bean.isDstChecked(name) ? "checked" : ""%>>&nbsp;<label for="dst_chk_<%=encName%>"><%=encName%></label></td>
</tr>
<%}
for (int i=0; i<bean.getDstMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td><input class=txtW name=dstMasks value="<%=bean.getDstMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to destinations filter"></th>
	<td><input class=txtW name=dstMasks validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMEs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secList>SMEs</div></td></tr>
<%
rowN = 0;
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input class=check id="sme_<%=encName%>" type=checkbox name=smeChks value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>>&nbsp;<label for="sme_<%=encName%>"><%=encName%></label></td>
</tr>
<%}%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secView>Options</div></td></tr>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input type="checkbox" name=strict <%=bean.isStrict() ? "checked" : ""%>>Strict Filter</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>&nbsp;</th>
	<td nowrap><input type="checkbox" name=showSrc <%=bean.isShowSrc() ? "checked" : ""%>>Show sources list</td>
</tr>
<tr class=rowLast>
	<th class=label>&nbsp;</th>
	<td nowrap><input type="checkbox" name=showDst <%=bean.isShowDst() ? "checked" : ""%>>Show destinations list</td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbClear value="Clear" title="Clear filter">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>