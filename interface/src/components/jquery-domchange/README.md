# jquery-domchange

jquery-domchange is a simple jQuery plugin to add support for DOM change events using jQuery's built-in special event system. There's no need for anything fancy--just start listening to the `domchange` event.

```js
$("#observable").on("domchange", function(event, changes) {
    // do something with the changes
});
```

### Usage

You can simply listen for the `domchange` event as you would with any other event:

```js
$("#observable").on("domchange", function(event, changes) {
    // something happened to the DOM!    
}
```

The event handler is passed (along with the usual `event` details) a list of DOM changes, each of which is a [`MutationRecord`](https://dom.spec.whatwg.org/#mutationrecord). Each `MutationRecord` contains details about the specific change to the DOM, including the type of change, the affected element, etc. You can find out exactly what it contains by [looking at its API](https://dom.spec.whatwg.org/#mutationrecord).

Although the plugin's defaults suffice for most scenarios, you may want to specify some configuration options to help improve performance. When adding your event listener, you can pass in an object to configure the kinds of changes you want to listen to:

```js
$("#observable").on("domchange",
    {
        descendents: false,
        recordPriorValues: {
            attributes: true
        }
    },
    function() {
        // something happened to the DOM!    
    }
);
```

### Quick Option Reference

- **events**
    + **attributes** (*true*): listen to attribute change events
    + **children** (*true*): listen to childList change events
    + **characterData** (*true*): listen to characterData change events
- **descendents** (*true*): watch for changes to the element's children as well as the element
- **recordPriorValues**
    + **attributes** (*false*): record the old value when the element's attribute is changed; if present, attribute events will always be recorded
    + **characterData** (*false*): record the old value when the element's character data is changed; if present, characterData events will always be recorded
- **attributeFilter** (*null*): listen only to attributes whitelisted here; set to a list (array) of attribute names; if present, attribute events will always be recorded

### Options

#### Change Types

There are several different types of changes that jquery-domchange can listen for:

- attribute (html attributes like `href`)
- children (nodes added and removed)
- character data (inner content)

By default, the plugin will listen for all three. However, if you only need to watch for specific types of changes, you can turn off the ones you don't need by disabling them when configuring the plugin:

```js
$("#observable").on("domchange",
    {
        events: {
            attributes: true,
            children: false,
            characterData: true
        }
    },
    function() {
        // something happened to the DOM!    
    }
);
```

If you're listening to attribute changes, you can filter which attributes you care about by whitelisting them:

```js
$("#observable").on("domchange",
    {
        attributeFilter: ['href']
    },
    function() {
        // something happened to the DOM!    
    }
);
```

(Note: if you set an `attributeFilter`, the browser will assume you want to listen to attribute events regardless of what other options you set, even in `events`.)

#### Listening to descendents

You can also choose whether or not you want to listen to changes to the element's children as well. The plugin enables that functionality by default; however, you can disable it to save performance:

```js
$("#observable").on("domchange",
    {
        descendents: false
    },
    function() {
        // something happened to the DOM!    
    }
);
```

#### Recording prior values

When an element's attributes or character data changes, you can specically request the old value by configuring the plugin to remember it:

```js
$("#observable").on("domchange",
    {
        recordPriorValues: {
            attributes: true,
            characterData: true
        }
    },
    function(event, changes) {
        console.log(changes[0].oldValue);  
    }
);
```

### About

#### Credits

jquery-domchange was written in 2015 by Colby Rogness and is licensed under the GNU GPL 2 (provided with this software). The initial code was generated using [jQuery Boilerplate](http://jqueryboilerplate.com/), which is licensed under the [MIT License](http://zenorocha.mit-license.org/). [This question](http://stackoverflow.com/questions/2844565/is-there-a-jquery-dom-change-listener/11546242#11546242) on StackOverflow as well as [Ben Alman's overview of jQuery's special events system](http://benalman.com/news/2010/03/jquery-special-events/) proved very helpful in understanding the concepts that this plugin uses.

#### Behind the scenes

jquery-domchange uses jQuery's built-in special events system to handle `domchange` events.

When you bind to the `domchange` event, the plugin creates a handler object which is stored in the [jQuery data store](http://api.jquery.com/jquery.data/). The handler uses the `MutationObserver` feature native to Firefox and Chrome to fire `domchange` events when the DOM is modified.

If you pass configuration options when you bind your listener, then the handler object will map your options to the `MutationObserver` options.

Because watching for DOM changes is a resource-intensive process, jquery-domchange supports removing listeners as well (e.g. using `.off('domready')`). When you remove `domready` listeners, jquery-domchange retrives the handler that it created when you registered your listener and uses it to remove the `MutationObserver` to prevent memory leaks and improve performance.
