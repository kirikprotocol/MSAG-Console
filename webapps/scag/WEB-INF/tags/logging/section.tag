<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@ taglib tagdir="/WEB-INF/tags/logging" prefix="sm-l"%>
<%@attribute name="fullName" required="true"%>
<c:set var="catItem" value="${bean.fullNameToCatInfo[fullName]}"/>
<table cellspacing="0" cellpadding="0" id="sectionValue_${catItem.fullName}" style="display:none">
    <col width='56px'/>
    <tr>
        <th/>
        <td>
            <c:forEach items="${catItem.children}" var="catItem">
            <c:choose>
                <c:when test="${catItem.value.hasChildren}">
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
                </c:when>
                <c:otherwise>
                    <sm-l:select name="${catItem.value.name}" fullName="${catItem.value.fullName}" priority="${catItem.value.priority}"/>
                </c:otherwise>
            </c:choose>
            </c:forEach>
        </td>
    </tr>
</table>

