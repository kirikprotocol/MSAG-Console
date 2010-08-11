function collasping_group_showhide_section(sectionId)
{
    var headerElem = document.getElementById("sectionHeader_" + sectionId);
    var valueElem = document.getElementById("sectionValue_" + sectionId);
    var openedElement = document.getElementById("sectionOpened_" + sectionId);  
    if (valueElem.style.display != "none")
    {
        headerElem.className = "collapsing_group_closed";
        valueElem.style.display = "none";
        openedElement.value="false";
    }
    else
    {
        headerElem.className = "collapsing_group_opened";
        valueElem.style.display = "";
        openedElement.value="true";
    }

    return false;
}