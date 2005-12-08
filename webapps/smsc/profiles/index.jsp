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
TITLE = getLocString("profiles.title");
switch(bean.process(request))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		break;
	case Index.RESULT_REFRESH:
		response.sendRedirect("index.jsp?refreshed=true&initialized=" + bean.isInitialized() + "&startPosition=" + bean.getStartPositionInt() + "&filterMask=" + URLEncoder.encode(bean.getFilterMask()) + "&runQuery=" + bean.isRunQuery() + "&closeQuery=" + bean.isCloseQuery());
		return;
	case Index.RESULT_ERROR:
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
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>

<input type=hidden name=startPosition value="<%=bean.getStartPosition()%>">
<input type=hidden name=profileMask>
<input type="hidden" name="runQuery">
<input type="hidden" name="closeQuery">
<input type=hidden name=totalSize value=<%=bean.getTotalSize()%>>
<input type=hidden name=sort>
<input type=hidden name=initialized value=true>

<script>
function editProfile(profileMask)
{
	document.getElementById('jbutton').name = "mbEdit";
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
        document.getElementById('runQuery').value = false;
	opForm.submit();
	return false;
}
function runQuery(){
    document.getElementById('runQuery').value = true;
}
function closeQuery(){
    document.getElementById('closeQuery').value = true;
    document.getElementById('runQuery').value = false;
}
</script>
<%=getLocString("profiles.profilePrefix")%> <input class=txt name=filterMask value="<%=bean.getFilterMask()%>" validation="address_prefix" onkeyup="resetValidation(this)">
</div>
<%
	page_menu_begin(out);
	page_menu_button(session, out, "mbQuery",  "common.buttons.query",  "common.buttons.query", "onClick=runQuery()");
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
	<th><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th><a href="javascript:setSort('mask')" <%=bean.getSort().endsWith("mask")            ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.maskHint")%>"><%=getLocString("common.sortmodes.mask")%></a></th>
	<th><a href="javascript:setSort('codeset')" <%=bean.getSort().endsWith("codeset")      ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.codepageHint")%>"><%=getLocString("common.sortmodes.codepage")%></a></th>
	<th><a href="javascript:setSort('reportinfo')" <%=bean.getSort().endsWith("reportinfo")? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.reportInfoHint")%>"><%=getLocString("common.sortmodes.reportInfo")%></a></th>
	<th><a href="javascript:setSort('locale')" <%=bean.getSort().endsWith("locale")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.localeHint")%>"><%=getLocString("common.sortmodes.locale")%></a></th>
	<th><a href="javascript:setSort('hidden')" <%=bean.getSort().endsWith("hidden")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.hiddenHint")%>"><%=getLocString("common.sortmodes.hidden")%></a></th>
	<th><a href="javascript:setSort('hidden_mod')" <%=bean.getSort().endsWith("hidden_mod")? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.modifiableHint")%>"><%=getLocString("common.sortmodes.modifiable")%></a></th>
	<th><a href="javascript:setSort('divert')" <%=bean.getSort().endsWith("divert")        ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.divertHint")%>"><%=getLocString("common.sortmodes.divert")%></a></th>
	<th><%=getLocString("common.sortmodes.divertActive")%></th>
	<th><a href="javascript:setSort('divert_mod')" <%=bean.getSort().endsWith("divert_mod")? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.divertModifiableHint")%>"><%=getLocString("common.sortmodes.divertModifiable")%></a></th>
	<th><a href="javascript:setSort('ussd7bit')" <%=bean.getSort().endsWith("ussd7bit")    ? (bean.getSort().charAt(0) == '-' ? "class=up" : "class=down") : ""%> title="<%=getLocString("common.sortmodes.ussd7bitHint")%>"><%=getLocString("common.sortmodes.ussd7bit")%></a></th>
</tr>
</thead>
<tbody>
<%! void appendDivertActiveFlag(StringBuffer divertActive, boolean value, String valueTitle){
  if (value){
    if (divertActive.length() >0)
      divertActive.append(", ");
    divertActive.append(valueTitle);
  }
}
%><%
		{
            int firstIndex = bean.getStartPositionInt()+1;
            int lastIndex = bean.getStartPositionInt()+bean.getPageSizeInt();
            if (lastIndex >= bean.getTotalSizeInt() || bean.getPageSizeInt() < 0)
            	lastIndex = bean.getTotalSizeInt();
            int row = 0;
            for (int cnt=firstIndex; cnt<=lastIndex; cnt++, row++) {
               DataItem item = (DataItem)bean.getProfiles().get(cnt-1);
				final String profileMask = (String)item.getValue("mask");
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
					<td><input class=check type=checkbox name=checked value="<%=encProfileMask%>" <%=bean.isProfileCheked(profileMask) ? "checked" : ""%> onclick="checkCheckboxes();"></td>
					<td class=name><%
					if (bean.isEditAllowed())
						{
							%><a href="javascript:editProfile('<%=profileMask%>')" title="<%=getLocString("profiles.editSubTitle")%>"><%=encProfileMask%></a><%
						}
						else
						{
							%><%=encProfileMask%><%
						}%></td>
					<td><%=StringEncoderDecoder.encode((String)item.getValue("codeset"))%></td>
					<td><%=StringEncoderDecoder.encode((String)item.getValue("reportinfo"))%></td>
					<td><%=StringEncoderDecoder.encode((String)item.getValue("locale"))%></td>
					<td align=center><%=item.getValue("alias_hide")%></td>
					<td align=center><%if (((Boolean)item.getValue("hidden_mod"    )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
					<td><%=divertEnc%></td>
          <td><%=divertActive%></td>
					<td align=center><%if (((Boolean)item.getValue("divert_mod"    )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
					<td align=center><%if (((Boolean)item.getValue("ussd7bit"      )).booleanValue()){%><img src="/images/ic_checked.gif"><%}else{%>&nbsp;<%}%></td>
				</tr><%
			}
		}%>
</tbody>
</table>
<%@ include file="/WEB-INF/inc/navbar_nofilter.jsp"%>
</div>
<%      if (bean.isQueried()){
            page_menu_begin(out);
            page_menu_button(session, out, "mbClose",  "common.buttons.close",  "profiles.closeHint", "onClick=closeQuery()");
            page_menu_space(out);
	        page_menu_end(out);
        }
%>
<%
		if (bean.isEditAllowed())
		{
			page_menu_begin(out);
			page_menu_button(session, out, "mbAdd",  "profiles.add",  "profiles.addHint");
			page_menu_button(session, out, "mbDelete", "common.buttons.delete", "profiles.deleteHint", "return confirm('"+getLocString("profiles.deleteConfirm")+"');", false);
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
