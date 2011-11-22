(function($){
	$.fn.hoverIntent = function(cfg) {

		// cX, cY = current X and Y position of mouse, updated by mousemove event
		// pX, pY = previous X and Y position of mouse, set by mouseover and polling interval
		var cX, cY, pX, pY;

		// A private function for getting mouse position
		var track = function(ev) {
			cX = ev.pageX;
			cY = ev.pageY;
		};

		// A private function for comparing current and previous mouse position
		var compare = function(ev,ob) {
			ob.hoverIntent_t = clearTimeout(ob.hoverIntent_t);
			// compare mouse positions to see if they've crossed the threshold
			if ( ( Math.abs(pX-cX) + Math.abs(pY-cY) ) < cfg.sensitivity ) {
				$(ob).unbind("mousemove",track);
				// set hoverIntent state to true (so mouseOut can be called)
				ob.hoverIntent_s = 1;
				return cfg.over.apply(ob,[ev]);
			} else {
				// set previous coordinates for next time
				pX = cX; pY = cY;
				// use self-calling timeout, guarantees intervals are spaced out properly (avoids JavaScript timer bugs)
				ob.hoverIntent_t = setTimeout( function(){compare(ev, ob);} , cfg.interval );
			}
		};

		// A private function for delaying the mouseOut function
		var delay = function(ev,ob) {
			ob.hoverIntent_t = clearTimeout(ob.hoverIntent_t);
			ob.hoverIntent_s = 0;
			return cfg.out.apply(ob,[ev]);
		};

    var handleMouseOver = function(e) {
      var p = e.fromElement || e.relatedTarget;
			while ( p && p != this ) { try { p = p.parentNode; } catch(e) { p = this; } }
			if ( p == this ) { return false; }

      var ev = jQuery.extend({},e);
			var ob = this;
      if (ob.hoverIntent_t) { ob.hoverIntent_t = clearTimeout(ob.hoverIntent_t); }

      pX = ev.pageX; pY = ev.pageY;
      // update "current" X and Y position based on mousemove
      $(ob).bind("mousemove",track);
      // start polling interval (self-calling timeout) to compare mouse coordinates over time
      if (ob.hoverIntent_s != 1) { ob.hoverIntent_t = setTimeout( function(){compare(ev,ob);} , cfg.interval );}
    };

    var handleMouseOut = function(e) {
      var p = e.toElement || e.relatedTarget;
			while ( p && p != this ) { try { p = p.parentNode; } catch(e) { p = this; } }
			if ( p == this ) { return false; }

      var ev = jQuery.extend({},e);
			var ob = this;

			if (ob.hoverIntent_t) { ob.hoverIntent_t = clearTimeout(ob.hoverIntent_t); }

      // unbind expensive mousemove event
      $(ob).unbind("mousemove",track);
      // if hoverIntent state is true, then call the mouseOut function after the specified delay
      if (ob.hoverIntent_s == 1) { ob.hoverIntent_t = setTimeout( function(){delay(ev,ob);} , cfg.timeout );}
    };

		// bind the function to the two event listeners
		return this.mouseover(handleMouseOver).mouseout(handleMouseOut);
	};

})(jQuery);

function Menu(el) {

  var mainMenuConfig = {
    sensitivity: 7, // number = sensitivity threshold (must be 1 or higher)
    interval: 10,  // number = milliseconds for onMouseOver polling interval
    timeout: 10,   // number = milliseconds delay before onMouseOut
    over : doOpenMainMenu,
    out : doClose
  };

  function doOpenMainMenu() {
    var v = $('ul:first',this);
    v.css('position', 'absolute');
    v.css("left", $(this).position().left);
    v.css("top", $(this).position().top + $(this).height());
    v.css('visibility', 'visible');
  }

  function doClose() {
    $('ul:first',this).css('visibility', 'hidden');
  }

  el.hoverIntent(mainMenuConfig);

  function doOpenSubMenu() {
    var v = $('ul:first',this)
    v.css('position', 'absolute');
    v.css("left", $(this).position().left + $(this).width());
    v.css("top", $(this).position().top);
    v.css('visibility', 'visible');
  }

  var subMenuConfig = {
    sensitivity: 7, // number = sensitivity threshold (must be 1 or higher)
    interval: 10,  // number = milliseconds for onMouseOver polling interval
    timeout: 200,   // number = milliseconds delay before onMouseOut
    over : doOpenSubMenu,
    out : doClose
  };

  el.find("li").hoverIntent(subMenuConfig);
  el.find("ul li:has(ul)").find("a:first").append(" &raquo; ");

  var uls = el.find("ul");
  $.each(uls, function(index, value){
    var lis = $(value).children("li");
    var maxWidth = 0;
    $.each(lis, function(idx, val) {
      $(val).css("border-style", "solid");
      if (idx == 0)
        $(val).css("border-width",  "1px 1px 0px 1px");
      else if (idx == lis.length -1)
        $(val).css("border-width",  "0px 1px 1px 1px");
      else
        $(val).css("border-width",  "0px 1px 0px 1px");

      if ($(val).hasClass("menubardelimiter"))
        $(val).css("border-bottom-width", "1px");

      if ($(val).width() > maxWidth)
        maxWidth = $(val).width();
    });
    $(value).css("width", maxWidth + 10 + "px");
  });
}