	<%@ include file="/sketches/header.jsp" %>
	<%@ include file="menu.jsp" %><h4>Select destination SMEs</h4>
	<form action="add_route.jsp" method="post">
		<table class="list" cellspacing="0">
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>subj</b></font> Subject 2
				</th>
				<td class="list">
					<select name="Group 2" style="width=100%;">
						<option value="id_1_sme">SME 1</option>
						<option value="id_2_sme" selected>SME 2</option>
						<option value="id_3_sme">SME 3</option>
						<option value="id_4_sme">SME 4</option>
					</select>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>subj</b></font> Subject 1
				</th>
				<td class="list">
					<select name="Subscr 1" style="width=100%;">
						<option value="id_1_sme">SME 1</option>
						<option value="id_2_sme">SME 2</option>
						<option value="id_3_sme">SME 3</option>
						<option value="id_4_sme" selected>SME 4</option>
					</select>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>subj</b></font> Subject 4
				</th>
				<td class="list">
					<select name="Subscr 4" style="width=100%;">
						<option value="id_1_sme" selected>SME 1</option>
						<option value="id_2_sme">SME 2</option>
						<option value="id_3_sme">SME 3</option>
						<option value="id_4_sme">SME 4</option>
					</select>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>mask</b></font> +7901
				</th>
				<td class="list">
					<select name="+7901" style="width=100%;">
						<option value="id_1_sme">SME 1</option>
						<option value="id_2_sme">SME 2</option>
						<option value="id_3_sme" selected>SME 3</option>
						<option value="id_4_sme">SME 4</option>
					</select>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>mask</b></font> +7801
				</th>
				<td class="list">
					<select name="+7801" style="width=100%;">
						<option value="id_1_sme">SME 1</option>
						<option value="id_2_sme" selected>SME 2</option>
						<option value="id_3_sme">SME 3</option>
						<option value="id_4_sme">SME 4</option>
					</select>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%"><font size="-2"><b>mask</b></font> 911
				</th>
				<td class="list">
					<select name="911" style="width=100%;">
						<option value="id_1_sme">SME 1</option>
						<option value="id_2_sme">SME 2</option>
						<option value="id_3_sme">SME 3</option>
						<option value="id_4_sme" selected>SME 4</option>
					</select>
				</td>
			</tr>
		</table>
		<input type="Submit">
	</form>
	<%@ include file="/sketches/footer.jsp" %>