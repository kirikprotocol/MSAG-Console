/*

  jsDOMenuBar Version 1.0.1
	Copyright (C) 2004 Toh Zhiqiang
  Released on 15 July 2004
  jsDOMenuBar is distributed under the terms of the GNU GPL license
  Refer to license.txt for more informatiom

*/

/*
Determine whether the browser is IE5.5.
*/
function isIE55() { // Private method
    return (navigator.userAgent.indexOf("MSIE 5.5") > -1);
}

/*
Get the left position of the menu bar menu.
*/
function getMainMenuBarMenuLeftPos(menuBarObj, menuBarItemObj, menuObj, x) { // Private method
    var barBorderLeftWidth = parseInt(getPropertyValue(menuBarObj, "border-left-width"));
    var menuBorderLeftWidth = parseInt(getPropertyValue(menuObj, "border-left-width"));
    if ((x + menuObj.offsetWidth) <= getClientWidth()) {
        if (isOpera()) {
            return (x - menuBorderLeftWidth);
        }
        else {
            return (x + barBorderLeftWidth - menuBorderLeftWidth);
        }
    }
    else {
        if (isOpera()) {
            return (x - menuObj.offsetWidth + menuBarItemObj.offsetWidth + menuBorderLeftWidth);
        }
        else {
            return (x - menuObj.offsetWidth + menuBarItemObj.offsetWidth + barBorderLeftWidth + menuBorderLeftWidth);
        }
    }
}

/*
Get the top position of the menu bar menu.
*/
function getMainMenuBarMenuTopPos(menuBarObj, menuObj, y) { // Private method
    if ((y + menuObj.offsetHeight) <= getClientHeight()) {
        return y;
    }
    else {
        return (y - menuObj.offsetHeight - menuBarObj.offsetHeight);
    }
}

/*
Pop up the menu bar menu.
*/
function popUpMenuBarMenu(menuBarObj, menuBarItemObj, menuObj) { // Private method
    if (menuObj.style.position == "fixed") {
        var x = menuBarObj.offsetLeft + menuBarItemObj.offsetLeft;
        var y = menuBarObj.offsetTop + menuBarObj.offsetHeight;
        var left = getMainMenuBarMenuLeftPos(menuBarObj, menuBarItemObj, menuObj, x);
        var right = getMainMenuBarMenuTopPos(menuBarObj, menuObj, y);
        menuObj.style.position = "absolute";
        menuObj.style.left = left + "px";
        menuObj.style.top = right + "px";
        menuObj.style.position = "fixed";
    }
    else {
        var x = menuBarObj.offsetLeft + menuBarItemObj.offsetLeft - getScrollLeft();
        var y = menuBarObj.offsetTop + menuBarObj.offsetHeight - getScrollTop();
        menuObj.style.left = (getMainMenuBarMenuLeftPos(menuBarObj, menuBarItemObj, menuObj, x) + getScrollLeft()) + "px";
        menuObj.style.top = (getMainMenuBarMenuTopPos(menuBarObj, menuObj, y) + getScrollTop()) + "px";
    }
    if ((isIE()) && (menuObj.mode == "fixed")) {
        menuObj.initialLeft = parseInt(menuObj.style.left) - getScrollLeft();
        menuObj.initialTop = parseInt(menuObj.style.top) - getScrollTop();
    }
    menuObj.style.visibility = "visible";
}

/*
Event handler that handles onmouseover event of the menu bar item.
*/
function menuBarItemOver(e) { // Private method
    //	if (this.parent.menuBarObj.activated) {
    if (!this.clicked) {
        var menuBarObj = this.parent.menuBarObj;
        var i;
        for (i = 0; i < menuBarObj.childNodes.length; i++) {
            if ((menuBarObj.childNodes[i].enabled) && (menuBarObj.childNodes[i].clicked)) {
                menuBarObj.childNodes[i].className = menuBarObj.childNodes[i].itemClassName;
                menuBarObj.childNodes[i].clicked = false;
                if (menuBarObj.childNodes[i].menu) {
                    hideMenus(menuBarObj.childNodes[i].menu.menuObj);
                }
                break;
            }
        }
        if (this.enabled) {
            if (this.menu) {
                this.onclick(e);
            }
            else {
                if (this.actionOnClick) {
                    this.className = this.itemClassNameClick;
                    this.clicked = true;
                }
            }
        }
    }
    /*
        }
        else {
            var menuBarObj = this.parent.menuBarObj;
            var i;
            for (i = 0; i < menuBarObj.childNodes.length; i++) {
                if (menuBarObj.childNodes[i].enabled) {
                    menuBarObj.childNodes[i].className = menuBarObj.childNodes[i].itemClassName;
                }
            }
            if ((this.enabled) && ((this.menu) || (this.actionOnClick))) {
                this.className = this.itemClassNameOver;
            }
        }
    */
}

/*
Event handler that handles onclick event of the menu bar item.
*/
function menuBarItemClick(e) { // Private method
    if (this.enabled) {
        if (this.menu) {
            if (this.clicked) {
                /*CBP:
                                this.className = this.itemClassNameOver;
                                hideMenus(this.menu.menuObj);
                                this.clicked = false;
                                this.parent.menuBarObj.activated = false;
                */
            }
            else {
                this.className = this.itemClassNameClick;
                popUpMenuBarMenu(this.parent.menuBarObj, this, this.menu.menuObj);
                this.clicked = true;
                this.parent.menuBarObj.activated = true;
            }
        }
        else {
            /*CBP

                        if (this.actionOnClick) {
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
                            this.className = this.itemClassName;
                            this.clicked = false;
                            this.parent.menuBarObj.activated = false;
                        }
            */

        }
    }
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
}

/*
Event handler that handles onmouseout event of the menu bar item.
*/
function menuBarItemOut() {
    if (!this.parent.menuBarObj.activated) {
        this.className = this.itemClassName;
    }
}

/*
Event handler that handles onmousedown event of the menu bar.
*/
function menuBarDown(e) {
    /*CBP*/
    draggingObj = this.parent.menuBarObj;
    var menuBarObj = this.parent.menuBarObj;
    menuBarObj.differenceLeft = getX(e) - menuBarObj.offsetLeft;
    menuBarObj.differenceTop = getY(e) - menuBarObj.offsetTop;
    document.onmousemove = mouseMoveHandler;
    /**/
}

/*
Event handler that handles onmouseup event of the menu bar.
*/
function menuBarUp() {
    alert('ssss');
    /*CBP*/
    draggingObj = null;
    var menuBarObj = this.parent.menuBarObj;
    menuBarObj.differenceLeft = 0;
    menuBarObj.differenceTop = 0;
    menuBarObj.initialLeft = menuBarObj.offsetLeft - getScrollLeft();
    menuBarObj.initialTop = menuBarObj.offsetTop - getScrollTop();
    document.onmousemove = null;
    /**/
}

/*
Add a new menu bar item to the menu bar.
Argument:
menuBarItemObj        : Required. Menu bar item object that is going to be added to the menu bar
                        object.
*/
function addMenuBarItem(menuBarItemObj) { // Public method
    var itemElm = createElm("div");
    itemElm.id = menuBarItemObj.id;
    itemElm.menu = menuBarItemObj.menu;
    itemElm.enabled = menuBarItemObj.enabled;
    itemElm.clicked = false;
    itemElm.actionOnClick = menuBarItemObj.actionOnClick;
    itemElm.itemClassName = menuBarItemObj.className;
    itemElm.itemClassNameOver = menuBarItemObj.classNameOver;
    itemElm.itemClassNameClick = menuBarItemObj.classNameClick;
    itemElm.className = itemElm.itemClassName;
    if (isIE5()) {
        itemElm.style.width = "0px";
    }
    if (isIE55()) {
        itemElm.style.width = "auto";
    }
    this.menuBarObj.appendChild(itemElm);
    var textNode = document.createTextNode(menuBarItemObj.displayText);
    itemElm.appendChild(textNode);
    itemElm.parent = this;
    itemElm.setClassName = function(className) { // Public method
        this.itemClassName = className;
        this.className = this.itemClassName;
    };
    itemElm.setClassNameOver = function(classNameOver) { // Public method
        this.itemClassNameOver = classNameOver;
    };
    itemElm.setClassNameClick = function(classNameClick) { // Public method
        this.itemClassNameClick = classNameClick;
    };
    itemElm.setDisplayText = function(text) { // Public method
        this.firstChild.nodeValue = text;
    };
    itemElm.setMenu = function(menu) {
        this.menu = menu;
    };
    itemElm.onmouseover = menuBarItemOver;
    itemElm.onclick = menuBarItemClick;
    itemElm.onmouseout = menuBarItemOut;
    if (menuBarItemObj.itemName.length > 0) {
        this.items[menuBarItemObj.itemName] = itemElm;
    }
    else {
        this.items[this.items.length] = itemElm;
    }
}

/*
Create a new menu bar item object.
Arguments:
displayText        : Required. String that specifies the text to be displayed on the menu bar item.
menuObj            : Optional. Menu object that is going to be the main menu for the menu bar item.
                     Defaults to null (no menu).
itemName           : Optional. String that specifies the name of the menu bar item. Defaults to ""
                     (no name).
enabled            : Optional. Boolean that specifies whether the menu bar item is enabled/disabled.
                     Defaults to true.
actionOnClick      : Optional. String that specifies the action to be done when the menu item is
                     being clicked if no main menu has been set for the menu bar item. Defaults to
                     "" (no action).
className          : Optional. String that specifies the CSS class selector for the menu bar item.
                     Defaults to "jsdomenubaritem".
classNameOver      : Optional. String that specifies the CSS class selector for the menu item when
                     the cursor is over it. Defaults to "jsdomenubaritemover".
classNameClick     : Optional. String that specifies the CSS class selector for the menu item when
                     the cursor is clicked on it. Defaults to "jsdomenubaritemclick".
*/
function menuBarItem() {
    this.displayText = arguments[0];
    this.id = "MenuBarItem" + (++menuBarItemCount);
    this.itemName = "";
    this.menu = null;
    this.enabled = true;
    this.actionOnClick = "";
    this.className = menuBarItemClassName;
    this.classNameOver = menuBarItemClassNameOver;
    this.classNameClick = menuBarItemClassNameClick;
    var length = arguments.length;
    if ((length > 1) && (typeof(arguments[1]) == "object")) {
        this.menu = arguments[1];
    }
    if ((length > 2) && (arguments[2].length > 0)) {
        this.itemName = arguments[2];
    }
    if ((length > 3) && (typeof(arguments[3]) == "boolean")) {
        this.enabled = arguments[3];
    }
    if ((length > 4) && (arguments[4].length > 0)) {
        this.actionOnClick = arguments[4];
    }
    if ((length > 5) && (arguments[5].length > 0)) {
        this.className = arguments[5];
    }
    if ((length > 6) && (arguments[6].length > 0)) {
        this.classNameOver = arguments[6];
    }
    if ((length > 7) && (arguments[7].length > 0)) {
        this.classNameClick = arguments[7];
    }
}

/*
Create a new menu bar object.
Arguments:
mode               : Optional. String that specifies the mode of the menu bar. Defaults to "absolute".
draggable          : Optional. Boolean that specifies whether the menu bar is draggable. Defaults to
                     false.
className          : Optional. String that specifies the CSS class selector for the menu bar. Defaults
                     to "jsdomenubardiv".
width              : Optional. Integer that specifies the width of the menu bar. Defaults to "auto".
height             : Optional. Integer that specifies the height of the menu bar. Defaults to "auto".
*/
function jsDOMenuBar() {
    this.items = new Array();
    var dragElm = createElm("div");
    dragElm.className = menuBarDragClassName;
    var textNode = document.createTextNode("");
    dragElm.appendChild(textNode);
    var menuBarElm = createElm("div");
    //CBP:	menuBarElm.appendChild(dragElm);
    menuBarElm.id = "DOMenuBar" + (++menuBarCount);
    menuBarElm.mode = "absolute";
    menuBarElm.draggable = false;
    menuBarElm.className = menuBarClassName;
    menuBarElm.activated = false;
    menuBarElm.initialLeft = 0;
    menuBarElm.initialTop = 0;
    menuBarElm.differenceLeft = 0;
    menuBarElm.differenceTop = 0;
    var length = arguments.length;
    if ((length > 0) && (arguments[0].length > 0)) {
        switch (arguments[0]) {
            case "absolute":
                menuBarElm.style.position = "absolute";
                break;
            case "fixed":
                if (isIE()) {
                    menuBarElm.style.position = "absolute";
                }
                else {
                    menuBarElm.style.position = "fixed";
                }
                menuBarElm.mode = "fixed";
                break;
        }
    }
    if ((length > 1) && (typeof(arguments[1]) == "boolean")) {
        menuBarElm.draggable = arguments[1];
        if (menuBarElm.draggable) {
            dragElm.style.visibility = "visible";
        }
        else {
            dragElm.style.visibility = "hidden";
        }
    }
    if ((length > 2) && (arguments[2].length > 0)) {
        menuBarElm.className = arguments[2];
    }
    if ((length > 3) && (typeof(arguments[3]) == "number") && (arguments[3] > 0)) {
        menuBarElm.style.width = arguments[3] + "px";
    }
    if ((length > 4) && (typeof(arguments[4]) == "number") && (arguments[4] > 0)) {
        menuBarElm.style.height = arguments[4] + "px";
    }
    with (menuBarElm.style) {
        left = "0px";
        top = "0px";
    }
    document.body.appendChild(menuBarElm);
    if (!getPropertyValue(menuBarElm, "border-left-width")) {
        menuBarElm.style.borderWidth = menuBarBorderWidth + "px";
    }
    this.menuBarObj = menuBarElm;
    this.dragObj = dragElm;
    dragElm.parent = this;
    this.addMenuBarItem = addMenuBarItem;
    this.menuBarObj.onclick = function(e) { // Private method
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
    dragElm.onmousedown = menuBarDown;
    dragElm.onmouseup = menuBarUp;
    this.setMode = function(mode) { // Public method
        switch (mode) {
            case "absolute":
                this.menuBarObj.style.position = "absolute";
                this.menuBarObj.mode = "absolute";
                var left = parseInt(this.menuBarObj.style.left);
                var top = parseInt(this.menuBarObj.style.top);
                this.menuBarObj.initialLeft = left;
                this.menuBarObj.initialTop = top;
                break;
            case "fixed":
                if (isIE()) {
                    this.menuBarObj.style.position = "absolute";
                    var left = parseInt(this.menuBarObj.style.left);
                    var top = parseInt(this.menuBarObj.style.top);
                    this.menuBarObj.initialLeft = left;
                    this.menuBarObj.initialTop = top;
                }
                else {
                    this.menuBarObj.style.position = "fixed";
                }
                this.menuBarObj.mode = "fixed";
                break;
        }
    };
    this.setDraggable = function(draggable) { // Public method
        if (typeof(draggable) == "boolean") {
            this.menuBarObj.draggable = draggable;
            if (this.menuBarObj.draggable) {
                this.dragObj.style.visibility = "visible";
            }
            else {
                this.dragObj.style.visibility = "hidden";
            }
        }
    };
    this.setClassName = function(className) { // Public method
        this.menuBarObj.className = className;
    };
    this.setDragClassName = function(className) { // Public method
        this.dragObj.className = className;
    };
    this.show = function() { // Public method
        this.menuBarObj.style.visibility = "visible";
    };
    this.hide = function() { // Public method
        this.menuBarObj.style.visibility = "hidden";
    };
    this.setX = function(x) { // Public method
        this.menuBarObj.initialLeft = x;
        this.menuBarObj.style.left = x + "px";
    };
    this.setY = function(y) { // Public method
        this.menuBarObj.initialTop = y;
        this.menuBarObj.style.top = y + "px";
    };
    this.moveTo = function(x, y) { // Public method
        this.menuBarObj.initialLeft = x;
        this.menuBarObj.initialTop = y;
        this.menuBarObj.style.left = x + "px";
        this.menuBarObj.style.top = y + "px";
    };
    this.moveBy = function(x, y) { // Public method
        var left = parseInt(this.menuBarObj.style.left);
        var top = parseInt(this.menuBarObj.style.top);
        this.menuBarObj.initialLeft = left + x;
        this.menuBarObj.initialTop = top + y;
        this.menuBarObj.style.left = (left + x) + "px";
        this.menuBarObj.style.top = (top + y) + "px";
    };
    this.setBorderWidth = function(width) { // Public method
        this.menuBarObj.style.borderWidth = width + "px";
    };
}

/*
Hide all menu bar menus.
*/
function hideMenuBarMenus() { // Public method
    var i, j;
    for (i = 1; i <= menuBarCount; i++) {
        var menuBarObj = getElmId("DOMenuBar" + i);
        for (j = 0; j < menuBarObj.childNodes.length; j++) {
            if ((menuBarObj.childNodes[j].enabled) && (menuBarObj.childNodes[j].clicked)) {
                menuBarObj.childNodes[j].className = menuBarObj.childNodes[j].itemClassName;
                menuBarObj.childNodes[j].clicked = false;
                hideMenus(menuBarObj.childNodes[j].menu.menuObj);
                break;
            }
        }
        menuBarObj.activated = false;
    }
}


/*
Event handler that handles mouse move event.
*/
function mouseMoveHandler(e) {
    if (draggingObj) {
        draggingObj.style.left = (getX(e) - draggingObj.differenceLeft) + "px";
        draggingObj.style.top = (getY(e) - draggingObj.differenceTop) + "px";
    }
}

/*
Event handler that handles scroll event.
*/
function menuBarScrollHandler() { // Private method
    var i;
    for (i = 1; i <= menuBarCount; i++) {
        var menuBarObj = getElmId("DOMenuBar" + i);
        if ((isIE()) && (menuBarObj.mode == "fixed")) {
            with (menuBarObj.style) {
                left = (menuBarObj.initialLeft + getScrollLeft()) + "px";
                top = (menuBarObj.initialTop + getScrollTop()) + "px";
            }
        }
    }
}

if (typeof(menuBarClassName) == "undefined") {
    var menuBarClassName = "jsdomenubardiv";
    // Public property
}

if (typeof(menuBarItemClassName) == "undefined") {
    var menuBarItemClassName = "jsdomenubaritem";
    // Public property
}

if (typeof(menuBarItemClassNameOver) == "undefined") {
    var menuBarItemClassNameOver = "jsdomenubaritemover";
    // Public property
}

if (typeof(menuBarItemClassNameClick) == "undefined") {
    var menuBarItemClassNameClick = "jsdomenubaritemclick";
    // Public property
}

if (typeof(menuBarDragClassName) == "undefined") {
    var menuBarDragClassName = "jsdomenubardragdiv";
    // Public property
}

if (typeof(menuBarBorderWidth) == "undefined") {
    var menuBarBorderWidth = 2;
    // Public property
}

var menuBarCount = 0;
// Private property
var menuBarItemCount = 0;
// Private property
var draggingObj = null;
