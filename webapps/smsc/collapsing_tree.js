function collasping_tree_showhide_section(sectionId)
{
	var headerElem = opForm.all(sectionId + "_header");
	var valueElem = opForm.all(sectionId + "_value");
	if (valueElem.style.display != "none")
	{
		headerElem.className = "collapsing_tree_closed";
		valueElem.style.display="none";
	}
	else
	{
		headerElem.className = "collapsing_tree_opened";
		valueElem.style.display = "";
	}
	
	return false;
}