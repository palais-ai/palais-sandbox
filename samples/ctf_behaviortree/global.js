function partial(func /*, 0..n args */) {
	var args = Array.prototype.slice.call(arguments, 1);
	return function() {
		var allArguments = args.concat(Array.prototype.slice.call(arguments));
		return func.apply(this, allArguments);
	};
}

// CREDITS: http://stackoverflow.com/questions/4152931/javascript-inheritance-call-super-constructor-or-use-prototype-chain
function extend(base, sub) {
  // Avoid instantiating the base class just to setup inheritance
  // See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/create
  // for a polyfill
  // Also, do a recursive merge of two prototypes, so we don't overwrite 
  // the existing prototype, but still maintain the inheritance chain
  // Thanks to @ccnokes
  var origProto = sub.prototype;
  sub.prototype = Object.create(base.prototype);
  for (var key in origProto)  {
     sub.prototype[key] = origProto[key];
  }
  // Remember the constructor property was set wrong, let's fix it
  sub.prototype.constructor = sub;
  // In ECMAScript5+ (all modern browsers), you can make the constructor property
  // non-enumerable if you define it like this instead
  Object.defineProperty(sub.prototype, 'constructor', { 
    enumerable: false, 
    value: sub 
  });
}

function construct(constructor, args) {
    function F() {
        return constructor.apply(this, args);
    }
    F.prototype = constructor.prototype;
    return new F();
}