<%@ page import="ru.novosoft.smsc.util.*,
                 java.util.*"%>
<%int rowN = 0;%>
<table class=secRep cellspacing=0 cellpadding=2 width="100%">
<col width="15%" align=right>
<col width="80%">
<col width="5%">
<tr><td colspan=3 class=secInner><div class=secView>Route information</div></td></tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>name:</th>
	<td colspan=2><input class=txtW name=routeId value="<%=bean.getRouteId()%>"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><label title="integer from 0 to 32767">priority:</label></th>
	<td colspan=2><input name=priority value="<%=bean.getPriority()%>" maxlength=5 class=txtW validation="priority" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><br></th>
	<td colspan=2><input id=permissible class=check type=checkbox name=permissible <%=bean.isPermissible() ? "checked" : ""%>>&nbsp;<label for=permissible>allowed</label></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><br></th>
	<td colspan=2><input class=check type=checkbox id=billing name=billing <%=bean.isBilling() ? "checked" : ""%>>&nbsp;<label for=billing>billing</label></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><br></th>
	<td colspan=2><input id=archiving class=check type=checkbox name=archiving <%=bean.isArchiving() ? "checked" : ""%>>&nbsp;<label for=archiving>archiving</label></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>service ID:</th>
	<td colspan=2><input name=serviceId value="<%=bean.getServiceId()%>" maxlength=5 class=txtW validation="route_serviceId" onkeyup="resetValidation(this)"></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ sources ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secList>Sources</div></td></tr>
<%rowN=0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
String hexName = StringEncoderDecoder.encodeHEX(name);
%>
<tr class=row<%=(rowN++)&1%>>
<td>&nbsp;</td>
<td colspan=2><input class=check id="subj_src_<%=hexName%>" type=checkbox name=checkedSources value="<%=encName%>" <%=bean.isSrcChecked(name) ? "checked" : ""%>>&nbsp;<label for="subj_src_<%=hexName%>"><%=encName%></label></td>
</tr>
<%}
for (int i=0; i<bean.getSrcMasks().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td>&nbsp;</td>
	<td colspan=2><input class=txtW name=srcMasks value="<%=bean.getSrcMasks()[i]%>" validation="routeMask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to sources"></td>
	<td colspan=2><input class=txtW name=srcMasks validation="routeMask" onkeyup="resetValidation(this)"></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Destinations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=secList>Destinations</div></td></tr>
<%rowN=0;
for (Iterator i = bean.getAllSubjects().iterator(); i.hasNext();)
{
String name = (String) i.next();
String encName = StringEncoderDecoder.encode(name);
String hexName = StringEncoderDecoder.encodeHEX(name);
%>
<tr class=row<%=(rowN++)&1%>>
	<td>&nbsp;</td>
	<td>
		<input onclick="document.all.dst_sme_<%=hexName%>.disabled=!this.checked" class=check id="subj_dst_<%=hexName%>" type=checkbox name=checkedDestinations value="<%=encName%>" <%=bean.isDstChecked(name) ? "checked" : ""%>>&nbsp;<label for="subj_dst_<%=hexName%>"><%=encName%></label></td>
<td><select name=dst_sme_<%=hexName%> <%=!bean.isDstChecked(name) ? "disabled" : ""%>>
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=bean.isSmeSelected(name, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
</tr>
<%}
for (int i=0; i<bean.getDstMasks().length; i++)
{
String dstMask = bean.getDstMasks()[i];
String encMask = StringEncoderDecoder.encode(dstMask);
String hexMask = StringEncoderDecoder.encodeHEX(dstMask);
%>
<tr class=row<%=(rowN++)&1%>>
<td>&nbsp;</td>
	<td><input class=txtW name=dstMasks value="<%=encMask%>" validation="routeMask"></td>
	<td><select name=dst_mask_sme_<%=hexMask%> onkeyup="resetValidation(this)">
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=bean.isMaskSmeSelected(dstMask, smeId) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new mask to destinations"></td>
	<td><input class=txtW name=dstMasks validation="routeMask" onkeyup="resetValidation(this)"></td>
	<td><select name=dst_mask_sme_>
		<%for (Iterator j = bean.getAllSmes().iterator(); j.hasNext(); )
		{
			String smeId = (String) j.next();
			String encSmeId = StringEncoderDecoder.encode(smeId);
			%><option value="<%=encSmeId%>" <%=smeId.equals(bean.getDst_mask_sme_()) ? "selected" : ""%>><%=encSmeId%></option><%
		}
		%>
		</select>
	</td>
</tr>
</table>