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
	this.CPATH = CPATH;
%><%!
	void createImgButton(JspWriter out, String imgUrl, String onclick, String tooltip) throws IOException
	{
		out.print("<a href=\"#\" onclick=\"" + onclick + "\" title=\"" + tooltip + "\"><img src=\""+imgUrl+"\"></a>");
	}
	void createInput(JspWriter out, String id) throws IOException
	{
		out.print("<input class=txt name=\"" +id + "\" id=\"" + id + "\">");
	}
	void createInput(JspWriter out, String id, String value) throws IOException
	{
		out.print("<input class=txt name=\"" + id + "\" id=\"" + id + "\" value=\"" + value + "\">");
	}
	String CPATH = "";
	void printAddSectionField(JspWriter out, String sectionFN) throws IOException
	{
		final String newSectionInput = "newSectionInput_" + sectionFN;
		out.print("<table width=100% cellspacing=1 border=0><col/><col width='16px'/>");
		out.print("<tr><td>");
		createInput(out, newSectionInput);
		out.print("</td><td>");
		createImgButton(out, CPATH + "/img/ico16_checked_g.gif", "addSection('" + sectionFN + "')", "Add new section");
		out.print("</td></tr>");
		out.print("</table>");
	}
	void printSubSections(JspWriter out, Section section, String sectionFN) throws IOException
	{
		for (Iterator i = section.getSectionNames().iterator(); i.hasNext();)
		{
			out.print("<tr><th/><td>");
			printSection(out, section.getSection((String) i.next()));
			out.print("</td></tr>");
		}
	}
	void printParams(JspWriter out, Section section, String sectionFN) throws IOException
	{
		final String newParamName = "newParamName_" + sectionFN;
		final String newParamValue = "newParamValue_" + sectionFN;
		final String paramTableID = "paramTable_" + sectionFN;
		//params

		out.print("<table id=\"" + paramTableID + "\" class=list width=100% cellspacing=1>");
		out.print("<col width='180px'/>");
		out.print("<col/>");
		out.print("<col width='16px'/>");
		int row = 0;
		if (section.getParamNames().size() > 0)
		{
			for (Iterator i = section.getParamNames().iterator(); i.hasNext();)
			{
				final String paramName = (String) i.next();
				final String paramFN = StringEncoderDecoder.encode(section.getFullName() + Section.NAME_DELIMETER + paramName);
				final String paramN = StringEncoderDecoder.encode(paramName);
				final String paramRow = "paramRow_" + paramFN;
				out.print("<tr class=row" + ((row++) & 1) + " id=\"" + paramRow + "\">");
				out.print("<th class=label>" + paramN + "</th>");
				out.print("<td>");
				createInput(out, paramFN, StringEncoderDecoder.encode(section.getParam(paramName)));
				out.print("</td><td>");
				createImgButton(out, CPATH + "/img/ico16_cross.gif", "removeParam('" + sectionFN + "', '" + paramN + "')", "Remove this param");
				out.print("</td>");
				out.print("</tr>");
			}
		}
		out.print("<tr class=row" + ((row++) & 1) + ">");
		out.print("<td>");
		createInput(out, newParamName);
		out.print("</td><td>");
		createInput(out, newParamValue);
		out.print("</td><td>");
		createImgButton(out, CPATH + "/img/ico16_checked_g.gif", "addParam('" + sectionFN + "')", "Add new param");
		out.print("</td>");
		out.print("</tr>");
		out.print("</table>");
	}
	void printSectionHeader(JspWriter out, final Section section, final String sectionFN, final String sectionValue) throws IOException
	{
		final String sectionN = StringEncoderDecoder.encode(section.getName());
		final String sectionHeader = "sectionHeader_" + sectionFN;

		out.print("<table border=0 cellspacing=0 cellpadding=0 width=100% id=\"" + sectionHeader + "\">");
		out.print("<col/>");
		out.print("<col width='16px'/>");

		out.print("<tr>");
		out.print("<th class=" + (section.getParent() == null ? "secTitleOpened" : "secTitleClosed") + " onclick=\"showhide(this, opForm.all('" + sectionValue + "'))\">");
		out.print(sectionN);
		out.print("</th>");
		if (section.getParent() != null)
		{
			out.print("<td class=secTitleOpenedClosedContinue>");
			createImgButton(out, CPATH + "/img/ico16_cross.gif", "removeSection('" + sectionFN + "')", "Remove this section");
			out.print("</td>");
		}
		out.print("</tr>");
		out.print("</table>");
	}
	void printSectionValue(JspWriter out, Section section, String sectionFN, String sectionValue) throws IOException
	{
		out.print("<table border=0 cellspacing=0 cellpadding=0 width=100%  id=\"" + sectionValue + "\" " + (section.getParent() == null ? "" : "style=\"display:none\"") + ">");
		out.print("<col width='20px'/>");

		out.print("<tr><th/><td>");
		printAddSectionField(out, sectionFN);
		out.print("</td></tr><tr><th/><td>");
		printParams(out, section, sectionFN);
		out.print("</td></tr>");
		printSubSections(out, section, sectionFN);

		out.print("</table>");
	}
	void printSection(JspWriter out, Section section) throws IOException
	{
		final String sectionFN = StringEncoderDecoder.encode(section.getFullName());
		final String sectionValue = "sectionValue_" + sectionFN;


		printSectionHeader(out, section,  sectionFN,  sectionValue);
      printSectionValue(out, section, sectionFN, sectionValue);
	}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<script language="JavaScript">
function createImgButton(imgUrl, onclickT, tooltipText)
{
	return "<a href=\"#\" onclick=\"" + onclickT + "\" title=\"" + tooltipText + "\"><img src=\""+imgUrl+"\"></a>";
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
	inputElement.className = "txt";
	newCell = newRow.insertCell();
	newCell.appendChild(inputElement);

	buttonElement = document.createElement("a");
	buttonElement.href = "#";
	buttonElement.title = "Remove this param";
	imgElement = document.createElement("img");
	imgElement.src = "<%=CPATH%>/img/ico16_cross.gif";
	imgElement.setAttribute('sectionName', sectionName);
	imgElement.setAttribute('paramName', paramNameElem.value);
	buttonElement.appendChild(imgElement);
	buttonElement.attachEvent("onclick", removeParam_Event);
	newCell = newRow.insertCell();
	newCell.appendChild(buttonElement);

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
function showhide_event()
{
	showhide(event.srcElement)
}
function showhide(helem, velem)
{
	if (velem.style.display != "none")
	{
		helem.className = "secTitleClosed";
		//elem.style.borderBottom = "1px solid #888888";
		velem.style.display="none";
	}
	else
	{
		helem.className = "secTitleOpened";
		velem.style.display = "";
	}
}
function sectionHeader(sectionName, fullName)
{
	var sectionHeader_ = "sectionHeader_" + fullName;
	var sectionValue_ = "sectionValue_" + fullName;
	return ""
		+ "<table border=0 cellspacing=0 cellpadding=0 width=100% id=\"" + sectionHeader_ +  "\">"
		+ "<col/>"
		+ "<col width='16px'/>"
		+ "<tr>"
		+ "<th class=secTitleClosed onclick=\"showhide(this, opForm.all('" + sectionValue_ + "'))\">"
		+ sectionName
		+ "</th>"
		+ "<td class=secTitleOpenedClosedContinue>"
		+ createImgButton("<%=CPATH%>/img/ico16_cross.gif", "removeSection('" + fullName + "')", "Remove this section")
		+ "</td>"
		+ "</tr>"
		+ "</table>";
}
function createInput(id_)
{
	return "<input class=txt name=\"" +id_ + "\" id=\"" + id_ + "\">";
}
function addSectionField(sectionFN)
{
	var newSectionInput = "newSectionInput_" + sectionFN;
	return ""
		+ "<table class=list width=100% cellspacing=1><col/><col width='16px'/>"
		+ "<tr><td>"
		+ createInput(newSectionInput)
		+ "</td><td>"
		+ createImgButton("<%=CPATH%>/img/ico16_checked_g.gif", "addSection('" + sectionFN + "')", "Add new section")
		+ "</td></tr>"
		+ "</table>";
}
function addParamField(sectionFN)
{
	var newParamName = "newParamName_" + sectionFN;
	var newParamValue = "newParamValue_" + sectionFN;
	var paramTableID = "paramTable_" + sectionFN;

	return ""
		+ "<table id=\"" + paramTableID + "\" class=list width=100% cellspacing=1>"
		+ "<col width='180px'/>"
		+ "<col/>"
		+ "<col width='16px'/>"
		+ "<tr class=row0>"
		+   "<td>"
		+     createInput(newParamName)
		+   "</td><td>"
		+     createInput(newParamValue)
		+   "</td><td>"
		+     createImgButton("<%=CPATH%>/img/ico16_checked_g.gif", "addParam('" + sectionFN + "')", "Add new param")
		+   "</td>"
		+ "</tr>"
		+ "</table>";

}
function sectionValue(sectionName, fullName)
{
	var sectionValue_ = "sectionValue_" + fullName;
	return ""
		+ "<table border=0 cellspacing=0 cellpadding=0 width=100%  id=\"" + sectionValue_ + "\" style=\"display:none\">"
		+ "<col width='20px'/>"
		+ "<col/>"

		+ "<tr><th/><td>"
		+ addSectionField(fullName)
		+ "</td></tr><tr><th/><td>"
		+ addParamField(fullName)
		+ "</td></tr>"

		+ "</table>";
}
function addSection(parentSectionName)
{
	parentSectionNameElem = opForm.all("newSectionInput_" + parentSectionName);
	newSectionName = parentSectionNameElem.value;
	fullName = parentSectionName + "<%=Section.NAME_DELIMETER%>" + newSectionName;
	paramTableID = "sectionValue_" + fullName;

	parentSectionTable = opForm.all("sectionValue_" + parentSectionName);

	parentNewRow = parentSectionTable.insertRow(parentSectionTable.rows.length);
	parentNewRow.insertCell();
	nestCell = parentNewRow.insertCell();
	nestCell.innerHTML = ""
		+ sectionHeader(newSectionName, fullName)
		+ sectionValue(newSectionName, fullName);

	parentSectionNameElem.value = "";
}
</script>
<input type=hidden name=locale value="<%=bean.getLocale()%>">
<input type=hidden name=initialized value=true>
<h2>Locale "<%=bean.getLocale()%>"</h2>
<%printSection(out, bean.getSettings());%>
<%printSection(out, bean.getResources());%>
<div class=secButtons>
<input class=btn type=submit name=mbSave value="Save" title="Save changes and return to locales list">
<input class=btn type=submit name=mbCancel value="Cancel" title="Cancel changes and return to locales list">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
