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
TITLE = "Profiles list";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_REFRESH:
		response.sendRedirect("index.jsp?refreshed=true&initialized=" + bean.isInitialized() + "&startPosition=" + bean.getStartPositionInt() + "&filterMask=" + URLEncoder.encode(bean.getFilterMask()));
		return;
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
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
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
Profile prefix <input class=txt name=filterMask value="<%=bean.getFilterMask()%>" validation="address_prefix" onkeyup="resetValidation(this)">
</div>
<%
	page_menu_begin(out);
	page_menu_button(out, "mbQuery",  "Query",  "Query");
	page_menu_space(out);
	page_menu_end(out);
	if (bean.isQueried())
	{
%>
<div class=content>
<!--div class=page_subtitle>Query result</div-->
<table class=list cellspacing=0>
<col width="1%">
<col width="94%" align=left>
<col width="1%" align=left>
<col width="1%" align=left>
<col width="1%" align=left>
<col width="1%" align=left>
<col width="1%" align=left>
<thead>
<tr>
	<th><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("mask")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by mask"        onclick='return setSort("mask")'      >mask</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("codeset")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by codepage"    onclick='return setSort("codeset")'   >codepage</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("reportinfo") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by report info" onclick='return setSort("reportinfo")'>report&nbsp;info</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("locale")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by locale"      onclick='return setSort("locale")'    >locale</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("hidden")     ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by locale"      onclick='return setSort("hidden")'    >hidden</a></th>
	<th><a href="#" <%=bean.getSort().endsWith("hidden_mod") ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="Sort by locale"      onclick='return setSort("hidden_mod")'>modifiable</a></th>
</tr>
</thead>
<tbody>
<%
		{
			int row = 0;
			for(Iterator i = bean.getProfiles().iterator(); i.hasNext(); row++)
			{
				DataItem item = (DataItem) i.next();
				final String profileMask = (String)item.getValue("Mask");
				final String encProfileMask = StringEncoderDecoder.encode(profileMask);%>
				<tr class=row<%=row&1%>>
					<td><input class=check type=checkbox name=checked value="<%=encProfileMask%>" <%=bean.isProfileCheked(profileMask) ? "checked" : ""%> onclick="checkCheckboxes();"></td>
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
					<td align=center><%if (((Boolean)item.getValue("hidden"    )).booleanValue()){%><img src="<%=CPATH%>/img/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
					<td align=center><%if (((Boolean)item.getValue("hidden_mod")).booleanValue()){%><img src="<%=CPATH%>/img/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
				</tr><%
			}
		}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%
		if (bean.isEditAllowed())
		{
			page_menu_begin(out);
			page_menu_button(out, "mbAdd",  "Add profile",  "Add new profile");
			page_menu_button(out, "mbDelete", "Delete", "Delete checked profiles", "return confirm('Are you sure to delete all checked profiles?');", false);
			page_menu_space(out);
			page_menu_end(out);
		}
	}%>
<script>
function checkCheckboxes()
{
	<%
		if (bean.isQueried() && bean.isEditAllowed())
		{%>
			var inputs = opForm.getElementsByTagName("input");
			var disabledDelete = true;
			for (i = 0; i < inputs.length; i++)
			{
				var inp = inputs[i];
				if (inp.type == "checkbox")
					disabledDelete &= !inp.checked;
			}
			mbDelete.disabled = disabledDelete;<%
		}%>
}
checkCheckboxes();
</script>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
