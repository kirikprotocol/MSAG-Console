<div class=content>
<div class=page_subtitle>Subject information</div>
<%int rowN = 0;%>
<%
//if (beanResult == bean.RESULT_OK)
{
%>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%">
<col width="85%">
<tr class=row<%=(rowN++)&1%>>
	<th>name:</th>
	<td><input class=txt name=name value="<%=bean.getName()%>" <%=isNew?"":"readonly"%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>default SME:</th>
	<td><select name=defSme>
		<%for (Iterator i = bean.getPossibleSmes().iterator(); i.hasNext(); )
		{
			String smeId = (String) i.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getDefSme()) ? "selected" : ""%>><%=encSmeId%></option><%
		}%>
	</select></td>
</tr>
</table>
<div class=page_subtitle>Masks</div>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%">
<col width="85%">
<%
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td colspan=2><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new mask to subject");%></td>
</tr>
</table>
<%}%>
</div>