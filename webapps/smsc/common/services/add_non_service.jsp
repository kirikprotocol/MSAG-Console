<%@ include file="/common/header.jsp"%>
<script>
function checkSystemType(field)
{
	if (field.value.length < 1)
	{
		alert("Please, enter system type");
		field.focus();
		return false;
	}
	return true;
}
</script>
<form onsubmit="return checkSystemType(system_type)" action="<%=urlPrefix+servicesPrefix%>/add_non_service_2.jsp" method="post">
  <h2>Add service:</h2>
  <h3>service parameters:</h3>
  <table class="list" cellspacing="0">
		<tr class="list">
			<th class="list">System Id</th>
			<td class="list"><input type="text" name="system id" style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">System Type</th>
			<td class="list"><input type="text" name="system_type" maxlength="13" style="width: 100%;" onblur="checkSystemType(this)"></td>
		</tr>
		<tr class="list">
			<th class="list">Type of number</th>
			<td class="list">
				<select name="type of number" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">International</option>
					<option value="2">National</option>
					<option value="3">Network Specific</option>
					<option value="4">Subscriber Number</option>
					<option value="5">Alphanumeric</option>
					<option value="6">Abbreviated</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Numbering plan</th>
			<td class="list">
				<select name="numbering plan" style="width: 100%;">
					<option value="0" selected>Unknown</option>
					<option value="1">ISDN (E163/E164)</option>
					<option value="3">Data (X.121)</option>
					<option value="4">Telex (F.69)</option>
					<option value="6">Land Mobile (E.212)</option>
					<option value="8">National</option>
					<option value="9">Private</option>
					<option value="10">ERMES</option>
					<option value="14">Internet (IP)</option>
					<option value="18">WAP Client Id (to be defined by WAP Forum)</option>
				</select>
			</td>
		</tr>
		<tr class="list">
			<th class="list">Interface version</th>
			<td class="list"><input type="text" name="interface version" value="3.4" readonly style="width: 100%;"></td>
		</tr>
		<tr class="list">
			<th class="list">Range of Address</th>
			<td class="list"><input maxlength="41" type="text" name="range of address" value="" style="width: 100%;"></td>
		</tr>
	</table>
	<div align="right">
		<INPUT type=submit value="Next page">
	</div>
</form>
<%@ include file="/common/footer.jsp"%>