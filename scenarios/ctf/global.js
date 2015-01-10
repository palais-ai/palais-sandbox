function partial(func /*, 0..n args */) {
	var args = Array.prototype.slice.call(arguments, 1);
	return function() {
		var allArguments = args.concat(Array.prototype.slice.call(arguments));
		return func.apply(this, allArguments);
	};
}

function extend(dst, src) {
	for (var key in src) {
	  dst[key] = src[key];
	}
	return dst;
}