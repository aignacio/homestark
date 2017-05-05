/*
 *  jQuery DOM Change - v0.0.1
 *  Simple jQuery plugin to add support for DOM change events.
 *  http://jqueryboilerplate.com
 *
 *  Made by Colby Rogness
 *  Under GNU GPL 2.0 License
 */
(function() {
  var __bind = function(fn, me){ return function(){ return fn.apply(me, arguments); }; };

  (function($, window) {
    var DOMChangeEventHandler, defaults, eventName;
    eventName = 'domchange';
    defaults = {
      events: {
        attributes: true,
        children: true,
        characterData: true
      },
      descendents: true,
      recordPriorValues: {
        attributes: false,
        characterData: false
      },
      attributeFilter: null
    };
    DOMChangeEventHandler = (function() {
      function DOMChangeEventHandler(element, options) {
        this.element = element;
        this.callback = __bind(this.callback, this);
        this._eventName = eventName;
        this._defaults = defaults;
        this._options = options;
        this.options = $.extend({}, defaults, options);
        this.hook();
      }

      DOMChangeEventHandler.prototype.hook = function() {
        var MutationObserver;
        if (this.observer != null) {
          return;
        }
        MutationObserver = window.MutationObserver || window.WebKitMutationObserver;
        this.observer = new MutationObserver(this.callback);
        return this.observer.observe(this.element, {
          childList: this.options.events.children,
          attributes: this.options.events.attributes,
          characterData: this.options.events.characterData,
          subtree: this.options.descendents,
          attributeOldValue: this.options.recordPriorValues.attributes,
          characterDataOldValue: this.options.recordPriorValues.characterDataOldValue
        });
      };

      DOMChangeEventHandler.prototype.unhook = function() {
        if (this.observer == null) {
          return;
        }
        this.observer.disconnect();
        return this.observer = null;
      };

      DOMChangeEventHandler.prototype.callback = function(changes) {
        return $(this.element).trigger('domchange', changes);
      };

      return DOMChangeEventHandler;

    })();
    return jQuery.event.special[eventName] = {
      add: function(params) {
        var element, handler, id, options;
        options = params.data;
        id = 'domchange_' + params.guid;
        element = this;
        handler = new DOMChangeEventHandler(element, options);
        return $.data(element, id, handler);
      },
      remove: function(params) {
        var element, handler, id;
        id = 'domchange_' + params.guid;
        element = this;
        handler = $.data(element, id);
        return handler.unhook();
      }
    };
  })(jQuery, window);

}).call(this);
