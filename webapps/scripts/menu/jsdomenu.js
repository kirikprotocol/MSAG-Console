/*

  jsDOMenu Version 1.2.1
	Copyright (C) 2003 - 2004 Toh Zhiqiang
  Released on 15 July 2004
  jsDOMenu is distributed under the terms of the GNU GPL license
  Refer to license.txt for more informatiom

*/

/*
Determine whether the browser is IE5 or IE5.5.
*/
function isIE5() { // Private method
    return (navigator.userAgent.indexOf("MSIE 5") > -1);
}

/*
Determine whether the browser is IE6.
*/
function isIE6() { // Private method
    return ((navigator.userAgent.indexOf("MSIE 6") > -1) && (navigator.userAgent.indexOf("Opera") == -1));
}

/*
Determine whether the browser is IE.
*/
function isIE() { // Private method
    return (isIE5() || isIE6());
}

/*
Determine whether the browser is Opera.
*/
function isOpera() { // Private method
    return (navigator.userAgent.indexOf("Opera") > -1);
}

/*
Determine whether the browser is Safari.
*/
function isSafari() { // Private method
    return ((navigator.userAgent.indexOf("Safari") > -1) && (navigator.userAgent.indexOf("Mac") > -1));
}

function getBrawserType() {
    if (isIE()) {
        return "IE";
    } else {
        return "OTHER";
    }
}

/*
Determine the page render mode.

0: Quirks mode.
1: Strict mode.
*/
function getPageMode() { // Private method
    if (document.compatMode) {
        switch (document.compatMode) {
            case "BackCompat":
                return 0;
            case "CSS1Compat":
                return 1;
            case "QuirksMode":
                return 0;
        }
    }
    else {
        if (isIE5()) {
            return 0;
        }
        if (isSafari()) {
            return 1;
        }
    }
    return 0;
}

/*
Alias for document.getElementById().
*/
function getElmId(id) { // Private method
    return document.getElementById(id);
}

/*
Alias for document.createElement().
*/
function createElm(tagName) { // Private method
    return document.createElement(tagName);
}

/*
Get the x-coordinate of the cursor position relative to the window.
*/
function getX(e) { // Private method
    if (!e) {
        var e = window.event;
    }
    if (isSafari()) {
        return (e.clientX - getScrollLeft());
    }
    else {
        return e.clientX;
    }
}

/*
Get the y-coordinate of the cursor position relative to the window.
*/
function getY(e) { // Private method
    if (!e) {
        var e = window.event;
    }
    if (isSafari()) {
        return (e.clientY - getScrollTop());
    }
    else {
        return e.clientY;
    }
}

/*
Get the scrollLeft property.
*/
function getScrollLeft() { // Private method
    switch (pageMode) {
        case 0:
            return document.body.scrollLeft;
        case 1:
            if (document.documentElement && (document.documentElement.scrollLeft > 0)) {
                return document.documentElement.scrollLeft;
            }
            else {
                return document.body.scrollLeft;
            }
    }
}

/*
Get the scrollTop property.
*/
function getScrollTop() { // Private method
    switch (pageMode) {
        case 0:
            return document.body.scrollTop;
        case 1:
            if (document.documentElement && (document.documentElement.scrollTop > 0)) {
                return document.documentElement.scrollTop;
            }
            else {
                return document.body.scrollTop;
            }
    }
}

/*
Get the clientHeight property.
*/
function getClientHeight() { // Private method
    switch (pageMode) {
        case 0:
            return document.body.clientHeight;
        case 1:
            if (isSafari()) {
                return self.innerHeight;
            }
            else {
                if ((!isOpera()) && document.documentElement && (document.documentElement.clientHeight > 0)) {
                    return document.documentElement.clientHeight;
                }
                else {
                    return document.body.clientHeight;
                }
            }
    }
}

/*
Get the clientWidth property.
*/
function getClientWidth() { // Private method
    switch (pageMode) {
        case 0:
            return document.body.clientWidth;
        case 1:
            if (isSafari()) {
                return self.innerWidth;
            }
            else {
                if ((!isOpera()) && document.documentElement && (document.documentElement.clientWidth > 0)) {
                    return document.documentElement.clientWidth;
                }
                else {
                    return document.body.clientWidth;
                }
            }
    }
}

/*
Get the left position of the pop-up menu.
*/
function getMainMenuLeftPos(menuObj, x) { // Private method
    if ((x + menuObj.offsetWidth) <= getClientWidth()) {
        return x;
    }
    else {
        if (x <= getClientWidth()) {
            return (x - menuObj.offsetWidth);
        }
        else {
            return (getClientWidth() - menuObj.offsetWidth);
        }
    }
}

/*
Get the top position of the pop-up menu.
*/
function getMainMenuTopPos(menuObj, y) { // Private method
    if ((y + menuObj.offsetHeight) <= getClientHeight()) {
        return y;
    }
    else {
        if (y <= getClientHeight()) {
            return (y - menuObj.offsetHeight);
        }
        else {
            return (getClientHeight() - menuObj.offsetHeight);
        }
    }
}

/*
Get the left position of the submenu.
*/
function getSubMenuLeftPos(menuObj, x, offset) { // Private method
    var borderLeftWidth = parseInt(getPropertyValue(menuObj, "border-left-width"));
    if ((x + menuObj.offsetWidth - borderLeftWidth - 5) <= getClientWidth()) {
        return (x - borderLeftWidth - 5);
    }
    else {
        if (x <= getClientWidth()) {
            return (x - menuObj.offsetWidth - offset);
        }
        else {
            return (getClientWidth() - menuObj.offsetWidth);
        }
    }
}

/*
Get the top position of the submenu.
*/
function getSubMenuTopPos(menuObj, y, offset) { // Private method
    var borderTopWidth = parseInt(getPropertyValue(menuObj, "border-top-width"));
    if ((y + menuObj.offsetHeight) <= getClientHeight()) {
        if (isSafari()) {
            return (y - borderTopWidth);
        }
        else {
            return y;
        }
    }
    else {
        if (y <= getClientHeight()) {
            if (isSafari()) {
                return (y - borderTopWidth - menuObj.offsetHeight + offset);
            }
            else {
                return (y - menuObj.offsetHeight + offset);
            }
        }
        else {
            return (getClientHeight() - menuObj.offsetHeight + offset);
        }
    }
}

/*
Pop up the main menu.
*/
function popUpMainMenu(menuObj, e) { // Private method
    menuObj.style.left = (getMainMenuLeftPos(menuObj, getX(e)) + getScrollLeft()) + "px";
    menuObj.style.top = (getMainMenuTopPos(menuObj, getY(e)) + getScrollTop()) + "px";
    popUpMenuObj.menuObj.style.visibility = "visible";
}

/*
Pop up the submenu.
*/
function popUpSubMenu(menuItemObj) { // Private method
    var parentMenuObj = menuItemObj.parent.menuObj;
    var menuObj = menuItemObj.subMenu.menuObj;
    var borderTopWidth = parseInt(getPropertyValue(menuObj, "border-top-width"));
    var borderBottomWidth = parseInt(getPropertyValue(menuObj, "border-bottom-width"));
    if (menuObj.style.position == "fixed") {
        var x = parentMenuObj.offsetLeft + parentMenuObj.offsetWidth;
        var y = parentMenuObj.offsetTop + menuItemObj.offsetTop;
        menuObj.style.position = "absolute";
        menuObj.style.left = getSubMenuLeftPos(menuObj, x, menuItemObj.offsetWidth) + "px";
        menuObj.style.top = getSubMenuTopPos(menuObj, y, menuItemObj.offsetHeight + borderBottomWidth + borderTopWidth) + "px";
        menuObj.style.position = "fixed";
    }
    else {
        var x = parentMenuObj.offsetLeft + parentMenuObj.offsetWidth - getScrollLeft();
        var y = parentMenuObj.offsetTop + menuItemObj.offsetTop - getScrollTop();
        menuObj.style.left = (getSubMenuLeftPos(menuObj, x, menuItemObj.offsetWidth) + getScrollLeft()) + "px";
        menuObj.style.top = (getSubMenuTopPos(menuObj, y, menuItemObj.offsetHeight + borderBottomWidth + borderTopWidth) + getScrollTop()) + "px";
    }
    if ((isIE()) && (menuObj.mode == "fixed")) {
        menuObj.initialLeft = parseInt(menuObj.style.left) - getScrollLeft();
        menuObj.initialTop = parseInt(menuObj.style.top) - getScrollTop();
    }
    menuObj.style.visibility = "visible";
}


/*
Convert the string into lower camel case.
*/
function toCamelCase(input) { // Private method
    var inputArray = input.split("-");
    if (inputArray.length == 1) {
        return inputArray[0];
    }
    else {
        var camelCase = inputArray[0];
        var i;
        for (i = 1; i < inputArray.length; i++) {
            var word = inputArray[i];
            camelCase += word.charAt(0).toUpperCase() + word.substring(1);
        }
        return camelCase;
    }
}

/*
Get the value of the property of the object.
*/
function getPropertyValue(obj, propertyName) { // Private method
    var propertyValue = obj.style[toCamelCase(propertyName)];
    if (propertyValue) {
        return propertyValue;
    }
    else {
        if (document.defaultView && document.defaultView.getComputedStyle) {
            return document.defaultView.getComputedStyle(obj, null).getPropertyValue(propertyName);
        }
        else {
            if (obj.currentStyle) {
                return obj.currentStyle[toCamelCase(propertyName)];
            }
            else {
                return null;
            }
        }
    }
}

/*
Refresh the menu items.
*/
function refreshMenuItems(menuObj) { // Private method
    var i;
    for (i = 0; i < menuObj.childNodes.length; i++) {
        if (menuObj.childNodes[i].enabled) {
            menuObj.childNodes[i].className = menuObj.childNodes[i].itemClassName;
            if (menuObj.childNodes[i].hasSubMenu) {
                var arrowObj = getElmId(menuObj.childNodes[i].id + "Arrow");
                arrowObj.className = menuObj.childNodes[i].arrowClassName;
            }
        }
    }
}

/*
Event handler that handles onmouseover event of the menu item.
@note: applies on SUB-items
*/
function menuItemOver(e) { // Private method
    if (this.parent.previousItem) {
        if (this.parent.previousItem.className == this.parent.previousItem.itemClassNameOver) {
            this.parent.previousItem.className = this.parent.previousItem.itemClassName;
        }
        if (this.parent.previousItem.hasSubMenu) {
            this.parent.previousItem.className = this.parent.previousItem.itemClassName;
            var arrowObj = getElmId(this.parent.previousItem.id + "Arrow");
            arrowObj.className = this.parent.previousItem.arrowClassName;
        }
        var menuObj = getElmId(this.parent.menuObj.id);
        var i;
        for (i = 0; i < menuObj.childNodes.length; i++) {
            if ((menuObj.childNodes[i].enabled) && (menuObj.childNodes[i].hasSubMenu)) {
                hideMenus(menuObj.childNodes[i].subMenu.menuObj);
            }
        }
    }
    if (this.enabled) {
        this.className = this.itemClassNameOver;
        if (this.hasSubMenu) {
            var arrowObj = getElmId(this.id + "Arrow");
            arrowObj.className = this.arrowClassNameOver;
            popUpSubMenu(this);
        }
    }
    this.parent.previousItem = this;
}

/*
Event handler that handles onclick event of the menu item.
*/
function menuItemClick(e) { // Private method
    if ((this.enabled) && (this.actionOnClick)) {
        var action = this.actionOnClick;
        if (action.indexOf("link:") == 0) {
            location.href = action.substr(5);
        }
        else {
            if (action.indexOf("code:") == 0) {
                eval(action.substr(5));
            }
            else {
                location.href = action;
            }
        }
    }
    if (!e) {
        try {
            var e = window.event;
            e.cancelBubble = true;
        } catch (ex) {
        }
    }
    if (e.stopPropagation) {
        e.stopPropagation();
    }
    if (this.parent.menuObj.mode == "cursor") {
        hideCursorMenus();
    }
    if ((this.parent.menuObj.mode == "absolute") || (this.parent.menuObj.mode == "fixed")) {
        hideVisibleMenus();
        if (typeof(hideMenuBarMenus) != "undefined") {
            hideMenuBarMenus();
        }
    }
}

/*
Event handler that handles onmouseout event of the menu item.
*/
function menuItemOut() { // Private method
    if (this.enabled) {
        if (!((this.hasSubMenu) && (this.subMenu.menuObj.style.visibility == "visible"))) {
            this.className = this.itemClassName;
        }
        if (this.hasSubMenu) {
            var arrowObj = getElmId(this.id + "Arrow");
            if (this.subMenu.menuObj.style.visibility == "visible") {
                arrowObj.className = this.arrowClassNameOver;
            }
            else {
                arrowObj.className = this.arrowClassName;
            }
        }
    }
}

/*
Set the menu object that will show up when the cursor is over the menu item object.
Argument:
menuObj        : Required. Menu object that will show up when the cursor is over the menu item object.
*/
function setSubMenu(menuObj) { // Public method
    var arrowElm = createElm("div");
    arrowElm.id = this.id + "Arrow";
    arrowElm.className = this.arrowClassName;
    this.appendChild(arrowElm);
    var height;
    if (isIE()) {
        var height = parseInt(getPropertyValue(arrowElm, "height"));
    }
    else {
        var height = arrowElm.offsetHeight;
    }
    arrowElm.style.top = Math.floor((this.offsetHeight - height) / 2) + "px";
    this.hasSubMenu = true;
    this.subMenu = menuObj;
    this.arrowObj = arrowElm;
    this.setArrowClassName = function(className) { // Public method
        if (this.arrowObj) {
            this.arrowClassName = className;
            this.arrowObj.className = this.arrowClassName;
        }
    };
    this.setArrowClassNameOver = function(className) { // Public method
        if (this.arrowObj) {
            this.arrowClassNameOver = className;
        }
    };
    menuObj.menuObj.style.zIndex = this.parent.menuObj.level + 1;
    menuObj.menuObj.level = this.parent.menuObj.level + 1;
}

/*
Add a new menu item to the menu.
Argument:
menuItemObj        : Required. Menu item object that is going to be added to the menu object.
*/
function addMenuItem(menuItemObj) { // Public method
    if (menuItemObj.displayText == "-") {
        var hrElm = createElm("hr");
        var itemElm = createElm("div");
        itemElm.appendChild(hrElm);
        itemElm.id = menuItemObj.id;
        if (menuItemObj.className.length > 0) {
            itemElm.sepClassName = menuItemObj.className;
        }
        else {
            itemElm.sepClassName = menuItemObj.sepClassName;
        }
        itemElm.className = itemElm.sepClassName;
        this.menuObj.appendChild(itemElm);
        itemElm.parent = this;
        itemElm.setClassName = function(className) { // Public method
            this.sepClassName = className;
            this.className = this.sepClassName;
        };
        itemElm.onclick = function(e) { // Private method
            if (!e) {
                try {
                    var e = window.event;
                    e.cancelBubble = true;
                } catch(ex) {
                }
            }
            if (e.stopPropagation) {
                e.stopPropagation();
            }
        };
        itemElm.onmouseover = menuItemOver;
        if (menuItemObj.itemName.length > 0) {
            this.items[menuItemObj.itemName] = itemElm;
        }
        else {
            this.items[this.items.length] = itemElm;
        }
    }
    else {
        var itemElm = createElm("div");
        itemElm.id = menuItemObj.id;
        itemElm.actionOnClick = menuItemObj.actionOnClick;
        itemElm.enabled = menuItemObj.enabled;
        itemElm.itemClassName = menuItemObj.className;
        itemElm.itemClassNameOver = menuItemObj.classNameOver;
        itemElm.className = itemElm.itemClassName;
        itemElm.hasSubMenu = false;
        itemElm.subMenu = null;
        itemElm.arrowClassName = arrowClassName;
        itemElm.arrowClassNameOver = arrowClassNameOver;
        this.menuObj.appendChild(itemElm);
        var textNode = document.createTextNode(menuItemObj.displayText);
        itemElm.appendChild(textNode);
        itemElm.parent = this;
        itemElm.setClassName = function(className) { // Public method
            this.itemClassName = className;
            this.className = this.itemClassName;
        };
        itemElm.setClassNameOver = function(classNameOver) { // Public method
            this.itemClassNameOver = classNameOver;
        };
        itemElm.setDisplayText = function(text) { // Public method
            this.firstChild.nodeValue = text;
        };
        itemElm.setSubMenu = setSubMenu;
        itemElm.onmouseover = menuItemOver;
        itemElm.onclick = menuItemClick;
        itemElm.onmouseout = menuItemOut;
        if (menuItemObj.itemName.length > 0) {
            this.items[menuItemObj.itemName] = itemElm;
        }
        else {
            this.items[this.items.length] = itemElm;
        }
    }
}

/*
Create a new menu item object.
Arguments:
displayText        : Required. String that specifies the text to be displayed on the menu item. If
                     displayText is "-", a menu separator will be created instead.
itemName           : Optional. String that specifies the name of the menu item. Defaults to "" (no
                     name).
actionOnClick      : Optional. String that specifies the action to be done when the menu item is
                     being clicked. Defaults to "" (no action).
enabled            : Optional. Boolean that specifies whether the menu item is enabled/disabled.
                     Defaults to true.
className          : Optional. String that specifies the CSS class selector for the menu item.
                     Defaults to "jsdomenuitem".
classNameOver      : Optional. String that specifies the CSS class selector for the menu item when
                     the cursor is over it. Defaults to "jsdomenuitemover".
*/
function menuItem() { // Public method
    this.displayText = arguments[0];
    if (this.displayText == "-") {
        this.id = "MenuSep" + (++sepCount);
        this.className = sepClassName;
    }
    else {
        this.id = "MenuItem" + (++menuItemCount);
        this.className = menuItemClassName;
    }
    this.itemName = "";
    this.actionOnClick = "";
    this.enabled = true;
    this.classNameOver = menuItemClassNameOver;
    this.sepClassName = sepClassName;
    var length = arguments.length;
    if ((length > 1) && (arguments[1].length > 0)) {
        this.itemName = arguments[1];
    }
    if ((length > 2) && (arguments[2].length > 0)) {
        this.actionOnClick = arguments[2];
    }
    if ((length > 3) && (typeof(arguments[3]) == "boolean")) {
        this.enabled = arguments[3];
    }
    if ((length > 4) && (arguments[4].length > 0)) {
        if (arguments[4] == "-") {
            this.className = arguments[4];
            this.sepClassName = arguments[4];
        }
        else {
            this.className = arguments[4];
        }
    }
    if ((length > 5) && (arguments[5].length > 0)) {
        this.itemClassNameOver = arguments[5];
    }
}

/*
Create a new menu object.
Arguments:
width              : Required. Integer that specifies the width of the menu.
className          : Optional. String that specifies the CSS class selector for the menu. Defaults
                     to "jsdomenudiv".
mode               : Optional. String that specifies the mode of the menu. Defaults to "cursor".
alwaysVisible      : Optional. Boolean that specifies whether the menu is always visible. Defaults
                     to false.
*/
function jsDOMenu() { // Public method
    this.items = new Array();
    var menuElm = createElm("div");
    menuElm.id = "DOMenu" + (++menuCount);
    menuElm.level = 10;
    menuElm.previousItem = null;
    menuElm.allExceptFilter = allExceptFilter;
    menuElm.noneExceptFilter = noneExceptFilter;
    menuElm.className = menuClassName;
    menuElm.mode = "cursor";
    menuElm.alwaysVisible = false;
    menuElm.initialLeft = 0;
    menuElm.initialTop = 0;
    var length = arguments.length;
    if ((length > 1) && (arguments[1].length > 0)) {
        menuElm.className = arguments[1];
    }
    if ((length > 2) && (arguments[2].length > 0)) {
        switch (arguments[2]) {
            case "cursor":
                menuElm.style.position = "absolute";
                break;
            case "absolute":
                menuElm.style.position = "absolute";
                menuElm.mode = "absolute";
                break;
            case "fixed":
                if (isIE()) {
                    menuElm.style.position = "absolute";
                }
                else {
                    menuElm.style.position = "fixed";
                }
                menuElm.mode = "fixed";
                break;
        }
    }
    if ((length > 3) && (typeof(arguments[3]) == "boolean")) {
        menuElm.alwaysVisible = arguments[3];
    }
    with (menuElm.style) {
        width = arguments[0] + "px";
        left = "0px";
        top = "0px";
    }
    document.body.appendChild(menuElm);
    if (!getPropertyValue(menuElm, "border-left-width")) {
        menuElm.style.borderWidth = menuBorderWidth + "px";
    }
    this.menuObj = menuElm;
    this.addMenuItem = addMenuItem;
    this.setClassName = function(className) { // Public method
        this.menuObj.className = className;
    };
    this.setMode = function(mode) { // Public method
        switch (mode) {
            case "cursor":
                this.menuObj.style.position = "absolute";
                this.menuObj.mode = "cursor";
                break;
            case "absolute":
                this.menuObj.style.position = "absolute";
                this.menuObj.mode = "absolute";
                var left = parseInt(this.menuObj.style.left);
                var top = parseInt(this.menuObj.style.top);
                this.menuObj.initialLeft = left;
                this.menuObj.initialTop = top;
                break;
            case "fixed":
                if (isIE()) {
                    this.menuObj.style.position = "absolute";
                    var left = parseInt(this.menuObj.style.left);
                    var top = parseInt(this.menuObj.style.top);
                    this.menuObj.initialLeft = left;
                    this.menuObj.initialTop = top;
                }
                else {
                    this.menuObj.style.position = "fixed";
                }
                this.menuObj.mode = "fixed";
                break;
        }
    };
    this.setAlwaysVisible = function(alwaysVisible) { // Public method
        if (typeof(alwaysVisible) == "boolean") {
            this.menuObj.alwaysVisible = alwaysVisible;
        }
    };
    this.show = function() { // Public method
        this.menuObj.style.visibility = "visible";
    };
    this.hide = function() { // Public method
        this.menuObj.style.visibility = "hidden";
        if (this.menuObj.mode == "cursor") {
            with (this.menuObj.style) {
                left = "0px";
                top = "0px";
            }
            menuObj.initialLeft = 0;
            menuObj.initialTop = 0;
        }
    };
    this.setX = function(x) { // Public method
        this.menuObj.initialLeft = x;
        this.menuObj.style.left = x + "px";
    };
    this.setY = function(y) { // Public method
        this.menuObj.initialTop = y;
        this.menuObj.style.top = y + "px";
    };
    this.moveTo = function(x, y) { // Public method
        this.menuObj.initialLeft = x;
        this.menuObj.initialTop = y;
        this.menuObj.style.left = x + "px";
        this.menuObj.style.top = y + "px";
    };
    this.moveBy = function(x, y) { // Public method
        var left = parseInt(this.menuObj.style.left);
        var top = parseInt(this.menuObj.style.top);
        this.menuObj.initialLeft = left + x;
        this.menuObj.initialTop = top + y;
        this.menuObj.style.left = (left + x) + "px";
        this.menuObj.style.top = (top + y) + "px";
    };
    this.setAllExceptFilter = function(filter) { // Public method
        this.menuObj.allExceptFilter = filter;
        this.menuObj.noneExceptFilter = new Array();
    };
    this.setNoneExceptFilter = function(filter) { // Public method
        this.menuObj.noneExceptFilter = filter;
        this.menuObj.allExceptFilter = new Array();
    };
    this.setBorderWidth = function(width) { // Public method
        this.menuObj.style.borderWidth = width + "px";
    };
}


/*
Determine whether any of the tag name/tag id pair in the filter matches the tagName/tagId pair.
*/
function findMatch(tagName, tagId, filter) { // Private method
    var i;
    for (i = 0; i < filter.length; i++) {
        var filterArray = filter[i].toLowerCase().split(".");
        if (((filterArray[0] == "*") && (filterArray[1] == "*")) ||
            ((filterArray[0] == "*") && (filterArray[1] == tagId)) ||
            ((filterArray[0] == tagName) && (filterArray[1] == "*")) ||
            ((filterArray[0] == tagName) && (filterArray[1] == tagId))) {
            return true;
        }
    }
    return false;
}

/*
Determine whether to show or hide the menu.
*/
function canShowMenu(tagName, tagId, allExcept, noneExcept) { // Private method
    if (allExcept.length > 0) {
        return (!findMatch(tagName.toLowerCase(), tagId.toLowerCase(), allExcept));
    }
    else {
        if (noneExcept.length > 0) {
            return findMatch(tagName.toLowerCase(), tagId.toLowerCase(), noneExcept);
        }
        else {
            return true;
        }
    }
}

/*
Shows/Hides the pop-up menu.
*/
function activatePopUpMenu(e) { // Private method
    if (!popUpMenuObj) {
        return;
    }
    var state = popUpMenuObj.menuObj.style.visibility;
    if (state == "visible") {
        var i;
        for (i = 1; i <= menuCount; i++) {
            var menuObj = getElmId("DOMenu" + i);
            if (menuObj.mode == "cursor") {
                with (menuObj.style) {
                    visibility = "hidden";
                    left = "0px";
                    top = "0px";
                }
                menuObj.initialLeft = 0;
                menuObj.initialTop = 0;
                refreshMenuItems(menuObj);
            }
        }
    }
    else {
        if (!e) {
            var e = window.event;
        }
        var targetElm = (e.target) ? e.target : e.srcElement;
        if (targetElm.nodeType == 3) {
            targetElm = targetElm.parentNode;
        }
        if (canShowMenu(targetElm.tagName, targetElm.id, popUpMenuObj.menuObj.allExceptFilter, popUpMenuObj.menuObj.noneExceptFilter)) {
            popUpMainMenu(popUpMenuObj.menuObj, e);
        }
    }
}

/*
Specifies how the pop-up menu shows/hide.
Arguments:
showValue          : Required. Integer that specifies how the menu shows.
hideValue          : Optional. Integer that specifies how the menu hides. If not specified, the
                     menu shows/hides in the same manner.

0: Shows/Hides the menu by left click only.
1: Shows/Hides the menu by right click only.
2: Shows/Hides the menu by left or right click.
*/
function activatePopUpMenuBy() { // Public method
    showValue = ((typeof(arguments[0]) == "number") && (arguments[0] > -1)) ? arguments[0] : 0;
    if (arguments.length > 1) {
        hideValue = ((typeof(arguments[1]) == "number") && (arguments[1] > -1)) ? arguments[1] : 0;
    }
    else {
        hideValue = showValue;
    }
    if ((showValue == 1) || (showValue == 2) || (hideValue == 1) || (hideValue == 2)) {
        document.oncontextmenu = rightClickHandler;
    }
}

/*
Hide all menus, except those with alwaysVisible = true.
*/
function hideAllMenus() { // Public method
    var i;
    for (i = 1; i <= menuCount; i++) {
        var menuObj = getElmId("DOMenu" + i);
        if (!menuObj.alwaysVisible) {
            if (menuObj.style.position == "fixed") {
                menuObj.style.position == "absolute";
                menuObj.style.visibility = "hidden";
                menuObj.style.position == "fixed";
            }
            else {
                menuObj.style.visibility = "hidden";
                if (menuObj.mode == "cursor") {
                    with (menuObj.style) {
                        left = "0px";
                        top = "0px";
                    }
                    menuObj.initialLeft = 0;
                    menuObj.initialTop = 0;
                }
            }
        }
        refreshMenuItems(menuObj);
    }
}

/*
Hide all menus with mode = "cursor", except those with alwaysVisible = true.
*/
function hideCursorMenus() { // Public method
    var i;
    for (i = 1; i <= menuCount; i++) {
        var menuObj = getElmId("DOMenu" + i);
        if ((menuObj.mode == "cursor") && (!menuObj.alwaysVisible)) {
            with (menuObj.style) {
                visibility = "hidden";
                left = "0px";
                top = "0px";
            }
            menuObj.initialLeft = 0;
            menuObj.initialTop = 0;
        }
        if (menuObj.mode == "cursor") {
            refreshMenuItems(menuObj);
        }
    }
}

/*
Hide all menus with mode = "absolute" or mode = "fixed", except those with alwaysVisible = true.
*/
function hideVisibleMenus() { // Public method
    var i;
    for (i = 1; i <= menuCount; i++) {
        var menuObj = getElmId("DOMenu" + i);
        if (((menuObj.mode == "absolute") || (menuObj.mode == "fixed")) && (!menuObj.alwaysVisible)) {
            if (menuObj.style.position == "fixed") {
                menuObj.style.position = "absolute";
                menuObj.style.visibility = "hidden";
                menuObj.style.position = "fixed";
            }
            else {
                menuObj.style.visibility = "hidden";
                with (menuObj.style) {
                    left = "0px";
                    top = "0px";
                }
                menuObj.initialLeft = 0;
                menuObj.initialTop = 0;
            }
        }
        if ((menuObj.mode == "absolute") || (menuObj.mode == "fixed")) {
            refreshMenuItems(menuObj);
        }
    }
}

/*
Hide the menu and all its submenus.
Argument:
menuObj            : Required. Menu object that specifies the menu and all its submenus to be
                     hidden.
*/
function hideMenus(menuObj) { // Public method
    refreshMenuItems(menuObj);
    var i;
    for (i = 0; i < menuObj.childNodes.length; i++) {
        if ((menuObj.childNodes[i].enabled) && (menuObj.childNodes[i].hasSubMenu)) {
            hideMenus(menuObj.childNodes[i].subMenu.menuObj);
        }
    }
    if (menuObj.style.position == "fixed") {
        menuObj.style.position = "absolute";
        menuObj.style.visibility = "hidden";
        menuObj.style.position = "fixed";
    }
    else {
        menuObj.style.visibility = "hidden";
        with (menuObj.style) {
            left = "0px";
            top = "0px";
        }
        menuObj.initialLeft = 0;
        menuObj.initialTop = 0;
    }
}

/*
Event handler that handles left click event.
*/
function leftClickHandler(e) { // Private method
    if ((getX(e) > getClientWidth()) || (getY(e) > getClientHeight())) {
        return;
    }
    if (!e) {
        var e = window.event;
    }
    if ((e.button) && (e.button == 2)) {
        return;
    }
    hideVisibleMenus();
    if (typeof(hideMenuBarMenus) == "function") {
        hideMenuBarMenus();
    }
    if (popUpMenuObj) {
        var state = popUpMenuObj.menuObj.style.visibility;
        if ((state == "visible") && ((hideValue == 0) || (hideValue == 2))) {
            activatePopUpMenu(e);
        }
        if (((state == "hidden") || (state == "")) && ((showValue == 0) || (showValue == 2))) {
            activatePopUpMenu(e);
        }
    }
}

/*
Event handler that handles right click event.
*/
function rightClickHandler(e) { // Private method
    if ((getX(e) > getClientWidth()) || (getY(e) > getClientHeight())) {
        return;
    }
    if (popUpMenuObj) {
        var state = popUpMenuObj.menuObj.style.visibility;
        if ((state == "visible") && ((hideValue == 1) || (hideValue == 2))) {
            activatePopUpMenu(e);
            return false;
        }
        if (((state == "hidden") || (state == "")) && ((showValue == 1) || (showValue == 2))) {
            activatePopUpMenu(e);
            return false;
        }
    }
}

/*
Event handler that handles scroll event.
*/
function scrollHandler() { // Private method
    var i;
    for (i = 1; i <= menuCount; i++) {
        var menuObj = getElmId("DOMenu" + i);
        if ((isIE()) && (menuObj.mode == "fixed")) {
            with (menuObj.style) {
                left = (menuObj.initialLeft + getScrollLeft()) + "px";
                top = (menuObj.initialTop + getScrollTop()) + "px";
            }
        }
    }
    if (typeof(menuBarCount) == "number") {
        menuBarScrollHandler();
    }
}

/*
Set the menu object to be the pop-up menu.
Argument:
menuObj            : Required. Menu object that specifies the pop-up menu.
*/
function setPopUpMenu(menuObj) { // Public method
    popUpMenuObj = menuObj;
}

/*
Check browser compatibility and create the menus.
*/
function initjsDOMenu() { // Public method
    if (document.createElement && document.getElementById) {
        createjsDOMenu();
        //document.cookie = "BROWSER_TYPE" + "=" + getBrawserType();
        document.cookie = "BROWSER_TYPE" + "=" + escape(getBrawserType());
    }
}

if (typeof(allExceptFilter) == "undefined") {
    var allExceptFilter = new Array("A.*",
            "BUTTON.*",
            "IMG.*",
            "INPUT.*",
            "OBJECT.*",
            "OPTION.*",
            "SELECT.*",
            "TEXTAREA.*");
    // Public property
}

if (typeof(noneExceptFilter) == "undefined") {
    var noneExceptFilter = new Array();
    // Public property
}

if (typeof(menuClassName) == "undefined") {
    var menuClassName = "jsdomenudiv";
    // Public property
}

if (typeof(menuItemClassName) == "undefined") {
    var menuItemClassName = "jsdomenuitem";
    // Public property
}

if (typeof(menuItemClassNameOver) == "undefined") {
    var menuItemClassNameOver = "jsdomenuitemover";
    // Public property
}

if (typeof(sepClassName) == "undefined") {
    var sepClassName = "jsdomenusep";
    // Public property
}

if (typeof(arrowClassName) == "undefined") {
    var arrowClassName = "jsdomenuarrow";
    // Public property
}

if (typeof(arrowClassNameOver) == "undefined") {
    var arrowClassNameOver = "jsdomenuarrowover";
    // Public property
}

if (typeof(menuBorderWidth) == "undefined") {
    var menuBorderWidth = 2;
    // Public property
}

var menuCount = 0;
// Private property
var menuItemCount = 0;
// Private property
var sepCount = 0;
// Private property
var popUpMenuObj = null;
// Private property
var showValue = 0;
// Private property
var hideValue = 0;
// Private property
var pageMode = getPageMode();
// Private property
document.onclick = leftClickHandler;
window.onscroll = scrollHandler;
