<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.routes.RoutesAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages, request.getParameterMap()))
{
	case RoutesAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case RoutesAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case RoutesAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}

int rowN = 0;
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ROUTES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Add route</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th>name:</th>
	<td><input class=txtW name=routeId value="<%=bean.getRouteId()%>"></td>
	<td>&nbsp;</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>is premissible:</th>
	<td><input class=txtW type=checkbox name=permissible <%=bean.isPermissible() ? "checked" : ""%>></td>
	<td>&nbsp;</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>billing:</th>
	<td><input class=txtW type=checkbox name=billing <%=bean.isBilling() ? "checked" : ""%>></td>
	<td>&nbsp;</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>archiving:</th>
	<td><input class=txtW type=checkbox name=archiving <%=bean.isArchiving() ? "checked" : ""%>></td>
	<td>&nbsp;</td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan=3>&nbsp;</th>
</tr>
<%
boolean isFirstSrc = true;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<%if (isFirstSrc) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getSrcMasks().length +1%>>source:</th><%isFirstSrc = false;}%>
	<td><input class=check id="subj_src_<%=encName%>" type=checkbox name=checkedSources value="<%=encName%>" <%=bean.isSrcChecked(name) ? "checked" : ""%>><label for="subj_src_<%=encName%>"><%=encName%></label></td>
	<td>&nbsp;</td>
</tr>
<%}
for (int i=0; i<bean.getSrcMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<%if (isFirstSrc) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getSrcMasks().length +1%>>source:</th><%isFirstSrc = false;}%>
	<td><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<%if (isFirstSrc) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getSrcMasks().length +1%>>source:</th><%isFirstSrc = false;}%>
	<td><input class=txtW name=srcMasks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to sources"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row<%=(rowN++)&1%>>
	<th colspan=3>&nbsp;</th>
</tr>
<%
boolean isFirstDst = true;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<%if (isFirstDst) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getDstMasks().length +1%>>destination:</th><%isFirstDst = false;}%>
	<td>
		<input class=check id="subj_dst_<%=encName%>" type=checkbox name=checkedDestinations value="<%=encName%>" <%=bean.isDstChecked(name) ? "checked" : ""%>><label for="subj_dst_<%=encName%>"><%=encName%></label>
		<select name=dst_sme_<%=encName%> <%=!bean.isDstChecked(name) ? "disabled" : ""%>>
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=bean.isSmeSelected(name, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
	<td>&nbsp;</td>
</tr>
<%}
for (int i=0; i<bean.getDstMasks().length; i++)
{
String dstMask = bean.getDstMasks()[i];
String encMask = StringEncoderDecoder.encode(dstMask);
%>
<tr class=row<%=(rowN++)&1%>>
	<%if (isFirstDst) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getDstMasks().length +1%>>destination:</th><%isFirstDst = false;}%>
	<td><input class=txtW name=dstMasks value="<%=encMask%>">
	</td>
	<td><select name=dst_mask_sme_<%=encMask%>>
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=bean.isMaskSmeSelected(dstMask, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
</tr>
<%}%>
<tr class=rowLast>
	<%if (isFirstDst) {%><th rowspan=<%=bean.getAllSubjects().size() + bean.getDstMasks().length +1%>>destination:</th><%isFirstDst = false;}%>
	<td><input class=txtW name=dstMasks>
		<select name=dst_mask_sme_>
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getDst_mask_sme_()) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to destinations"></td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbSave value="Add route" title="Add new route">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>