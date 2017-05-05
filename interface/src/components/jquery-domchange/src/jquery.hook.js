(function($, window, document) {
  $.hook = function(fns) {
    if (typeof fns === 'string') {
      fns = fns.split(' ');
    }
    $.makeArray(fns);
    return jQuery.each(fns, function(i, method) {
      var old;
      old = $.fn[method];
      if (old && !old.__hookold) {
        $.fn[method] = function() {
          var result;
          this.triggerHandler('onbefore' + method);
          this.triggerHandler('on' + method);
          result = old.apply(this, arguments);
          this.triggerHandler('onafter' + method);
          return result;
        };
        return $.fn[method].__hookold = old;
      }
    });
  };
  return $.unhook = function(fns) {
    if (typeof fns === 'string') {
      fns = fns.split(' ');
    }
    $.makeArray(fns);
    return jQuery.each(fns, function(i, method) {
      var current;
      current = $.fn[method];
      if (current && current.__hookold) {
        return $.fn[method] = current.__hookold;
      }
    });
  };
})(jQuery, window, document);
