<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.profiler.Profile"%>
<%
	String addressString = request.getParameter("mask");
	if (addressString == null || addressString.length() == 0)
		throw new AdminException("Mask parameter not defined");
	
	Mask address = new Mask(addressString);
	byte codePage = Byte.decode(request.getParameter("codepage")).byteValue();
	byte reportOptions = Byte.decode(request.getParameter("report")).byteValue();
	boolean isAdd = Boolean.valueOf(request.getParameter("isAdd")).booleanValue();
	
	Profile profile = new Profile(address, codePage, reportOptions);
	%><h4>Profile: <%=profile.getMask().getMask()%></h4><%

	switch (smsc.updateProfile(address, profile))
	{
		case 1:	//pusUpdated
			%>Changes applied<%
			break;
		case 2: //pusInserted
			%>New profile inserted<%
			break;
		case 3: //pusUnchanged
			if (isAdd)
			{
				%><div style="color:red">Profile not added because identical to default profile</div><%
			}
			else
			{
				%>Profile not changed<%
			}
			break;
		case 4: //pusError
			%><div style="color:red">Error</div><%
			break;
		default: 
			%><div style="color:red">Unknown response</div><%
	}
	%>
<table class="list" cellspacing="0">
	<tr class="list">
		<th class="list" width="33%">Mask</th>
		<td class="list" width="33%"><%=profile.getMask().getMask()%></td>
	</tr>
	<tr class="list">
		<th class="list" width="33%">Report mode</th>
		<td class="list" width="33%"><%=profile.getReportOptionsString()%></td>
	</tr>
	<tr class="list">
		<th class="list">CodePage</th>
		<td class="list"><%=profile.getCodepageString()%></td>
	</tr>
</table>
<%@ include file="/common/footer.jsp"%>