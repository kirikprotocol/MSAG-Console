<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.Lookup,
					  ru.novosoft.smsc.jsp.util.tables.DataItem,
					  ru.novosoft.smsc.admin.profiler.ProfileEx,
					  java.net.URLEncoder,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.Lookup"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Lookup profile";
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
abonent number <input class=txt name=profile value="<%=bean.getProfile() != null ? bean.getProfile() : ""%>" validation="mask" onkeyup="resetValidation(this)">
</div>
	<%
	page_menu_begin(out);
	page_menu_button(out, "mbLookup",  "Lookup",  "Lookup profile");
	page_menu_space(out);
	page_menu_end(out);

	if (bean.getProfile() != null && beanResult == Lookup.RESULT_OK)
	{
	int rowN = 0;%>
	<div class=content>
	<div class=query_result><%
		switch (bean.getMatchType()) {
			case ProfileEx.MATCH_DEFAULT:
				out.print("default profile matched");
				break;
			case ProfileEx.MATCH_EXACT:
				out.print("profile matched exactly");
				break;
			case ProfileEx.MATCH_MASK:
				out.print("mask &quot;" + bean.getMatchAddress().getNormalizedMask() + "&quot; matched");
				break;
			default:
				out.print("unknown");
		}

    if (bean.getProfileDealiased() != null)
    {
      %><br>alias <%=bean.getProfile()%> dealiased to <%=bean.getProfileDealiased()%><%
    }
    if (bean.getMatchType() == ProfileEx.MATCH_EXACT && bean.getProfileAliased() != null)
    {
      %><br>profile <%=bean.getMatchAddress().getMask()%> has alias <%=bean.getProfileAliased()%><br><%
    }
		%>
  </div>
	<br>
	<table class=properties_list cellspacing=0>
	<col width="15%">
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>code page</th>
		<td><%=bean.getCodepage()%></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
    <th nowrap>USSD 7 bit</th>
    <td><img src="/images/ic_<%=bean.isUssd7bit() ? "" : "not_"%>checked.gif"></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>report options</th>
		<td><%=bean.getReportOptions()%></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>locale</th>
		<td><%=bean.getLocale()%>&nbsp;</td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>alias hide</th>
		<td><img src="/images/ic_<%=bean.isAliasHide() ? "" : "not_"%>checked.gif"></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>alias modifiable</th>
		<td><img src="/images/ic_<%=bean.isAliasModifiable() ? "" : "not_"%>checked.gif"></td>
	</tr>
	<tr class=row<%=(rowN++)&1%>>
		<th nowrap>divert</th>
		<td><%=bean.getDivert()%>&nbsp;</td>
	</tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert active unconditional</th>
    <td><img src="/images/ic_<%=bean.isDivertActiveUnconditional() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert active absent</th>
    <td><img src="/images/ic_<%=bean.isDivertActiveAbsent() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert active blocked</th>
    <td><img src="/images/ic_<%=bean.isDivertActiveBlocked() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert active barred</th>
    <td><img src="/images/ic_<%=bean.isDivertActiveBarred() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert active capacity</th>
    <td><img src="/images/ic_<%=bean.isDivertActiveCapacity() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>divert modifiable</th>
    <td><img src="/images/ic_<%=bean.isDivertModifiable() ? "" : "not_"%>checked.gif"></td>
  </tr>
  <tr class=row<%=(rowN++)&1%>>
    <th nowrap>UDH concatenate</th>
    <td><img src="/images/ic_<%=bean.isUdhConcat() ? "" : "not_"%>checked.gif"></td>
  </tr>
	</table>
	</div>
	<%
	page_menu_begin(out);
	switch (bean.getMatchType())
	{
		case ProfileEx.MATCH_MASK:
			page_menu_button(out, "mbAdd",        "Add profile",  "Add new profile");
			page_menu_button(out, "mbEditMask",   "Edit mask",    "Edit matched mask");
			page_menu_button(out, "mbDeleteMask", "Delete mask",  "Delete matched mask", "return confirm('Are you sure to delete matched mask?');");
			break;
		case ProfileEx.MATCH_DEFAULT:
			page_menu_button(out, "mbAdd",      "Add profile",  "Add new profile");
			break;
		case ProfileEx.MATCH_EXACT:
			page_menu_button(out, "mbEdit",   "Edit",   "Edit this profile");
			page_menu_button(out, "mbDelete", "Delete", "Delete this profile", "return confirm('Are you sure to delete this profile?');");
			break;
	}
	page_menu_space(out);
	page_menu_end(out);
	}
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
