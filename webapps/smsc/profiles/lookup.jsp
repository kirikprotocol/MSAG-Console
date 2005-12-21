<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.Lookup,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  ru.novosoft.smsc.admin.profiler.ProfileEx,
					  java.net.URLEncoder,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.Lookup"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("profiles.lookupTitle");
int beanResult = bean.process(request);
switch(beanResult)
{
	case Lookup.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Lookup.RESULT_OK:
		break;
	case Lookup.RESULT_ERROR:
		break;
	case Lookup.RESULT_ADD:
		response.sendRedirect("profilesAdd.jsp?returnPath=lookup&mask=" + URLEncoder.encode(bean.getProfileDealiased() != null ? bean.getProfileDealiased() : bean.getProfile()));
		return;
	case Lookup.RESULT_REFRESH:
		response.sendRedirect("lookup.jsp?mbRefreshed=true&profile=" + URLEncoder.encode(bean.getProfile() == null ? "" : bean.getProfile()));
		return;
	case Lookup.RESULT_EDIT:
		response.sendRedirect("profilesEdit.jsp?returnPath=lookup&mask=" + URLEncoder.encode(bean.getProfileDealiased() != null ? bean.getProfileDealiased() : bean.getProfile()));
		return;
	case Lookup.RESULT_EDIT_MASK:
		response.sendRedirect("profilesEdit.jsp?returnPath=lookup&mask=" + URLEncoder.encode(bean.getMatchAddress().getMask()));
		return;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES_LOOKUP";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<%=getLocString("profiles.abonentNumber")%>&nbsp;<input class=txt name=profile value="<%=bean.getProfile() != null ? bean.getProfile() : ""%>" validation="mask" onkeyup="resetValidation(this)">
</div>
	<%
	page_menu_begin(out);
	page_menu_button(session, out, "mbLookup",  "common.buttons.lookup",  "profiles.lookupHint");
	page_menu_space(out);
	page_menu_end(out);

	if (bean.getProfile() != null && beanResult == Lookup.RESULT_OK)
	{
	int rowN = 0;%>
	<div class=content>
	<div class=query_result><%
		switch (bean.getMatchType()) {
			case ProfileEx.MATCH_DEFAULT:
				out.print(getLocString("profiles.matchedResult.default"));
				break;
			case ProfileEx.MATCH_EXACT:
				out.print(getLocString("profiles.matchedResult.exact"));
				break;
			case ProfileEx.MATCH_MASK:
				out.print(getLocString("profiles.matchedResult.maskPre") + bean.getMatchAddress().getNormalizedMask() + getLocString("profiles.matchedResult.maskPost"));
				break;
			default:
				out.print(getLocString("profiles.matchedResult.unknown"));
		}

    if (bean.getProfileDealiased() != null)
    {
      %><br><%=getLocString("profiles.dealiasedPre") + bean.getProfile() + getLocString("profiles.dealiasedPost") + bean.getProfileDealiased() + "\""%><%
    }
    if (bean.getMatchType() == ProfileEx.MATCH_EXACT && bean.getProfileAliased() != null)
    {
      %><br><%=getLocString("profiles.aliasedPre") + bean.getMatchAddress().getMask() + getLocString("profiles.aliasedPost") + bean.getProfileAliased() + "\""%><br><%
    }
		%>
  </div>
	<br>
	<table class=properties_list cellspacing=0>
	<col width="15%">
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.codepage")%></th>
		<td><%=bean.getCodepage()%></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.ussd7bit")%></th>
    <td><img src="/images/ic_<%=bean.isUssd7bit() ? "" : "not_"%>checked.gif"></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.reportOptions")%></th>
		<td><%=bean.getReportOptions()%></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.locale")%></th>
		<td><%=bean.getLocale()%>&nbsp;</td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.aliasHide")%></th>
		<td><%
      switch (bean.getAliasHide()) {
        case Profile.ALIAS_HIDE_false: {
          %><img src="/images/ic_not_checked.gif"> <%=getLocString("profiles.aliasHide.unhide")%><%
        }
          break;
        case Profile.ALIAS_HIDE_true: {
          %><img src="/images/ic_checked.gif"> <%=getLocString("profiles.aliasHide.hide")%><%
        }
          break;
        case Profile.ALIAS_HIDE_substitute: {
          %><%=getLocString("profiles.aliasHide.substitute")%><%
        }
          break;
      }
		%></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.aliasModifiable")%></th>
		<td><img src="/images/ic_<%=bean.isAliasModifiable() ? "" : "not_"%>checked.gif"></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap><%=getLocString("profiles.divert")%></th>
		<td><%=bean.getDivert()%>&nbsp;</td>
	</tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertActiveUnconditional")%></th>
    <td><img src="/images/ic_<%=bean.isDivertActiveUnconditional() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertActiveAbsent")%></th>
    <td><img src="/images/ic_<%=bean.isDivertActiveAbsent() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertActiveBlocked")%></th>
    <td><img src="/images/ic_<%=bean.isDivertActiveBlocked() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertActiveBarred")%></th>
    <td><img src="/images/ic_<%=bean.isDivertActiveBarred() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertActiveCapacity")%></th>
    <td><img src="/images/ic_<%=bean.isDivertActiveCapacity() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.divertModifiable")%></th>
    <td><img src="/images/ic_<%=bean.isDivertModifiable() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.udhConcatenate")%></th>
    <td><img src="/images/ic_<%=bean.isUdhConcat() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap><%=getLocString("profiles.translit")%></th>
    <td><img src="/images/ic_<%=bean.isTranslit() ? "" : "not_"%>checked.gif"></td>
  </tr>
	</table>
	</div>
	<%
	page_menu_begin(out);
	switch (bean.getMatchType())
	{
		case ProfileEx.MATCH_MASK:
			page_menu_button(session, out, "mbAdd",        "profiles.add",      "profiles.addHint");
			page_menu_button(session, out, "mbEditMask",   "profiles.editMask", "profiles.editMaskHint");
			page_menu_confirm_button(session, out, "mbDeleteMask", "profiles.deleteMask",  "profiles.deleteMaskHint", getLocString("profiles.deleteMaskConfirm"));
			break;
		case ProfileEx.MATCH_DEFAULT:
			page_menu_button(session, out, "mbAdd",        "profiles.add",  "profiles.addHint");
			break;
		case ProfileEx.MATCH_EXACT:
			page_menu_button(session, out, "mbEdit",   "common.buttons.edit",   "profiles.editProfileHint");
			page_menu_confirm_button(session, out, "mbDelete", "common.buttons.delete", "profiles.deleteProfileHint", getLocString("profiles.deleteProfileConfirm"));
			break;
	}
	page_menu_space(out);
	page_menu_end(out);
	}
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
