# Note that when compiling with coffeescript, the plugin is wrapped in another
# anonymous function. We do not need to pass in undefined as well, since
# coffeescript uses (void 0) instead.
do ($ = jQuery, window) ->

	# window and document are passed through as local variable rather than global
	# as this (slightly) quickens the resolution process and can be more efficiently
	# minified (especially when both are regularly referenced in your plugin).

	# Create the defaults once
	eventName = 'domchange'
	defaults =
		events:
			attributes: yes
			children: yes
			characterData: yes
		descendents: yes
		recordPriorValues:
			attributes: no
			characterData: no
		attributeFilter: null

	# The plugin constructor
	class DOMChangeEventHandler
		constructor: (@element, options) ->
			@_eventName = eventName
			@_defaults = defaults
			@_options = options
			@options = $.extend {}, defaults, options
			@hook()

		hook: ->
			if @observer? then return

			# firefox and chrome implement this feature under different names
			MutationObserver = window.MutationObserver || window.WebKitMutationObserver

			# this (expensive) object watches for changes
			@observer = new MutationObserver @callback

			# configure the observer with our target element and options
			@observer.observe @element,
				childList:				@options.events.children
				attributes:				@options.events.attributes
				characterData:			@options.events.characterData
				subtree:				@options.descendents
				attributeOldValue:		@options.recordPriorValues.attributes
				characterDataOldValue:	@options.recordPriorValues.characterDataOldValue

		unhook: ->
			return unless @observer?
			@observer.disconnect()
			@observer = null

		callback: (changes) =>
			$(@element).trigger 'domchange', changes

			
	jQuery.event.special[eventName] =
		add: (params) ->
			options = params.data
			id = 'domchange_' + params.guid
			element = @
			# fn = params.handler

			handler = new DOMChangeEventHandler element, options
			$.data element, id, handler

		remove: (params) ->
			id = 'domchange_' + params.guid
			element = @
			# fn = params.handler

			handler = $.data element, id
			handler.unhook()


	# # The plugin constructor
	# class DOMChangePlugin
	# 	constructor: (@element, options) ->
	# 		# jQuery has an extend method which merges the contents of two or
	# 		# more objects, storing the result in the first object. The first object
	# 		# is generally empty as we don't want to alter the default options for
	# 		# future instances of the plugin
	# 		@options = $.extend {}, defaults, options
	# 		@_defaults = defaults
	# 		@_name = pluginName
	# 		@init()

	# 	init: ->
	# 		# firefox and chrome implement this feature under different names
	# 		MutationObserver = window.MutationObserver || window.WebKitMutationObserver

	# 		# this (expensive) object watches for changes
	# 		@observer = new MutationObserver @callback

	# 		# configure the observer with our target element and options
	# 		@observer.observe @element,
	# 			childList:				@options.events.children
	# 			attributes:				@options.events.attributes
	# 			characterData:			@options.events.characterData
	# 			subtree:				@options.descendents
	# 			attributeOldValue:		@options.recordPriorValues.attributes
	# 			characterDataOldValue:	@options.recordPriorValues.characterDataOldValue

	# 	# this is called when a change is made
	# 	callback: (mutations) =>
	# 		$(@element).trigger 'domchange', mutations

	# 	# this method doesn't really work =p
	# 	# unhook: ->
	# 	# 	@observer.disconnect()

	# # A really lightweight plugin wrapper around the constructor,
	# # preventing against multiple instantiations
	# $.fn[pluginName] = (options) ->
	# 	@each ->
	# 		if !$.data(@, 'plugin_#{pluginName}')?
	# 			$.data(@, 'plugin_#{pluginName}', new DOMChangePlugin(@, options))
