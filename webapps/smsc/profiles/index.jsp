<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.Index,
                 ru.novosoft.smsc.jsp.util.tables.DataItem,
					  java.net.URLEncoder,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Iterator"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Profiles";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
		response.sendRedirect("profilesAdd.jsp?returnPath=profiles");
		return;
	case Index.RESULT_EDIT:
		response.sendRedirect("profilesEdit.jsp?returnPath=profiles&mask="+URLEncoder.encode(bean.getProfileMask()));
		return;
	default:
		STATUS.append("<span class=CF00>Error</span>");
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>

<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=profileMask>
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=initialized value=true>

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
<div class=page_subtitle>Filter</div>
<table class=properties_list cellspacing=0>
<col width="10%">
<col width="90%">
<%
	int row = 0;
	for (int i=0; i<bean.getMasks().length; i++)
	{
%>
<tr class=row<%=(row++)&1%>>
	<td><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
	<td>&nbsp</td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAddMask", "Add", "Add new mask to filter");%></td>
</tr>
</table>
<div class=page_subtitle>Query result</div>
<table class=list cellspacing=0>
<col width="1%">
<col width="40%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<thead>
<tr>
	<th><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("mask")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by mask"        onclick='return setSort("mask")'      >mask</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("codeset")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by codepage"    onclick='return setSort("codeset")'   >codepage</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("reportinfo") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by report info" onclick='return setSort("reportinfo")'>report&nbsp;info</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("locale")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by locale"      onclick='return setSort("locale")'    >locale</a></th>
</tr>
</thead>
<tbody>
<%{
row = 0;
for(Iterator i = bean.getProfiles().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
final String profileMask = (String)item.getValue("Mask");
final String encProfileMask = StringEncoderDecoder.encode(profileMask);
%>
<tr class=row<%=row&1%>>
	<td><input class=check type=checkbox name=checked value="<%=encProfileMask%>" <%=bean.isProfileCheked(profileMask) ? "checked" : ""%>></td>
	<td class=name><%
	if (bean.isEditAllowed())
		{
			%><a href="#" title="Edit profile" onClick='return editProfile("<%=profileMask%>")'><%=encProfileMask%></a><%
		}
		else
		{
			%><%=encProfileMask%><%
		}%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Codepage"))%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Report info"))%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("locale"))%></td>
</tr>
<%}}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%
if (bean.isEditAllowed())
{
	page_menu_begin(out);
	page_menu_button(out, "mbReQuery",  "Requery",  "Requery");
	page_menu_button(out, "mbAdd",  "Add profile",  "Add new profile");
	page_menu_button(out, "mbDelete", "Delete", "Delete checked profiles", "return confirm('Are you sure to delete all checked profiles?');");
	page_menu_space(out);
	page_menu_end(out);
}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
