<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsAdd"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.subjects.SubjectsAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case SubjectsAdd.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case SubjectsAdd.RESULT_OK:
		STATUS.append("Ok");
		break;
	case SubjectsAdd.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Add subject</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
	<th>name:</th>
	<td><input class=txtW name=name value="<%=bean.getName()%>"></td>
	<td>&nbsp;</td>
</tr>
<tr class=row0>
	<th>default SME:</th>
	<td><select name=defSme>
		<%for (Iterator i = bean.getPossibleSmes().iterator(); i.hasNext(); )
		{
			String smeId = (String) i.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value=<%=encSmeId%> <%=smeId.equals(bean.getDefSme()) ? "selected" : ""%>><%=encSmeId%></option><%
		}%>
	</select></td>
	<td>&nbsp;</td>
</tr>
<%
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row0>
	<th>mask:</th>
	<td><input class=txtW name=masks value="<%=bean.getMasks()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=rowLast>
	<th>mask:</th>
	<td><input class=txtW name=masks></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to subject"></td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbSave value="Add subject" title="Add new subject">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>