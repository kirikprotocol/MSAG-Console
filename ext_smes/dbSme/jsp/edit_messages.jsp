<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditMessages"/>
<%@include file="header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditProvider"%>
<%
int result = UC.process(request, response);
if (result == UC.RESULT_Done)
{
	if (UC.getJobName() != null && UC.getJobName().length() > 0 && UC.getProviderName() != null && UC.getProviderName().length() > 0)
		response.sendRedirect("edit_provider.jsp?" + CCEditProvider.PARAM_Name + '=' + URLEncoder.encode(UC.getProviderName()));
	else if (UC.getProviderName() != null && UC.getProviderName().length() > 0)
		response.sendRedirect("edit_provider.jsp?" + CCEditProvider.PARAM_Name + '=' + URLEncoder.encode(UC.getProviderName()));
	else
		response.sendRedirect("index.jsp");
} else if (result == UC.RESULT_Error) 
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error:</div><%
}
%>

<%--!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<form method="post">
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Global messages</td>
</tr>
</table>
<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<col align=center width="85%">
<tbody>
<%
Map messages = UC.getMessages();
String pmt[] = UC.getPossibleMessageTypes();
for (int i = UC.getPossibleStartIndex(); i<pmt.length; i++)
{
	String name = pmt[i];
	String value = (String) messages.get(name);
	if (value == null)
		value = "";
	%><tr>
		<th><%=StringEncoderDecoder.encode(name)%></th>
		<td><input class="edit" type="Text" name="<%=StringEncoderDecoder.encode(UC.PARAM_Name + name)%>" value="<%=StringEncoderDecoder.encode(value)%>"></td>
	</tr><%
}
%>
</tbody>
</table>
<div align="right">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Save%>" value="Save">
	<input class="button" type="Reset" value="Reset">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Cancel%>" value="Cancel">
</div>
</form>
</body>
</html>
