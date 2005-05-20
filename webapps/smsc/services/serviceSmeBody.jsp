<%@ page import="ru.novosoft.smsc.admin.route.SME"%>
<col width="15%" align=right>
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.priority")%></th>
	<td><input class=txt type="text" name=priority maxlength=5 value="<%=bean.getPriority()%>" validation="priority" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.systemType")%></th>
	<td><input class=txt type="text" name="systemType" value="<%=StringEncoderDecoder.encode(bean.getSystemType())%>" maxlength="13"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.typeOfNumber")%></th>
	<td><div class=select>
		<select name="typeOfNumber">
		<option value="0"<%=bean.getTypeOfNumberInt() == 0 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.unknown")%></option>
		<option value="1"<%=bean.getTypeOfNumberInt() == 1 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.international")%></option>
		<option value="2"<%=bean.getTypeOfNumberInt() == 2 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.national")%></option>
		<option value="3"<%=bean.getTypeOfNumberInt() == 3 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.networkSpecific")%></option>
		<option value="4"<%=bean.getTypeOfNumberInt() == 4 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.subscriberNumber")%></option>
		<option value="5"<%=bean.getTypeOfNumberInt() == 5 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.alphanumeric")%></option>
		<option value="6"<%=bean.getTypeOfNumberInt() == 6 ? " selected" : ""%>><%=getLocString("services.typeOfNumber.abbreviated")%></option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.numberingPlan")%></th>
	<td><div class=select>
		<select name="numberingPlan">
		<option value="0" <%=bean.getNumberingPlanInt() == 0  ? " selected" : ""%>><%=getLocString("services.numberingPlan.unknown")%></option>
		<option value="1" <%=bean.getNumberingPlanInt() == 1  ? " selected" : ""%>><%=getLocString("services.numberingPlan.isdn")%></option>
		<option value="3" <%=bean.getNumberingPlanInt() == 3  ? " selected" : ""%>><%=getLocString("services.numberingPlan.data")%></option>
		<option value="4" <%=bean.getNumberingPlanInt() == 4  ? " selected" : ""%>><%=getLocString("services.numberingPlan.telex")%></option>
		<option value="6" <%=bean.getNumberingPlanInt() == 6  ? " selected" : ""%>><%=getLocString("services.numberingPlan.landMobile")%></option>
		<option value="8" <%=bean.getNumberingPlanInt() == 8  ? " selected" : ""%>><%=getLocString("services.numberingPlan.national")%></option>
		<option value="9" <%=bean.getNumberingPlanInt() == 9  ? " selected" : ""%>><%=getLocString("services.numberingPlan.private")%></option>
		<option value="10"<%=bean.getNumberingPlanInt() == 10 ? " selected" : ""%>><%=getLocString("services.numberingPlan.ermes")%></option>
		<option value="14"<%=bean.getNumberingPlanInt() == 14 ? " selected" : ""%>><%=getLocString("services.numberingPlan.internet")%></option>
		<option value="18"<%=bean.getNumberingPlanInt() == 18 ? " selected" : ""%>><%=getLocString("services.numberingPlan.wap")%></option>
		</select></div>
	</td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.interfaceVersion")%></th>
	<td><input class=txt type="text" name="interfaceVersion" value="<%=bean.getInterfaceVersion()==null||bean.getInterfaceVersion().length()==0?"3.4":bean.getInterfaceVersion()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.rangeOfAddress")%></th>
	<td><input class=txt maxlength="41" type="text" name="rangeOfAddress" value="<%=StringEncoderDecoder.encode(bean.getRangeOfAddress())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="wantAlias_check_id"><%=getLocString("services.wantAlias")%></label></th>
	<td><input class=check type="checkbox" name="wantAlias" id="wantAlias_check_id" <%=bean.isWantAlias() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="forceDC_check_id"><%=getLocString("services.forceDataCoding")%></label></th>
	<td><input class=check type="checkbox" name="forceDC" id="forceDC_check_id" <%=bean.isForceDC() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.timeout")%></th>
	<td><input class=txt type="text" name="timeout" value="<%=bean.getTimeout()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.password")%></th>
	<td><input class=txt maxlength="8" type="text" name="password" value="<%=StringEncoderDecoder.encode(bean.getPassword())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.receiptSchemeName")%></th>
	<td><input class=txt maxlength="32" type="text" name="receiptSchemeName" value="<%=StringEncoderDecoder.encode(bean.getReceiptSchemeName() == null ? "default" : bean.getReceiptSchemeName())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="disabled_check_id"><%=getLocString("services.disabled")%></label></th>
	<td><input class=check type="checkbox" name="disabled" id="disabled_check_id" <%=bean.isDisabled() ? "checked" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("services.mode")%></th>
	<td><div class=select>
		<select name="mode">
		<option value="<%=SME.MODE_TX%>"  <%=bean.getModeByte() == SME.MODE_TX  ? " selected" : ""%>>TX</option>
		<option value="<%=SME.MODE_RX%>"  <%=bean.getModeByte() == SME.MODE_RX  ? " selected" : ""%>>RX</option>
		<option value="<%=SME.MODE_TRX%>" <%=bean.getModeByte() == SME.MODE_TRX ? " selected" : ""%>>TRX</option>
		</select></div></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="disabled_check_id"><%=getLocString("services.proclimit")%></label></th>
	<td><input class=txt maxlength="8" type="text" name="proclimit" value="<%=StringEncoderDecoder.encode(bean.getProclimit())%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th><label for="disabled_check_id"><%=getLocString("services.schedlimit")%></label></th>
	<td><input class=txt maxlength="8" type="text" name="schedlimit" value="<%=StringEncoderDecoder.encode(bean.getSchedlimit())%>"></td>
</tr>
