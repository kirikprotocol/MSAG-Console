<%@ page import="ru.novosoft.smsc.admin.profiler.Profile,
                 java.util.Iterator,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<input type=hidden name=returnPath value="<%=bean.getReturnPath()%>">
<div class=content>
<div class=page_subtitle>Profile information</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr class=row0>
	<th>mask</th>
	<td><input class=txt name="mask" value="<%=bean.getMask() == null || bean.getMask().trim().length() == 0 ? ".0.0.0" : bean.getMask()%>" <%=isEdit ? "readonly" : ""%> validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row1>
	<th>codepage</th>
	<td><div class=select><select class=txt name=codepage>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=bean.getByteCodepage() == Profile.CODEPAGE_Default ? "SELECTED" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_Latin1 %>" <%=bean.getByteCodepage() == Profile.CODEPAGE_Latin1    ? "SELECTED" : ""%>>Latin1</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=bean.getByteCodepage() == Profile.CODEPAGE_UCS2    ? "SELECTED" : ""%>>UCS2</option>
			<option value="<%=Profile.CODEPAGE_UCS2AndLatin1%>" <%=bean.getByteCodepage() == Profile.CODEPAGE_UCS2AndLatin1    ? "SELECTED" : ""%>>UCS2&Latin1</option>
		</select>
    <input id=ussd7bitCheckbox class=check type=checkbox type="checkbox" name="ussd7bit" value="true"  <%=bean.isUssd7bit() ? "checked" : ""%>><label for=ussd7bitCheckbox>USSD in 7-bit</label>
    </div>
  </td>
</tr>
<tr class=row0>
	<th>report mode</th>
	<td><div class=select><select class=txt name="report">
			<option value="<%=Profile.REPORT_OPTION_None %>" <%=bean.getByteReport() == Profile.REPORT_OPTION_None  ? "SELECTED" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Final%>" <%=bean.getByteReport() == Profile.REPORT_OPTION_Final ? "SELECTED" : ""%>>final only</option>
			<option value="<%=Profile.REPORT_OPTION_Full %>" <%=bean.getByteReport() == Profile.REPORT_OPTION_Full  ? "SELECTED" : ""%>>full</option>
		</select></div></td>
</tr>
<tr class=row1>
	<th>locale</th>
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
	<th>alias</th>
	<td>
    <input class=check type=radio type="radio" name="aliasHide" value="true"  <%= (bean.isAliasHide()) ? "checked":""%>>Hide
    <input class=check type=radio type="radio" name="aliasHide" value="false" <%= (bean.isAliasHide()) ? "":"checked"%>>Unhide
    <input class=check type=checkbox type="checkbox" name="aliasModifiable" value="true" <%= (bean.isAliasModifiable()) ? "checked":""%>>Modifiable
  </td>
</tr>
<tr class=row1>
	<th>divert</th>
	<td><input class=txt name="divert" value="<%=bean.getDivert()%>" validation="address" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th><label for=divertActive>divert active</label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertActive name="divertActive" value="true" <%= (bean.isDivertActive()) ? "checked":""%>></td>
</tr>
<tr class=row1>
	<th><label for=divertModifiable>divert modifiable</label></th>
	<td><input class=check type=checkbox type="checkbox" id=divertModifiable name="divertModifiable" value="true" <%= (bean.isDivertModifiable()) ? "checked":""%>></td>
</tr>
</table>
</div>