//  get canvas
var c = document.getElementById("myCanvas");

// create 2D context (not WEBGL?)
var context = c.getContext("2d");

// set image width and height
var imgWidth = 600;
var imgHeight = 400;
var imgData = context.createImageData(imgWidth, imgHeight);
var ii;
var jj;

// for each column (x coordinate)
for (jj = 0; jj < imgWidth; jj++) {
    // possibly the
    var qq = jj * 4;

    // for each row from top to bottom? (y coordinate)
    for (ii = 0; ii < imgHeight; ii += 1) {
        // what is this
        var pp = ii * imgWidth * 4 + qq;

        //
        if (jj < 256) {
            imgData.data[pp] = jj;
        } else {
            imgData.data[pp] = 0;
        }
        if (ii < 256) {
            imgData.data[pp + 1] = ii;
        } else {
            imgData.data[pp + 1] = 0;
        }
        if (ii > 255 && jj > 255) {
            imgData.data[pp + 2] = 255;
        } else {
            imgData.data[pp + 2] = 0;
        }
        imgData.data[pp + 3] = 255;
    }
}

// put the image (sized at 600 * 400) at coordinates starting (x, y) == (20, 20)
context.putImageData(imgData, 20, 20);
