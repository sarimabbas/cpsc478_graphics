// j is the column
// i is the row
// e.g. j == 0 (first column), i == 1 (second row)
// then for a 100 width image,
// index = (1 * 100 * 4) + (0 * 4) == 400
const getPixelAtCoord = (j, i, inputData) => {
    // prettier-ignore
    // index = (the current horizontal "line") + (horizontal offset)
    const index = (i * inputData.width * 4) + (j * 4);
    const size = inputData.width * inputData.height * 4;
    // return correct pixel
    if (index >= 0 && index < size) {
        return [
            inputData.data[index],
            inputData.data[index + 1],
            inputData.data[index + 2],
            inputData.data[index + 3]
        ];
        // return black pixel
    } else {
        return [0, 0, 0, 255];
    }
};

// i: column
// j: row
const getSumOfSurroundPixels = (j, i, inputData) => {
    let red = 0;
    let blue = 0;
    let green = 0;
    let alpha = 255;
    for (let jv = j - 2; jv < j + 3; jv++) {
        for (let iv = i - 2; iv < i + 3; iv++) {
            const pixel = getPixelAtCoord(jv, iv, inputData);
            red += pixel[0];
            blue += pixel[1];
            green += pixel[2];
            alpha += pixel[3];
        }
    }
    return [red, blue, green, alpha];
};

const applyBlurToPixel = (j, i, inputData) => {
    // get RGBA sum of 5x5 area centered on pixel
    const rgba_sum = getSumOfSurroundPixels(j, i, inputData);

    // take an average and apply
    return [
        rgba_sum[0] / 25,
        rgba_sum[1] / 25,
        rgba_sum[2] / 25,
        rgba_sum[3] / 25
    ];
};

// adapted from http://stackoverflow.com/questions/3914203/javascript-filter-image-color
function createCanvas(image) {
    // create a new canvas element
    var myCanvas = document.createElement("canvas");
    var myCanvasContext = myCanvas.getContext("2d");

    var imgWidth = image.width;
    var imgHeight = image.height;

    // set the width and height to show two copies of the image
    myCanvas.width = 2 * imgWidth + 10;
    myCanvas.height = imgHeight;

    // draw the image
    myCanvasContext.drawImage(image, 0, 0);

    // get all the input and output image data into arrays
    var imageData = myCanvasContext.getImageData(0, 0, imgWidth, imgHeight);
    var imoutData = myCanvasContext.getImageData(0, 0, imgWidth, imgHeight);

    // go through it all
    for (j = 0; j < imageData.width; j++) {
        for (i = 0; i < imageData.height; i++) {
            // find index of current pixel
            // prettier-ignore
            const index = (i * imageData.width * 4) + (j * 4);
            // find blurred pixel values
            const blurredPixel = applyBlurToPixel(j, i, imageData);
            // apply blur
            imoutData.data[index] = blurredPixel[0];
            imoutData.data[index + 1] = blurredPixel[1];
            imoutData.data[index + 2] = blurredPixel[2];
            imoutData.data[index + 3] = blurredPixel[3];
        }
    }

    // put the image data back into the canvas
    myCanvasContext.putImageData(
        imoutData,
        imageData.width + 10, // start ten pixels to the right
        0, // start at x = 0
        0, // start at y = 0
        0, // what the F is this
        imageData.width, // same sizes
        imageData.height
    );

    // append it to the body
    document.body.appendChild(myCanvas);
}

function loadImage() {
    var img = new Image();
    img.onload = function() {
        createCanvas(img);
    };
    img.src = document.getElementById("imagefilename").files[0].name;
}
