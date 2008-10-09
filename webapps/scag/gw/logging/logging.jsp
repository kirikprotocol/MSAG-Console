<%@ include file="/WEB-INF/inc/header.jspf" %>
<%@taglib prefix="sm-l" tagdir="/WEB-INF/tags/logging"%>

<sm:page title="logging.title">
<jsp:attribute name="menu">
</jsp:attribute>
<jsp:body>

<script type="text/javascript">
    function collasping_tree_showhide_section(sectionId) {

        var headerElem = document.getElementById("sectionHeader_" + sectionId);
//        var valueElem = opForm.all("sectionValue_" + sectionId);
        var valueElem = document.getElementById("sectionValue_" + sectionId);
        if (valueElem.style.display != "none") {
            headerElem.className = "collapsing_tree_closed_logging";
            valueElem.style.display = "none";
        } else {
            headerElem.className = "collapsing_tree_opened_logging";
            valueElem.style.display = "";
        }
        return false;
    }
</script>

<c:set var="tree" value="${bean.rootCategory}"/>
<table cellspacing=0 cellpadding=0 id="sectionValue_">
    <col width='56px'/>
    <tr>
        <td>
            <c:if test="${tree.root}">
                <sm-l:select name="${tree.name}" fullName="${tree.name}" priority="${tree.priority}" />
            </c:if>
            <c:if test="${tree.hasChilds}">
                <c:set var="ch" value="${tree.childs}"/>
                <c:forEach items="${ch}" var="catItem">
                <c:if test="${!catItem.value.hasChilds}">
                    <sm-l:select name="${catItem.value.name}" fullName="${catItem.value.fullName}" priority="${catItem.value.priority}"/>
                </c:if>
                </c:forEach>
                <c:forEach items="${ch}" var="catItem">
                    <c:if test="${catItem.value.hasChilds}">
                        <table cellspacing=0 border="0">
                            <tr>
                                <td width="10%">
                                    <div class=collapsing_tree_closed_logging id="sectionHeader_${catItem.value.fullName}"
                                         onclick="collasping_tree_showhide_section('${catItem.value.fullName}')">
                                         ${catItem.value.name}
                                    </div>
                                </td>
                                <td>
                                    <sm-l:select name="_empty_name_" fullName="${catItem.value.fullName}" priority="${catItem.value.priority}"/>
                                </td>
                            </tr>
                        </table>
                    <sm-l:section fullName="${catItem.value.fullName}"/>
                    </c:if>
                </c:forEach>
            </c:if>
        </td>
    </tr>
</table>
<br>
<c:choose>
    <c:when test="${bean.running}">
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="logging.item.mbApply.value" enabled="true" title="logging.item.mbApply.title" onclick=""/>
            <td>&nbsp</td>
            <td>
            <c:set var="page_menu_delimiter_needed" value="false" scope="request"/>
            <sm-pm:item name="mbSavePermanent" value="logging.item.mbWrite.value" enabled="true" title="logging.item.mbWrite.title" onclick=""/>
            <sm-pm:item name="mbRead" value="logging.item.mbRead.value" enabled="true" title="logging.item.mbRead.title" onclick=""/>
            <sm-pm:space/>
            </td>
<%--            <sm-pm:item name="mbSave" value="logging.item.mbApply.value" enabled="true" title="item.mbapply.title"/>--%>
<%--            <sm-pm:item name="mbSavePermanent" value="logging.item.mbWrite.value" enabled="true"--%>
<%--                        title="logging.item.save.permanent.title"--%>
<%--                        onclick=""/>--%>
<%--            <sm-pm:item name="mbSave" value="logging.item.mbRead.value" enabled="true" title="item.mbapply.title"/>--%>
<%--            <sm-pm:space/>--%>
        </sm-pm:menu>
    </c:when>
    <c:otherwise>
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="logging.item.mbApply.value" enabled="false" title="logging.item.mbApply.title" onclick=""/>
            <td>&nbsp</td>
            <td>
            <c:set var="page_menu_delimiter_needed" value="false" scope="request"/>
            <sm-pm:item name="mbSavePermanent" value="logging.item.mbWrite.value" enabled="true" title="logging.item.mbWrite.title" onclick=""/>
            <sm-pm:item name="mbRead" value="logging.item.mbRead.value" enabled="true" title="logging.item.mbRead.title" onclick=""/>
            <sm-pm:space/>
            </td>
        </sm-pm:menu>
    </c:otherwise>
</c:choose>
</jsp:body>
</sm:page>