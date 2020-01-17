const whichColor = p => {
    if (Math.random() <= p) {
        return "one";
    } else {
        return "two";
    }
};

const iterateByRow = (
    imgWidth,
    imgHeight,
    probability,
    oneColor,
    twoColor,
    imgData
) => {
    // go through all the subpixels/indices, 4 at a time (i.e. 1 pixel at a time)
    const size = imgWidth * imgHeight * 4;
    for (let subpixel = 0; subpixel < size; subpixel += 4) {
        const getColor = whichColor(probability);
        imgData.data[subpixel] = getColor === "one" ? oneColor[0] : twoColor[0];
        imgData.data[subpixel + 1] =
            getColor === "one" ? oneColor[1] : twoColor[1];
        imgData.data[subpixel + 2] =
            getColor === "one" ? oneColor[2] : twoColor[2];
        imgData.data[subpixel + 3] = 255;
    }
};

const iterateByColumn = (
    imgWidth,
    imgHeight,
    probability,
    oneColor,
    twoColor,
    imgData
) => {
    // for each column up to the width
    for (jj = 0; jj < imgWidth; jj++) {
        // 4 subpixels in a pixel
        var qq = jj * 4; // qq == 4

        // for each row up to the height
        for (ii = 0; ii < imgHeight; ii += 1) {
            // get index of the next pixel in the column (i.e. skip width)
            var pp = ii * imgWidth * 4 + qq;

            const getColor = whichColor(probability);

            imgData.data[pp] = getColor === "one" ? oneColor[0] : twoColor[0];
            imgData.data[pp + 1] =
                getColor === "one" ? oneColor[1] : twoColor[1];
            imgData.data[pp + 2] =
                getColor === "one" ? oneColor[2] : twoColor[2];
            imgData.data[pp + 3] = 255;
        }
    }
};

function doStuff() {
    // get ref to canvas
    var c = document.getElementById("myCanvas");

    // create 2D context
    var context = c.getContext("2d");

    // set image attributes
    var imgWidth = 500;
    var imgHeight = 500;

    // get probability from form
    var probability = document.getElementById("probability").value;

    // set the first color values
    var onecolor = [255, 255, 255]; // this is the default
    onecolor[0] = document.getElementById("oneRed").value;
    onecolor[1] = document.getElementById("oneGreen").value;
    onecolor[2] = document.getElementById("oneBlue").value;

    // set the second color values
    var twocolor = [255, 255, 255];
    twocolor[0] = document.getElementById("twoRed").value;
    twocolor[1] = document.getElementById("twoGreen").value;
    twocolor[2] = document.getElementById("twoBlue").value;

    // create image
    var imgData = context.createImageData(imgWidth, imgHeight);

    // modify this section to make checkerboard
    // default image is one gray square

    iterateByColumn(
        imgWidth,
        imgHeight,
        probability,
        onecolor,
        twocolor,
        imgData
    );

    // iterateByRow(imgWidth, imgHeight, probability, onecolor, twocolor, imgData);

    context.putImageData(imgData, 20, 20);
}
