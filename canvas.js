// Simple canvas functions


// Get one based on the quality
function canvasSetResizeHighQuality(object,quality) {
	var ctx=object.getContext('2d');	
    
	if (typeof(ctx.imageSmoothingEnabled)!="undefined") ctx.imageSmoothingEnabled = quality; else
	if (typeof(ctx.mozImageSmoothingEnabled)!="undefined") ctx.mozImageSmoothingEnabled = quality; else
	if (typeof(ctx.webkitImageSmoothingEnabled)!="undefined") ctx.webkitImageSmoothingEnabled = quality;
    
    return ctx;
}


// Get the canvas
function getCanvas(object) {
    return canvasSetResizeHighQuality(object,false);
}





// Taken from http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
function dist2(v, w) { return Math.pow(v.x - w.x,2) + Math.pow(v.y - w.y,2) }
function distToSegmentSquared(p, v, w) {
  var l2 = dist2(v, w);
  if (l2 == 0) return dist2(p, v);
  var t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
  t = Math.max(0, Math.min(1, t));
  return dist2(p, { x: v.x + t * (w.x - v.x),
                    y: v.y + t * (w.y - v.y) });
}
function distToSegment(p, v, w) { return Math.sqrt(distToSegmentSquared(p, v, w)); }



// Creates a blank hidden offscreen canvas
function createHiddenCanvas(width, height) {
	var sourceCanvas = document.createElement("canvas");
	sourceCanvas.width = width;
    sourceCanvas.height = height;
	sourceCanvas.style.display="none";	
	return {canvas:getCanvas(sourceCanvas), container:sourceCanvas};
}

// http://paulirish.com/2011/requestanimationframe-for-smart-animating/
// http://my.opera.com/emoller/blog/2011/12/20/requestanimationframe-for-smart-er-animating
 
// requestAnimationFrame polyfill by Erik Möller
// fixes from Paul Irish and Tino Zijdel
 
(function() {
    var lastTime = 0;
    var vendors = ['ms', 'moz', 'webkit', 'o'];
    for(var x = 0; x < vendors.length && !window.requestAnimationFrame; ++x) {
        window.requestAnimationFrame = window[vendors[x]+'RequestAnimationFrame'];
        window.cancelAnimationFrame = window[vendors[x]+'CancelAnimationFrame']
                                   || window[vendors[x]+'CancelRequestAnimationFrame'];
    }
 
    if (!window.requestAnimationFrame)
        window.requestAnimationFrame = function(callback, element) {
            var currTime = new Date().getTime();
            var timeToCall = Math.max(0, 16 - (currTime - lastTime));
            var id = window.setTimeout(function() { callback(currTime + timeToCall); },
              timeToCall);
            lastTime = currTime + timeToCall;
            return id;
        };
 
    if (!window.cancelAnimationFrame)
        window.cancelAnimationFrame = function(id) {
            clearTimeout(id);
        };
}());