function collasping_tree_showhide_section(sectionId)
{
    var headerElem = document.getElementById("sectionHeader_" + sectionId);
    var valueElem = document.getElementById("sectionValue_" + sectionId);
    if (valueElem.style.display != "none")
    {
        headerElem.className = "collapsing_tree_closed";
        valueElem.style.display = "none";
    }
    else
    {
        headerElem.className = "collapsing_tree_opened";
        valueElem.style.display = "";
    }

    return false;
}