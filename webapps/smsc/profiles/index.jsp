<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Profiles List";
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
	case Index.RESULT_FILTER:
		response.sendRedirect("profilesFilter.jsp");
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("profilesAdd.jsp");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("profilesEdit.jsp?mask="+URLEncoder.encode(bean.getProfileMask()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>

<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=profileMask>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>

<script>
function editProfile(profileMask)
{
	document.all.jbutton.name = "mbEdit";
	opForm.profileMask.value = profileMask;
	opForm.submit();
	return false;
}
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
<table class=secRep cellspacing=1 width="100%">
<col width="60%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<thead>
<tr>
	<th><a href="#" <%=bean.getSort().endsWith("mask")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by mask" onclick='return setSort("mask")'>mask</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("codeset")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by codepage" onclick='return setSort("codeset")'>codepage</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("reportinfo") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by report info" onclick='return setSort("reportinfo")'>report&nbsp;info</a></th>
</tr>
</thead>
<tbody>
<%{
int row = 0;
for(Iterator i = bean.getProfiles().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
%>
<tr class=row<%=row&1%>>
	<td class=name><%
		if (bean.isEditAllowed())
		{
			%><a href="#" title="Edit profile" onClick='return editProfile("<%=(String)item.getValue("Mask")%>")'><%=StringEncoderDecoder.encode((String)item.getValue("Mask"))%></a><%
		}
		else
		{
			%><%=StringEncoderDecoder.encode((String)item.getValue("Mask"))%><%
		}%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Codepage"))%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Report info"))%></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar.jsp"%>
<%
if (bean.isEditAllowed())
{
	%><div class=secButtons>
	<input class=btn type=submit name=mbAdd value="Add profile" title="Add profile">
	</div><%
}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
