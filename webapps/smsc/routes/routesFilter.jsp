<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter,
                 ru.novosoft.smsc.jsp.util.tables.impl.RouteFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
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
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Routes filter</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="10%" align=right>
<col width="85%">
<col width="10%">
<%int rowN = 0;%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan="3">sources</th>
</tr>
<%
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="src_chk_<%=encName%>" type=checkbox name=srcChks value="<%=encName%>" <%=bean.isSrcChecked(name) ? "checked" : ""%>></td>
	<td><label for="src_chk_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}
for (int i=0; i<bean.getSrcMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txtW name=srcMasks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to sources filter"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan="3">destinations</th>
</tr>
<%
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="dst_chk_<%=encName%>" type=checkbox name=dstChks value="<%=encName%>" <%=bean.isDstChecked(name) ? "checked" : ""%>></td>
	<td><label for="dst_chk_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}
for (int i=0; i<bean.getDstMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txtW name=dstMasks value="<%=bean.getDstMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txtW name=dstMasks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to destinations filter"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SMEs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan="3">SMEs</th>
</tr>
<%
for (Iterator i = bean.getAllSmes().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check id="sme_<%=encName%>" type=checkbox name=smeChks value="<%=encName%>" <%=bean.isSmeChecked(name) ? "checked" : ""%>></td>
	<td><label for="sme_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan="3">options</th>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<td><input type="checkbox" name=strict <%=bean.isStrict() ? "checked" : ""%>>Strict&nbsp;Filter</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<td><input type="checkbox" name=showSrc <%=bean.isShowSrc() ? "checked" : ""%>>Show&nbsp;sources&nbsp;list</td>
</tr>
<tr class=rowLast>
	<td><input type="checkbox" name=showDst <%=bean.isShowDst() ? "checked" : ""%>>Show&nbsp;destinations&nbsp;list</td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>