<%@
 include file="/WEB-INF/inc/code_header.jsp"
%><%@
 page import="java.util.LinkedList,
					  ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesEdit,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.jsp.smsc.localeResources.Section,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Iterator"
%><jsp:useBean
 id="bean" class="ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesEdit" scope="page"
/><jsp:setProperty
name="bean" property="*"
/><%
	MENU0_SELECTION = "MENU0_LOCALE_RESOURCES";
	FORM_METHOD = "POST";
	TITLE = "Resource edit";
	switch (bean.process(appContext, errorMessages, loginedUserPrincipal, request.getParameterMap()))
	{
		case LocaleResourcesEdit.RESULT_DONE:
			response.sendRedirect("index.jsp");
			return;
		case LocaleResourcesEdit.RESULT_OK:
			STATUS.append("Ok");
			break;
		case LocaleResourcesEdit.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
	}
%><%!
	String createButton(String buttonText, String onclick)
	{
		return "<input class=btn type=button"
				+ " value=\"" + buttonText + "\""
				+ " onclick=\""+onclick+ "\""
				+ ">";
	}
	String createInput(String id)
	{
		return "<input class=txtW name=\"" +id + "\" id=\"" + id + "\">";
	}
	String createInput(String id, String value)
	{
		return "<input class=txtW name=\"" + id + "\" id=\"" + id + "\" value=\"" + value + "\">";
	}
%><%!
	String printSection(Section section)
	{
		final String sectionFN = StringEncoderDecoder.encode(section.getFullName());
		final String sectionN = StringEncoderDecoder.encode(section.getName());

		final String sectionId = "section_" + sectionFN;
		final String addSectionId = "addSection_" + sectionFN;
		final String newParamName = "newParamName_" + sectionFN;
		final String newParamValue = "newParamValue_" + sectionFN;
		final String newSectionInput = "newSectionInput_" + sectionFN;


		String result = "";
		result += "<dl id=\"" + sectionId + "\">";

		result += "<dt class=" + (section.getParent() == null ? "secTitleOpened" : "secTitleClosed") + " onclick='showhide(this)'>";
		result += sectionN;
		if (section.getParent() != null)
			result += "&nbsp;" + createButton("remove this section", "removeSection('" + sectionFN + "')");
		result += "</dt>";

		result += "<dd id=\""+addSectionId + "\" " + (section.getParent() == null ? "" : "style=\"display:none\"") + ">";
		// sections
		result += "<table class=secRep width=100% cellspacing=1><col width=99% >";
		result += "<tr><td>";
		result += createInput(newSectionInput);
		result += "</td><td>";
		result += createButton("Add new section", "addSection('" + sectionFN + "')");
		result += "</td></tr></table>";
		for (Iterator i = section.getSectionNames().iterator(); i.hasNext();)
		{
			result += printSection(section.getSection((String) i.next()));
		}

		//params
		final String paramTableID = "paramTable_" + sectionFN;
		result += "<table id=\"" + paramTableID + "\" class=secRep width=100% cellspacing=1>";
		result += "<col width=180px>";
		result += "<col>";
		result += "<col width=1% >";
		if (section.getParamNames().size() > 0)
		{
			int row = 0;
			for (Iterator i = section.getParamNames().iterator(); i.hasNext();)
			{
				final String paramName = (String) i.next();
				final String paramFN = StringEncoderDecoder.encode(section.getFullName() + Section.NAME_DELIMETER + paramName);
				final String paramN = StringEncoderDecoder.encode(paramName);
				final String paramRow = "paramRow_"+paramFN;
				result += "<tr class=row" + ((row++) & 1) + " id=\"" + paramRow + "\">";
				result +=   "<th class=label>" + paramN + "</th>";
				result +=   "<td>" + createInput(paramFN, StringEncoderDecoder.encode(section.getParam(paramName)))+ "</td>";
				result +=   "<td>" + createButton("Remove param", "removeParam('" + sectionFN + "', '" + paramN + "')") + "</td>";
				result += "</tr>";
			}
			result += "<tr class=row" + ((row++) & 1) + ">";
			result +=   "<td>" + createInput(newParamName) + "</td>";
			result +=   "<td>" + createInput(newParamValue) + "</td>";
			result +=   "<td>" + createButton("Add new param",  "addParam('" + sectionFN + "')") + "</td>";
			result += "</tr>";
		}
		result += "</table>";
		result += "</dd>";

		result += "</dl>";
		return result;
	}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<script language="JavaScript">
function addParam(sectionName)
{
	tableElem = opForm.all("paramTable_" + sectionName);
	paramNameElem = opForm.all("newParamName_" + sectionName);
	paramValueElem = opForm.all("newParamValue_" + sectionName);

	rowClassName = "row" + (tableElem.rows.length & 1);
	newRow = tableElem.insertRow(tableElem.rows.length-1);
	newRow.className = rowClassName;
	newRow.id = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
	newCell = document.createElement("th");
	newCell.className = "label";
	newCell.innerText = paramNameElem.value;
	newRow.appendChild(newCell);

	inputElement = document.createElement("input");
	inputElement.name = sectionName + "<%=Section.NAME_DELIMETER%>" + paramNameElem.value;
	inputElement.value = paramValueElem.value;
	inputElement.className = "txtW";
	newCell = newRow.insertCell();
	newCell.appendChild(inputElement);

	inputElement = document.createElement("input");
	inputElement.className = "btn";
	inputElement.value = "Remove param";
	inputElement.setAttribute('sectionName', sectionName);
	inputElement.setAttribute('paramName', paramNameElem.value);
	inputElement.attachEvent("onclick", removeParam_Event);
	newCell = newRow.insertCell();
	newCell.appendChild(inputElement);

	paramNameElem.value = "";
	paramValueElem.value = "";
}
function removeParam_Event()
{
	removeParam(event.srcElement.attributes.sectionName.nodeValue, event.srcElement.attributes.paramName.nodeValue);
}
function removeParam(sectionName, paramName)
{
	tableElem = opForm.all("paramTable_" + sectionName);
	rowId = "paramRow_" + sectionName + "<%=Section.NAME_DELIMETER%>" + paramName;
	rowElem = tableElem.rows(rowId);
	tableElem.deleteRow(rowElem.rowIndex);
}
function removeSection(sectionName)
{
	sectionElem = opForm.all("section_" + sectionName);
	sectionElem.removeNode(true);
}
function showhide_event()
{
	showhide(event.srcElement)
}
function showhide(elem)
{
	var st = elem.nextSibling.style;
	if (st.display != "none")
	{
		elem.className = "secTitleClosed";
		//elem.style.borderBottom = "1px solid #888888";
		st.display="none";
	}
	else
	{
		elem.className = "secTitleOpened";
		st.display = "";
	}
}
function addSection(parentSectionName)
{
	parentSectionNameElem = opForm.all("newSectionInput_" + parentSectionName);
	newSectionName = parentSectionNameElem.value;
	fullName = parentSectionName + "<%=Section.NAME_DELIMETER%>" + newSectionName;
	paramTableID = "paramTable_" + fullName;

	parentSectionElem = opForm.all("addSection_" + parentSectionName);
   sectionElem = document.createElement("dl");
	sectionElem.id = "section_" + fullName;

	dtElem = document.createElement("dt");
	dtElem.className = "secTitleOpened";
	dtElem.innerText = newSectionName;
	dtElem.innerHTML = dtElem.innerHTML + "&nbsp;<input type=button class=btn value='remove this section' onclick=\"removeSection('" + fullName + "')\">";
	dtElem.attachEvent("onclick", showhide_event);
	sectionElem.appendChild(dtElem);

	paramName = "newParamName_" + fullName;
	paramValue = "newParamValue_" + fullName;
	newSectionInput = "newSectionInput_" + fullName;
	ddElem = document.createElement("dd");
	ddElem.id = "addSection_" + fullName;
	ddElem.innerHTML = ""

		+ "<table class=secRep width=100% cellspacing=1><col width=99% >"
		+ "<tr><td>"
		+ "<input class=txtW name=\"" + newSectionInput + "\" id=\"" + newSectionInput + "\">"
		+ "</td><td>"
		+ "<input class=btn type=button value=\"Add new section\" onclick=\"addSection('" + fullName + "')\">"
		+ "</td></tr></table>"
	   + "<table id=\"" + paramTableID + "\" class=secRep width=100% cellspacing=1>"
		+   "<tr class=row0>"
		+     "<td><input class=txtW name=\"" + paramName + "\" id=\"" + paramName + "\"></td>"
		+     "<td><input class=txtW name=\"" + paramValue + "\" id=\"" + paramValue + "\"></td>"
		+     "<td><input class=btn type=button value=\"Add new param\" onclick=\"addParam('" + fullName + "')\"></td>"
		+   "</tr>"
		+ "</table>";
	sectionElem.appendChild(ddElem);

	parentSectionElem.appendChild(sectionElem);

	parentSectionNameElem.value = "";
}
</script>
<input type=hidden name=locale value="<%=bean.getLocale()%>">
<input type=hidden name=initialized value=true>
<h2>Locale "<%=bean.getLocale()%>"</h2>
<%=printSection(bean.getSettings())%>
<%=printSection(bean.getResources())%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save changes and return to locales list">
<input class=btn type=submit name=mbCancel value="Cancel" title="Cancel changes and return to locales list">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
