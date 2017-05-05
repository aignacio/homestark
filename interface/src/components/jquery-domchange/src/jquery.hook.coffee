# Below follows my CoffeeScript port of Aaron Heckmann's jQuery Hook plugin,
# which is dual-licensed under the MIT and GPL licenses:
# http://www.opensource.org/licenses/mit-license.php
# http://www.gnu.org/licenses/gpl.html
# See the project page on GitHub for more details:
# https://github.com/aheckmann/jquery.hook

$.hook = (fns) ->
	fns = fns.split ' ' if typeof fns is 'string'
	$.makeArray fns
	# if typeof fns === 'string'
	# 	fns = fns.split ' '
	# else
	# 	fns = $.makeArray fns

	jQuery.each fns, (i, method) ->
		old = $.fn[method]
		if old and !old.__hookold
			$.fn[method] = ->
				@triggerHandler 'onbefore' + method
				@triggerHandler 'on' + method
				result = old.apply @, arguments
				@triggerHandler 'onafter' + method
				result

			$.fn[method].__hookold = old

$.unhook = (fns) ->
	fns = fns.split ' ' if typeof fns is 'string'
	$.makeArray fns
	# if typeof fns === 'string'
	# 	fns = fns.split ' '
	# else
	# 	fns = $.makeArray fns

	jQuery.each fns, (i, method) ->
		current = $.fn[method]
		$.fn[method] = current.__hookold if current and current.__hookold
