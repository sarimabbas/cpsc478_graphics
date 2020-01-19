var shaderProgram = null;
var vertexBuffer = null;
var vertexColorBuffer = null;
var aPositionIndex = -1;
var aVertexColor = -1;

// num: how many triangles to make
const createTrianglePositions = num => {
    // special cases (num is 0-2)
    // prettier-ignore
    const specialCases = [
        [0,      1,      0,
        .707,   .707,   0,
        0,      0,      0],
        [1,      0,      0, 
        .707,   -.707,  0, 
        0,      0,      0]
    ];
    if (num == 0) {
        return new Float32Array([]);
    } else if (num == 1) {
        return new Float32Array(specialCases.slice(0, 1).flat(Infinity));
    } else if (num == 2) {
        return new Float32Array(specialCases.flat(Infinity));
    }

    // general case (num >= 3)
    let vertices = [];
    const theta = (2 * Math.PI) / num;
    for (let i = 0; i < num; i++) {
        // calculate the second coordinate (the first is the origin)
        // the radius/hypotenuse of the triangle is 1 (because webGL canvas is -1 to 1)
        // theta * i because we make the triangle every multiple of theta
        const second_x = 1 * Math.cos(theta * (i + 1));
        const second_y = 1 * Math.sin(theta * (i + 1));

        // if the second or more vertex
        let triangle = [0, 0, 0];
        if (vertices.length > 0) {
            // create all the vertices for this triangle
            // prettier-ignore
            triangle = [
                0, 0, 0, // first coordinate
                second_x, second_y, 0, // second coordinate
                // third coordinate is the previous second coordinate
                vertices[i-1][3], vertices[i-1][4], vertices[i-1][5] 
            ]
            // if no triangle has been added yet, we need to add all the coordinates
        } else {
            // prettier-ignore
            const third_x = 1 * Math.cos(2 * Math.PI * (i + 1));

            // prettier-ignore
            triangle = [
                0, 0, 0, // first coordinate,
                second_x, second_y, 0, // second
                third_x, 0, 0 // third is just the straight line with x coordinate
            ];
        }
        vertices.push(triangle);
    }
    // flatten the vertices array
    console.log(vertices);
    vertices = vertices.flat(Infinity);
    return new Float32Array(vertices);
};

const createTriangleColorsWheel = num => {
    let colors = [];
    for (let i = 0; i < num; i++) {
        if (i % 2 == 0) {
            // all vertices of the triangle should be red
            // prettier-ignore
            colors.push([ 
                1.0, 0.0, 0.0,
                1.0, 0.0, 0.0,
                1.0, 0.0, 0.0,
            ])
        } else {
            // all vertices of the triangle should be blue
            // prettier-ignore
            colors.push([ 
                0.0, 0.0, 1.0,
                0.0, 0.0, 1.0,
                0.0, 0.0, 1.0,
            ])
        }
    }
    colors = colors.flat(Infinity);
    return new Float32Array(colors);
};

/// Initialize the data buffer to pass to the rendering pipeline
/// the geometry and its attributes.
function initBuffers(gl, num) {
    // prettier-ignore
    //     triangleVertices = new Float32Array([
    //     0,      1,      0,
    //     .707,   .707,   0,
    //     0,      0,      0,
    //     1,      0,      0,
    //     .707,   -.707,  0,
    //     0,      0,      0
    // ]);

    triangleVertices = createTrianglePositions(num);

    // prettier-ignore
    // var triangleVerticesColor = new Float32Array([
    // 1.0, 0.0, 0.0,  // color of the 1st vertex (red)
    // 0.0, 1.0, 0.0,  // color of the 2nd vertex (green)
    // 0.0, 0.0, 1.0 , // color of the 3rd vertex (blue)
    // 1.0, 1.0, 0.0,  // color of the 4th vertex (yellow)
    // 1.0, 1.0, 1.0,
    // 0.0, 1.0, 1.0
    // ]);

    var triangleVerticesColor = createTriangleColorsWheel(num)

    vertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vertexColorBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexColorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVerticesColor, gl.STATIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
}

/// Define and compile a very simple shader.
function initShaders(gl) {
    // prettier-ignore
    var vertexShaderSource = "\
  attribute vec3 a_position;                  \n\
  attribute vec3 a_color;                     \n\
  varying vec3 vertexcolor;                   \n\
  void main(void)                             \n\
  {                                           \n\
    vertexcolor = a_color;                  \n\
    gl_Position = vec4(a_position, 1.0);    \n\
  }                                           \n\
  ";

    // prettier-ignore
    var fragmentShaderSource = "\
  precision highp float;                      \n\
  varying vec3 vertexcolor;                   \n\
  void main(void)                             \n\
  {                                           \n\
    gl_FragColor = vec4(vertexcolor, 1.0);  \n\
  }                                           \n\
  ";

    // create the vertex shader
    var vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexShaderSource);
    gl.compileShader(vertexShader);

    // create the fragment shader
    var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentShaderSource);
    gl.compileShader(fragmentShader);

    // Create the shader program
    shaderProgram = gl.createProgram();
    gl.attachShader(shaderProgram, vertexShader);
    gl.attachShader(shaderProgram, fragmentShader);
    gl.linkProgram(shaderProgram);

    // If creating the shader program failed, we show compilation and linking errors.
    if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
        alert("Unable to initialize the shader program.");
        var str = "";
        str += "VS:\n" + gl.getShaderInfoLog(vertexShader) + "\n\n";
        str += "FS:\n" + gl.getShaderInfoLog(fragmentShader) + "\n\n";
        str += "PROG:\n" + gl.getProgramInfoLog(shaderProgram);
        alert(str);
    }
}

/// Draw the given triangle interpolating vertices color.
function renderTriangle(gl, num) {
    // Clear the framebuffer of the rendering context
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // enable the current shader program
    gl.useProgram(shaderProgram);

    // connect the buffer containing the vertices of the triangle with the position attribute
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
    aPositionIndex = gl.getAttribLocation(shaderProgram, "a_position");
    gl.enableVertexAttribArray(aPositionIndex);
    gl.vertexAttribPointer(aPositionIndex, 3, gl.FLOAT, false, 0, 0);

    // connect the buffer containing the color of each vertex with the color attribute
    gl.bindBuffer(gl.ARRAY_BUFFER, vertexColorBuffer);
    aVertexColor = gl.getAttribLocation(shaderProgram, "a_color");
    gl.enableVertexAttribArray(aVertexColor);
    gl.vertexAttribPointer(aVertexColor, 3, gl.FLOAT, false, 0, 0);

    // start to draw (!)
    // gl.drawArrays(gl.TRIANGLES, 0, 3);
    gl.drawArrays(gl.TRIANGLES, 0, num * 3);

    // disable the current shading program
    gl.useProgram(null);
}
