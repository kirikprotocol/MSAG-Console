<%@ page import="ru.novosoft.smsc.admin.profiler.Profile,
                 java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%--jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesBean"/--%>
<input type=hidden name=returnPath value="<%=bean.getReturnPath()%>">
<div class=content>
<div class=page_subtitle><%=getLocString("profiles.subTitle")%></div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row0>
	<th><%=getLocString("profiles.mask")%></th>
	<td><input class=txt name="mask" value="<%=bean.getMask() == null || bean.getMask().trim().length() == 0 ? ".0.0.0" : bean.getMask()%>" <%=isEdit ? "readonly" : ""%> validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row1>
	<th><%=getLocString("profiles.codepage")%></th>
	<td><div class=select><select class=txt name=codepage>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=bean.getByteCodepage() == Profile.CODEPAGE_Default ? "SELECTED" : ""%>><%=getLocString("profiles.codepage.default")%></option>
			<option value="<%=Profile.CODEPAGE_Latin1 %>" <%=bean.getByteCodepage() == Profile.CODEPAGE_Latin1    ? "SELECTED" : ""%>><%=getLocString("profiles.codepage.latin1")%></option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=bean.getByteCodepage() == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>><%=getLocString("profiles.codepage.ucs2")%></option>
			<option value="<%=Profile.CODEPAGE_UCS2AndLatin1%>" <%=bean.getByteCodepage() == Profile.CODEPAGE_UCS2AndLatin1    ? "SELECTED" : ""%>><%=getLocString("profiles.codepage.ucs2AndLatin1")%></option>
		</select>
    <input id=ussd7bitCheckbox class=check type=checkbox type="checkbox" name="ussd7bit" value="true"  <%=bean.isUssd7bit() ? "checked" : ""%>><label for=ussd7bitCheckbox><%=getLocString("profiles.ussd7bit")%></label>
    </div>
  </td>
</tr>
<tr class=row0>
	<th><%=getLocString("profiles.reportOptions")%></th>
	<td><div class=select><select class=txt name="report">
			<option value="<%=Profile.REPORT_OPTION_None %>" <%=bean.getByteReport() == Profile.REPORT_OPTION_None  ? "SELECTED" : ""%>><%=getLocString("profiles.reportOptions.none")%></option>
			<option value="<%=Profile.REPORT_OPTION_Final%>" <%=bean.getByteReport() == Profile.REPORT_OPTION_Final ? "SELECTED" : ""%>><%=getLocString("profiles.reportOptions.final")%></option>
			<option value="<%=Profile.REPORT_OPTION_Full %>" <%=bean.getByteReport() == Profile.REPORT_OPTION_Full  ? "SELECTED" : ""%>><%=getLocString("profiles.reportOptions.full")%></option>
		</select></div></td>
</tr>
<tr class=row1>
	<th><%=getLocString("profiles.locale")%></th>
	<td><div class=select><select class=txt name="locale">
		<%for (Iterator i = bean.getRegisteredLocales().iterator(); i.hasNext();)
		{
			String registeredLocale = (String) i.next();
			boolean selected = registeredLocale.equals(bean.getLocale());
			String encRegisteredLocale = StringEncoderDecoder.encode(registeredLocale);
			%>
			<option value="<%=encRegisteredLocale%>" <%=selected ? "SELECTED" : ""%>><%=encRegisteredLocale%></option><%
		}%>
		</select></div></td>
</tr>
<tr class=row0>
	<th><%=getLocString("profiles.aliasHide")%></th>
	<td>
    <input class=check type="radio" name="aliasHide" id="aliasHide_true" value="true"  <%= (bean.getAliasHideByte() == Profile.ALIAS_HIDE_true) ? "checked":""%>><label for="aliasHide_true"><%=getLocString("profiles.aliasHide.hide")%></label>
    <input class=check type="radio" name="aliasHide" id="aliasHide_false" value="false" <%= (bean.getAliasHideByte() == Profile.ALIAS_HIDE_false) ? "checked":""%>><label for="aliasHide_false"><%=getLocString("profiles.aliasHide.unhide")%></label>
    <input class=check type="radio" name="aliasHide" id="aliasHide_substitute" value="substitute" <%= (bean.getAliasHideByte() == Profile.ALIAS_HIDE_substitute) ? "checked":""%>><label for="aliasHide_substitute"><%=getLocString("profiles.aliasHide.substitute")%></label>
    <input class=check type="checkbox" name="aliasModifiable" id="aliasModifiable" value="true" <%= (bean.isAliasModifiable()) ? "checked":""%>><label for="aliasModifiable"><%=getLocString("profiles.aliasModifiable")%></label>
  </td>
</tr>
<tr class=row1>
	<th><%=getLocString("profiles.divert")%></th>
	<td><input class=txt name="divert" value="<%=bean.getDivert()%>" validation="address" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th><label for=divertActiveUnconditional><%=getLocString("profiles.divertActiveUnconditional")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActiveUnconditional name="divertActiveUnconditional" value="true" <%= (bean.isDivertActiveUnconditional()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=divertActiveAbsent><%=getLocString("profiles.divertActiveAbsent")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActiveAbsent name="divertActiveAbsent" value="true" <%= (bean.isDivertActiveAbsent()) ? "checked":""%>></td>
</tr>
<tr class=row0>
	<th><label for=divertActiveBlocked><%=getLocString("profiles.divertActiveBlocked")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActiveBlocked name="divertActiveBlocked" value="true" <%= (bean.isDivertActiveBlocked()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=divertActiveBarred><%=getLocString("profiles.divertActiveBarred")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActiveBarred name="divertActiveBarred" value="true" <%= (bean.isDivertActiveBarred()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=divertActiveCapacity><%=getLocString("profiles.divertActiveCapacity")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActiveCapacity name="divertActiveCapacity" value="true" <%= (bean.isDivertActiveCapacity()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=divertModifiable><%=getLocString("profiles.divertModifiable")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertModifiable name="divertModifiable" value="true" <%= (bean.isDivertModifiable()) ? "checked":""%>></td>
</tr>
<tr class=row0>
	<th><label for=udhConcat><%=getLocString("profiles.udhConcatenate")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=udhConcat name="udhConcat" value="true" <%= (bean.isUdhConcat()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=translit><%=getLocString("profiles.translit")%></label></th>
	<td><input class=check type=checkbox type="checkbox" id=translit name="translit" value="true" <%= (bean.isTranslit()) ? "checked":""%>></td>
</tr>
</table>
</div>