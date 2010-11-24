function MenuBar(headerId, itemsId) {

  this.menuBarObj = this;

  var menuBarElm = document.getElementById(headerId);
  menuBarElm.className = "menubardiv";

  var itemsElm = document.getElementById(itemsId);
  itemsElm.className = "menudiv";
  itemsElm.style.visibility = "hidden";

  menuBarElm.onmouseover = function(e) {
    itemsElm.style.visibility = "visible";
    menuBarElm.className = "menubardivover";
  };

  itemsElm.onmouseover = menuBarElm.onmouseover;

  menuBarElm.onmouseout = function(e) {
    itemsElm.style.visibility = "hidden";
    menuBarElm.className = "menubardiv";
    return false;
  };

  itemsElm.onmouseout = menuBarElm.onmouseout;

  this.addMenuItem = function(menuItemObj) {

    var itemElm = document.getElementById(menuItemObj.itemId);

    itemElm.onmouseover = function(e) {
      itemsElm.onmouseover(e);
      itemElm.className = menuItemObj.itemClassNameOver;
    };

    itemElm.onmouseout = function(e) {
      itemElm.className = menuItemObj.itemClassName;
    };

  };
}

function MenuItem(itemId) {
  this.itemId = itemId;

  this.itemClassNameOver="menuitemover";
  this.itemClassName="menuitem";
}

