<%@
 include file="/WEB-INF/inc/code_header.jsp"
%><%@
 page import="java.util.LinkedList,
					  ru.novosoft.smsc.jsp.smsc.localeResources.LocaleResourcesEdit,
					  ru.novosoft.smsc.jsp.SMSCJspException,
					  ru.novosoft.smsc.jsp.SMSCErrors,
					  ru.novosoft.smsc.jsp.smsc.localeResources.Section,
					  ru.novosoft.smsc.util.StringEncoderDecoder,
					  java.util.Iterator,
				  java.io.PrintWriter,
				  java.io.IOException"
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
%><%@
include file="/WEB-INF/inc/collapsing_tree.jsp"%><%!
	void printAddParam(JspWriter out, Section section) throws IOException
	{
		String sectionFullName = section.getFullName();
		out.print("<tr class=row" + ((row++) & 1) + ">");
      out.print("<th><input class=txt id=\"newParamName_" + sectionFullName + "\" name=\"newParamName_" + sectionFullName + "\"></th>");
		out.print("<td width=100% ><input class=txtW id=\"newParamValue_" + sectionFullName + "\" name=\"newParamValue_" + sectionFullName + "\"></td>");
		out.print("<td><img src=\"" + CPATH + "/img/but_add.gif\" onclick=\"addParam('" + sectionFullName + "')\" title='Add new parameter'></td>");
		out.print("</tr>");
	}
	void printAddSection(JspWriter out, Section section) throws IOException
	{
		String sectionFullName = section.getFullName();
		out.print("<div>");
		out.print("<input class=txt id=\"newSectionInput_" + sectionFullName + "\" name=\"newSectionInput_" + sectionFullName + "\">");
		out.print("<img src=\"" + CPATH + "/img/but_add.gif\" onclick=\"addSection('" + sectionFullName + "')\" title='Add new section' style='position:relative;top:4px'></div>");
	}
	void printResourceSection(JspWriter out, Section section) throws IOException
	{
		final String sectionFullName = section.getFullName();
		startSection(out, sectionFullName, section.getName(), section.getParent() == null,
						 "<img src=\"" + CPATH + "/img/ico16_cross.gif\" onclick=\"removeSection('" + sectionFullName + "')\" title='Remove this section'>");
		startParams(out, "paramTable_" + sectionFullName);
		for (Iterator i = section.getParamNames().iterator(); i.hasNext();) {
			String paramName = (String) i.next();
			param(out,
					paramName,
					sectionFullName + '.' + paramName,
					section.getParam(paramName),
					"paramRow_" + sectionFullName + Section.NAME_DELIMETER + paramName,
					"<img src=\"" + CPATH + "/img/ico16_cross.gif\" onclick=\"removeParam('" + sectionFullName + "', '" + paramName +"')\" title='Remove this parameter'>");
		}
		printAddParam(out, section);
		finishParams(out);
		for (Iterator i = section.getSectionNames().iterator(); i.hasNext();) {
			printResourceSection(out, section.getSection((String) i.next()));
		}
		printAddSection(out, section);
		finishSection(out);
	}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<script language="JavaScript">
function createImgButton(imgUrl, onclickT, tooltipText)
{
	return "<img src=\""+imgUrl+"\" onclick=\"" + onclickT + "\" title=\"" + tooltipText + "\">";
}
function addParam(sectionName)
{
	tableElem = opForm.all("paramTable_" + sectionName);
	paramNameElem = opForm.all("newParamName_" + sectionName);
	paramValueElem = opForm.all("newParamValue_" + sectionName);

	newRow = tableElem.insertRow(tableElem.rows.length-1);
	newRow.className = "row" + (tableElem.rows.length & 1);
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

	imgElement = document.createElement("img");
	imgElement.src = "<%=CPATH%>/img/ico16_cross.gif";
	imgElement.setAttribute('sectionName', sectionName);
	imgElement.setAttribute('paramName', paramNameElem.value);
	imgElement.attachEvent("onclick", removeParam_Event);
	newCell = newRow.insertCell();
	newCell.appendChild(imgElement);

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
	sectionElem = opForm.all("sectionHeader_" + sectionName);
	sectionElem.removeNode(true);
	sectionElem = opForm.all("sectionValue_" + sectionName);
	sectionElem.removeNode(true);
}
function sectionHeader(sectionName, fullName)
{
	var sectionHeader_ = "sectionHeader_" + fullName;
	return ""
		+ "<div class=collapsing_tree_opened id=\"" + sectionHeader_ + "\" onclick=\"collasping_tree_showhide_section('" + fullName + "')\">"
		+ "<table cellspacing=0><tr><td width=100%>"
		+ sectionName
		+ "</td>"
		+ "<td>"
		+ createImgButton("<%=CPATH%>/img/ico16_cross.gif", "removeSection('" + fullName + "')", "Remove this section")
		+ "</td>"
		+ "</tr></table></div>";
}
function createInput(id_, className)
{
	return "<input class=" + className + " name=\"" +id_ + "\" id=\"" + id_ + "\">";
}
function addSectionField(sectionFN)
{
	var newSectionInput = "newSectionInput_" + sectionFN;
	return ""
		+ "<div>"
		+ createInput(newSectionInput, "txt")
		+ createImgButton("<%=CPATH%>/img/but_add.gif", "addSection('" + sectionFN + "')", "Add new section")
		+ "</div>";
}
function addParamField(sectionFN)
{
	var newParamName = "newParamName_" + sectionFN;
	var newParamValue = "newParamValue_" + sectionFN;
	var paramTableID = "paramTable_" + sectionFN;

	return ""
		+ "<table class=properties_list cellspacing=0 id=\"" + paramTableID + "\">"
		+ "<col width=150px>"
		+ "<tr class=row0>"
		+   "<td>"
		+     createInput(newParamName, "txt")
		+   "</td><td>"
		+     createInput(newParamValue, "txtW")
		+   "</td><td>"
		+     createImgButton("<%=CPATH%>/img/but_add.gif", "addParam('" + sectionFN + "')", "Add new param")
		+   "</td>"
		+ "</tr>"
		+ "</table>";

}
function sectionValue(sectionName, fullName)
{
	var sectionValue_ = "sectionValue_" + fullName;
	return ""
		+ "<table cellspacing=0 id=\"" + sectionValue_ + "\">"
		+ "<col width='56px'/>"
		+ "<tr><th/><td>"
		+ addParamField(fullName)
		+ addSectionField(fullName)
		+ "</td></tr>"
		+ "</table>";
}
function addSection(parentSectionName)
{
	newSectionNameElem = opForm.all("newSectionInput_" + parentSectionName);
	newSectionName = newSectionNameElem.value;
	fullName = parentSectionName + "<%=Section.NAME_DELIMETER%>" + newSectionName;

	parentSectionTable = opForm.all("sectionValue_" + parentSectionName);

	parentNewRow = parentSectionTable.insertRow(parentSectionTable.rows.length);
	parentNewRow.insertCell();
	nestCell = parentNewRow.insertCell();
	nestCell.innerHTML = ""
		+ sectionHeader(newSectionName, fullName)
		+ sectionValue(newSectionName, fullName);

	newSectionNameElem.value = "";
}
</script>
<%
	page_menu_begin(out);
	page_menu_button(out, "mbSave", "Save", "Save changes and return to locales list");
   page_menu_button(out, "mbCancel", "Cancel", "Cancel changes and return to locales list");
	page_menu_space(out);
	page_menu_end(out);
%>
<div class=content>
<input type=hidden name=locale value="<%=bean.getLocale()%>">
<input type=hidden name=initialized value=true>
<h2>Locale "<%=bean.getLocale()%>"</h2>
<%printResourceSection(out, bean.getSettings());%>
<%printResourceSection(out, bean.getResources());%>
</div>
<%
	page_menu_begin(out);
	page_menu_button(out, "mbSave", "Save", "Save changes and return to locales list");
   page_menu_button(out, "mbCancel", "Cancel", "Cancel changes and return to locales list");
	page_menu_space(out);
	page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>