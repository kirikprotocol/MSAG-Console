<%@ include file="/WEB-INF/inc/header.jspf" %>

<sm:page title="logging.title">
<jsp:attribute name="menu">
</jsp:attribute>
<jsp:body>
<script type="text/javascript">
    function collasping_tree_showhide_section(sectionId) {

        var headerElem = opForm.all("sectionHeader_" + sectionId);
        var imgElem = opForm.all("img_"+sectionId);
        var valueElem = opForm.all("sectionValue_" + sectionId);
        if (valueElem.style.display != "none") {
            headerElem.className = "collapsing_tree_closed";
            imgElem.src = "content/images/list_closed.gif";
            valueElem.style.display = "none";
        } else {
            headerElem.className = "collapsing_tree_opened";
            imgElem.src = "content/images/list_opened.gif";
            valueElem.style.display = "";
        }

        return false;
    }
</script>
<c:set var="child" value="${bean.rootCategories}"/>
<c:set var="tree" value="${bean.rootCategory}"/>

    <table cellspacing=0 cellpadding=0 id="sectionValue_">
        <col width='56px'/>
        <tr>
            <td>
                <c:if test="${tree.root}">
                    <table class=properties_list cellspacing=0><col width=150px>
                        <tr class=row0>
                            <th nowrap>${tree.name}</th>
                            <td><select name="category_${tree.name}" id="category_${tree.name}"
                                        style="font-size:80%;">
                                <c:forEach items="${bean.priorities}" var="i">
                                    <c:choose>
                                        <c:when test="${tree.priority==i}">
                                            <option value="${fn:escapeXml(i)}"
                                                    selected="true">${fn:escapeXml(i)}</option>
                                        </c:when>
                                        <c:otherwise>
                                            <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                        </c:otherwise>
                                    </c:choose>
                                </c:forEach>
                            </select>
                            </td>
                        </tr>
                    </table>
                </c:if>
                <c:if test="${tree.hasChilds}">
                    <c:set var="ch" value="${tree.childs}"/>
                    <c:forEach items="${ch}" var="catItem">
                        <c:if test="${!catItem.value.hasChilds}">
                            <table class=properties_list cellspacing=0><col width=150px>
                                <tr class=row0>
                                    <th nowrap>${catItem.value.name}</th>
                                    <td><select name="category_${catItem.value.fullName}" id="category_${catItem.value.fullName}"
                                                style="font-size:80%;">
                                        <c:forEach items="${bean.priorities}" var="i">
                                            <c:choose>
                                                <c:when test="${catItem.value.priority==i}">
                                                    <option value="${fn:escapeXml(i)}"
                                                            selected="true">${fn:escapeXml(i)}</option>
                                                </c:when>
                                                <c:otherwise>
                                                    <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                                </c:otherwise>
                                            </c:choose>
                                        </c:forEach>
                                    </select>
                                    </td>
                                </tr>
                            </table>
                        </c:if>
                        </c:forEach>
                        <c:forEach items="${ch}" var="catItem">
                        <c:if test="${catItem.value.hasChilds}">
                            <div class=collapsing_tree_closed id="sectionHeader_${catItem.value.fullName}"
                                 onclick="collasping_tree_showhide_section('${catItem.value.fullName}')">
                                <table cellspacing=0>
                                    <tr>
                                        <td width=100%><img id="img_${catItem.value.fullName}" src="content/images/list_closed.gif" alt="">&nbsp;${catItem.value.name}</td>
                                        <td>

                                            <%--<select name="category_${catItem.value.fullName}" id="category_${catItem.value.fullName}" class="txt" style="font-size:80%;">--%>
                                                <%--<c:forEach items="${bean.priorities}" var="i">--%>
                                                    <%--<c:choose>--%>
                                                        <%--<c:when test="${catItem.value.priority==i}">--%>
                                                            <%--<option value="${fn:escapeXml(i)}"--%>
                                                                    <%--selected="true">${fn:escapeXml(i)}--%>
                                                            <!--</option>-->
                                                        <%--</c:when>--%>
                                                        <%--<c:otherwise>--%>
                                                            <%--<option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>--%>
                                                        <%--</c:otherwise>--%>
                                                    <%--</c:choose>--%>
                                                <%--</c:forEach>--%>
                                            <!--</select>-->
                                        </td>
                                    </tr>
                                </table>
                            </div>
                            <table cellspacing="0" cellpadding="0" id="sectionValue_${catItem.value.fullName}"
                                   style="display:none">
                                <col width='56px'/>
                                <tr>
                                    <th/>
                                    <td>
                                        <c:set var="chh" value="${catItem.value.childs}"/>
                                        <c:forEach items="${chh}" var="catItem2">
                                            <c:choose>
                                                <c:when test="${catItem2.value.hasChilds}">
                                                    <div class=collapsing_tree_closed
                                                         id="sectionHeader_${catItem2.value.fullName}"
                                                         onclick="collasping_tree_showhide_section('${catItem2.value.fullName}')">
                                                        <table cellspacing=0>
                                                            <tr>
                                                                <td width=100%><img id="img_${catItem2.value.fullName}" src="content/images/list_closed.gif" alt="">&nbsp;${catItem2.value.name}</td>
                                                                <td>
                                                                    <%--<select name="category_${catItem2.value.fullName}"  id="category_${catItem2.value.fullName}"--%>
                                                                            <!--class="txt"-->
                                                                            <%--style="font-size:80%;">--%>
                                                                        <%--<c:forEach items="${bean.priorities}" var="i">--%>
                                                                            <%--<c:choose>--%>
                                                                                <%--<c:when test="${catItem2.value.priority==i}">--%>
                                                                                    <%--<option value="${fn:escapeXml(i)}"--%>
                                                                                            <%--selected="true">${fn:escapeXml(i)}--%>
                                                                                    <!--</option>-->
                                                                                <%--</c:when>--%>
                                                                                <%--<c:otherwise>--%>
                                                                                    <%--<option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>--%>
                                                                                <%--</c:otherwise>--%>
                                                                            <%--</c:choose>--%>
                                                                        <%--</c:forEach>--%>
                                                                    <!--</select>-->
                                                                </td>
                                                            </tr>
                                                        </table>
                                                    </div>


                                                    <table cellspacing="0" cellpadding="0"
                                                           id="sectionValue_${catItem2.value.fullName}"
                                                           style="display:none">
                                                        <col width='56px'/>
                                                        <tr>
                                                            <th/>
                                                            <td>
                                                                <c:set var="ch3" value="${catItem2.value.childs}"/>
                                                                <c:forEach items="${ch3}" var="catItem3">
                                                                    <c:choose>
                                                                        <c:when test="${catItem3.value.hasChilds}">
                                                                            <div class=collapsing_tree_closed
                                                                                 id="sectionHeader_${catItem3.value.fullName}"
                                                                                 onclick="collasping_tree_showhide_section('${catItem3.value.fullName}')">
                                                                                <table cellspacing=0>
                                                                                    <tr>
                                                                                        <td width=100%><img id="img_${catItem3.value.fullName}" src="content/images/list_closed.gif" alt="">&nbsp;${catItem3.value.name}</td>
                                                                                        <td>
                                                                                            <%--<select name="category_${catItem3.value.fullName}"  id="category_${catItem3.value.fullName}"--%>
                                                                                                    <!--class="txt"-->
                                                                                                    <%--style="font-size:80%;">--%>
                                                                                                <%--<c:forEach--%>
                                                                                                        <%--items="${bean.priorities}"--%>
                                                                                                        <%--var="i">--%>
                                                                                                    <%--<c:choose>--%>
                                                                                                        <%--<c:when test="${catItem3.value.priority==i}">--%>
                                                                                                            <%--<option value="${fn:escapeXml(i)}"--%>
                                                                                                                    <%--selected="true">${fn:escapeXml(i)}--%>
                                                                                                            <!--</option>-->
                                                                                                        <%--</c:when>--%>
                                                                                                        <%--<c:otherwise>--%>
                                                                                                            <%--<option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>--%>
                                                                                                        <%--</c:otherwise>--%>
                                                                                                    <%--</c:choose>--%>
                                                                                                <%--</c:forEach>--%>
                                                                                            <!--</select>-->
                                                                                        </td>
                                                                                    </tr>
                                                                                </table>
                                                                            </div>

                                                                            <!---------------------------- BEGIN 1-------------------------------------------->
                                                                            <table cellspacing="0" cellpadding="0"
                                                                                   id="sectionValue_${catItem3.value.fullName}"
                                                                                   style="display:none">
                                                                                <col width='56px'/>
                                                                                <tr>
                                                                                    <th/>
                                                                                    <td>
                                                                                        <c:set var="ch4"
                                                                                               value="${catItem3.value.childs}"/>
                                                                                        <c:forEach items="${ch4}"
                                                                                                   var="catItem4">
                                                                                            <c:choose>
                                                                                                <c:when test="${catItem4.value.hasChilds}">
                                                                                                    <div class=collapsing_tree_closed
                                                                                                         id="sectionHeader_${catItem4.value.fullName}"
                                                                                                         onclick="collasping_tree_showhide_section('${catItem4.value.fullName}')">
                                                                                                        <table cellspacing=0>
                                                                                                            <tr>
                                                                                                                <td width=100%><img id="img_${catItem4.value.fullName}" src="content/images/list_closed.gif" alt="">&nbsp;${catItem4.value.name}</td>
                                                                                                                <td>
                                                                                                                    <%--<select name="category_${catItem4.value.fullName}"  id="category_${catItem4.value.fullName}"--%>
                                                                                                                            <!--class="txt"-->
                                                                                                                            <%--style="font-size:80%;">--%>
                                                                                                                        <%--<c:forEach--%>
                                                                                                                                <%--items="${bean.priorities}"--%>
                                                                                                                                <%--var="i">--%>
                                                                                                                            <%--<c:choose>--%>
                                                                                                                                <%--<c:when test="${catItem4.value.priority==i}">--%>
                                                                                                                                    <%--<option value="${fn:escapeXml(i)}"--%>
                                                                                                                                            <%--selected="true">${fn:escapeXml(i)}--%>
                                                                                                                                    <!--</option>-->
                                                                                                                                <%--</c:when>--%>
                                                                                                                                <%--<c:otherwise>--%>
                                                                                                                                    <%--<option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>--%>
                                                                                                                                <%--</c:otherwise>--%>
                                                                                                                            <%--</c:choose>--%>
                                                                                                                        <%--</c:forEach>--%>
                                                                                                                    <!--</select>-->
                                                                                                                </td>
                                                                                                            </tr>
                                                                                                        </table>
                                                                                                    </div>

                                                                                                    <!---------------------------- BEGIN 2-------------------------------------------->
                                                                            <table cellspacing="0" cellpadding="0"
                                                                                   id="sectionValue_${catItem4.value.fullName}"
                                                                                   style="display:none">
                                                                                <col width='56px'/>
                                                                                <tr>
                                                                                    <th/>
                                                                                    <td>
                                                                                        <c:set var="ch5"
                                                                                               value="${catItem4.value.childs}"/>
                                                                                        <c:forEach items="${ch5}"
                                                                                                   var="catItem5">
                                                                                            <c:choose>
                                                                                                <c:when test="${catItem5.value.hasChilds}">
                                                                                                    <div class=collapsing_tree_closed
                                                                                                         id="sectionHeader_${catItem5.value.fullName}"
                                                                                                         onclick="collasping_tree_showhide_section('${catItem5.value.fullName}')">
                                                                                                        <table cellspacing=0>
                                                                                                            <tr>
                                                                                                                <td width=100%><img id="img_${catItem5.value.fullName}" src="content/images/list_closed.gif" alt="">&nbsp;${catItem5.value.name}</td>
                                                                                                                <td>
                                                                                                                    <%--<select name="category_${catItem5.value.fullName}"  id="category_${catItem5.value.fullName}"--%>
                                                                                                                            <!--class="txt"-->
                                                                                                                            <%--style="font-size:80%;">--%>
                                                                                                                        <%--<c:forEach--%>
                                                                                                                                <%--items="${bean.priorities}"--%>
                                                                                                                                <%--var="i">--%>
                                                                                                                            <%--<c:choose>--%>
                                                                                                                                <%--<c:when test="${catItem5.value.priority==i}">--%>
                                                                                                                                    <%--<option value="${fn:escapeXml(i)}"--%>
                                                                                                                                            <%--selected="true">${fn:escapeXml(i)}--%>
                                                                                                                                    <!--</option>-->
                                                                                                                                <%--</c:when>--%>
                                                                                                                                <%--<c:otherwise>--%>
                                                                                                                                    <%--<option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>--%>
                                                                                                                                <%--</c:otherwise>--%>
                                                                                                                            <%--</c:choose>--%>
                                                                                                                        <%--</c:forEach>--%>
                                                                                                                    <!--</select>-->
                                                                                                                </td>
                                                                                                            </tr>
                                                                                                        </table>
                                                                                                    </div>


                                                                                                </c:when>
                                                                                                <c:otherwise>
                                                                                                    <table class="properties_list"
                                                                                                           cellspacing="0">
                                                                                                        <col width="150px">
                                                                                                        <tr class=row${rowNum%2}>
                                                                                                            <th nowrap>${catItem5.value.name}</th>
                                                                                                            <td>
                                                                                                                <select name="category_${catItem5.value.fullName}" id="category_${catItem5.value.fullName}"
                                                                                                                        class="txt"
                                                                                                                        style="font-size:80%;">
                                                                                                                    <c:forEach
                                                                                                                            items="${bean.priorities}"
                                                                                                                            var="i">
                                                                                                                        <c:choose>
                                                                                                                            <c:when test="${catItem5.value.priority==i}">
                                                                                                                                <option value="${fn:escapeXml(i)}"
                                                                                                                                        selected="true">${fn:escapeXml(i)}
                                                                                                                                </option>
                                                                                                                            </c:when>
                                                                                                                            <c:otherwise>
                                                                                                                                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                                                                                                            </c:otherwise>
                                                                                                                        </c:choose>
                                                                                                                    </c:forEach>
                                                                                                                </select>
                                                                                                            </td>
                                                                                                        </tr>
                                                                                                    </table>
                                                                                                </c:otherwise>
                                                                                            </c:choose>
                                                                                        </c:forEach>
                                                                                    </td>
                                                                                </tr>
                                                                            </table>

                                                                            <!---------------------------------  END 2--------------------------------------->


                                                                                                </c:when>
                                                                                                <c:otherwise>
                                                                                                    <table class="properties_list"
                                                                                                           cellspacing="0">
                                                                                                        <col width="150px">
                                                                                                        <tr class=row${rowNum%2}>
                                                                                                            <th nowrap>${catItem4.value.name}</th>
                                                                                                            <td>
                                                                                                                <select name="category_${catItem4.value.fullName}"  id="category_${catItem4.value.fullName}"
                                                                                                                        class="txt"
                                                                                                                        style="font-size:80%;">
                                                                                                                    <c:forEach
                                                                                                                            items="${bean.priorities}"
                                                                                                                            var="i">
                                                                                                                        <c:choose>
                                                                                                                            <c:when test="${catItem4.value.priority==i}">
                                                                                                                                <option value="${fn:escapeXml(i)}"
                                                                                                                                        selected="true">${fn:escapeXml(i)}
                                                                                                                                </option>
                                                                                                                            </c:when>
                                                                                                                            <c:otherwise>
                                                                                                                                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                                                                                                            </c:otherwise>
                                                                                                                        </c:choose>
                                                                                                                    </c:forEach>
                                                                                                                </select>
                                                                                                            </td>
                                                                                                        </tr>
                                                                                                    </table>
                                                                                                </c:otherwise>
                                                                                            </c:choose>
                                                                                        </c:forEach>
                                                                                    </td>
                                                                                </tr>
                                                                            </table>

                                                                            <!---------------------------------  END 1--------------------------------------->


                                                                        </c:when>
                                                                        <c:otherwise>
                                                                            <table class="properties_list"
                                                                                   cellspacing="0">
                                                                                <col width="150px">
                                                                                <tr class=row${rowNum%2}>
                                                                                    <th nowrap>${catItem3.value.name}</th>
                                                                                    <td>
                                                                                        <select name="category_${catItem3.value.fullName}" id="category_${catItem3.value.fullName}"
                                                                                                class="txt"
                                                                                                style="font-size:80%;">
                                                                                            <c:forEach
                                                                                                    items="${bean.priorities}"
                                                                                                    var="i">
                                                                                                <c:choose>
                                                                                                    <c:when test="${catItem3.value.priority==i}">
                                                                                                        <option value="${fn:escapeXml(i)}"
                                                                                                                selected="true">${fn:escapeXml(i)}
                                                                                                        </option>
                                                                                                    </c:when>
                                                                                                    <c:otherwise>
                                                                                                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                                                                                    </c:otherwise>
                                                                                                </c:choose>
                                                                                            </c:forEach>
                                                                                        </select>
                                                                                    </td>
                                                                                </tr>
                                                                            </table>
                                                                        </c:otherwise>
                                                                    </c:choose>
                                                                </c:forEach>
                                                            </td>
                                                        </tr>
                                                    </table>



                                                </c:when>
                                                <c:otherwise>
                                                    <table class="properties_list" cellspacing="0">
                                                        <col width="150px">
                                                        <tr class=row${rowNum%2}>
                                                            <th nowrap>${catItem2.value.name}</th>
                                                            <td>
                                                                <select name="category_${catItem2.value.fullName}" id="category_${catItem2.value.fullName}"
                                                                        class="txt"
                                                                        style="font-size:80%;">
                                                                    <c:forEach items="${bean.priorities}" var="i">
                                                                        <c:choose>
                                                                            <c:when test="${catItem2.value.priority==i}">
                                                                                <option value="${fn:escapeXml(i)}"
                                                                                        selected="true">${fn:escapeXml(i)}
                                                                                </option>
                                                                            </c:when>
                                                                            <c:otherwise>
                                                                                <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                                                                            </c:otherwise>
                                                                        </c:choose>
                                                                    </c:forEach>
                                                                </select>
                                                            </td>
                                                        </tr>
                                                    </table>
                                                </c:otherwise>
                                            </c:choose>
                                        </c:forEach>
                                    </td>
                                </tr>
                            </table>
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
            <sm-pm:item name="mbSave" value="item.mbapply.value" enabled="true" title="item.mbapply.title"/>
            <sm-pm:item name="mbSavePermanent" value="logging.item.save.permanent.value" enabled="true"
                        title="logging.item.save.permanent.title"
                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </c:when>
    <c:otherwise>
        <sm-pm:menu>
            <sm-pm:item name="mbSave" value="item.mbapply.value" enabled="false" title="item.mbapply.title"/>
            <sm-pm:item name="mbSavePermanent" value="logging.item.save.permanent.value" enabled="false"
                        title="logging.item.save.permanent.title"
                        onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </c:otherwise>
</c:choose>
</jsp:body>
</sm:page>