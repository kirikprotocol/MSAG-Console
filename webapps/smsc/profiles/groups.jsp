<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.Groups,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.net.URLEncoder,
					  java.util.Iterator,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.Groups"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("profiles.groupsTitle");
switch(bean.process(request))
{
	case Groups.RESULT_DONE:
		response.sendRedirect("groups.jsp");
		return;
	case Groups.RESULT_OK:
		break;
	case Groups.RESULT_ERROR:
		break;
	case Groups.RESULT_FILTER:
		response.sendRedirect("profilesFilter.jsp");
		return;
	case Groups.RESULT_ADD:
		response.sendRedirect("profilesAdd.jsp?returnPath=groups");
		return;
	case Groups.RESULT_EDIT:
		response.sendRedirect("profilesEdit.jsp?returnPath=groups&mask="+URLEncoder.encode(bean.getProfileMask(), "UTF-8"));
		return;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES_GROUPS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>

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
<table class=list cellspacing=1 width="100%">
<col width="1%">
<col width="40%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<col width="20%" align=left>
<thead>
<tr>
	<th><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="#" <%=bean.getSort().endsWith("mask")          ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.maskHint")%>"              onclick='return setSort("mask")'         ><%=getLocString("common.sortmodes.mask")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("codeset")       ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.codepageHint")%>"          onclick='return setSort("codeset")'      ><%=getLocString("common.sortmodes.codepage")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("reportinfo")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.reportInfoHint")%>"        onclick='return setSort("reportinfo")'   ><%=getLocString("common.sortmodes.reportInfo")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("locale")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.localeHint")%>"            onclick='return setSort("locale")'       ><%=getLocString("common.sortmodes.locale")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("hidden")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.hiddenHint")%>"            onclick='return setSort("hidden")'       ><%=getLocString("common.sortmodes.hidden")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("hidden_mod")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.modifiableHint")%>"        onclick='return setSort("hidden_mod")'   ><%=getLocString("common.sortmodes.modifiable")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("divert")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.divertHint")%>"            onclick='return setSort("divert")'       ><%=getLocString("common.sortmodes.divert")%></a></th>
	<th><%=getLocString("common.sortmodes.divertActive")%></th>
	<th><a href="#" <%=bean.getSort().endsWith("divert_mod")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.divertModifiableHint")%>"  onclick='return setSort("divert_mod")'   ><%=getLocString("common.sortmodes.divertModifiable")%></a></th>
	<th><a href="#" <%=bean.getSort().endsWith("ussd7bit")      ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.ussd7bitHint")%>"          onclick='return setSort("ussd7bit")'     ><%=getLocString("common.sortmodes.ussd7bit")%></a></th>
</tr>
</thead>
<tbody>
<%! void appendDivertActiveFlag(StringBuffer divertActive, boolean value, String valueTitle)
{
  if (value)
  {
    if (divertActive.length() >0)
      divertActive.append(", ");
    divertActive.append(valueTitle);
  }
}
%><%{
int row = 0;
for(Iterator i = bean.getProfiles().iterator(); i.hasNext(); row++)
{
DataItem item = (DataItem) i.next();
final String profileMask = (String)item.getValue("Mask");
final String encProfileMask = StringEncoderDecoder.encode(profileMask);
final String divert = (String)item.getValue("divert");
final String divertEnc = divert == null || divert.length() == 0 ? "&nbsp;" : StringEncoderDecoder.encode(divert);
StringBuffer divertActive = new StringBuffer();
appendDivertActiveFlag(divertActive, ((Boolean)item.getValue("divert_actUnc" )).booleanValue(), "unconditional");
appendDivertActiveFlag(divertActive, ((Boolean)item.getValue("divert_actAbs" )).booleanValue(), "absent");
appendDivertActiveFlag(divertActive, ((Boolean)item.getValue("divert_actBlo" )).booleanValue(), "blocked");
appendDivertActiveFlag(divertActive, ((Boolean)item.getValue("divert_actBar" )).booleanValue(), "barred");
appendDivertActiveFlag(divertActive, ((Boolean)item.getValue("divert_actCap" )).booleanValue(), "capacity");
if (divertActive.length() == 0)
  divertActive.append("&nbsp;");
%>
<tr class=row<%=row&1%>>
	<td><input class=check type=checkbox name=checked value="<%=encProfileMask%>" <%=bean.isProfileCheked(profileMask) ? "checked" : ""%>></td>
	<td class=name><%
		if (bean.isEditAllowed())
		{
			%><a href="#" title="<%=getLocString("profiles.editSubTitle")%>" onClick='return editProfile("<%=profileMask%>")'><%=encProfileMask%></a><%
		}
		else
		{
			%><%=encProfileMask%><%
		}%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Codepage"))%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("Report info"))%></td>
	<td><%=StringEncoderDecoder.encode((String)item.getValue("locale"))%></td>
  <td align=center><%=item.getValue("alias hide")%></td>
  <td align=center><%if (((Boolean)item.getValue("hidden_mod"    )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
  <td><%=divertEnc%></td>
  <td><%=divertActive%></td>
  <td align=center><%if (((Boolean)item.getValue("divert_mod"    )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
	<td align=center><%if (((Boolean)item.getValue("ussd7bit"      )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
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
	page_menu_button(session, out, "mbAdd",  "profiles.add",  "profiles.addHint");
	page_menu_button(session, out, "mbDelete", "common.buttons.delete", "profiles.deleteHint", "return confirm('"+getLocString("profiles.deleteConfirm")+"');");
	page_menu_space(out);
	page_menu_end(out);
}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
