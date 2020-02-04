var shaderProgram  = null;
var vertexBuffer = null;
var vertexColorBuffer = null;
var aPositionIndex = -1;
var aVertexColor = -1;
var totalVertices = -1;

///// Initialize the data buffer to pass to the rendering pipeline
///// the geometry and its attributes.
function initBuffers(gl) 
{
  totalVertices = 4;
  triangleVertices = new Float32Array([
    -1,-1,0,
    -1,1,0,
    1,-1,0,
    1,1,0
  ]);

  vertexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
  gl.bindBuffer(gl.ARRAY_BUFFER, null);
}

///// Define and compile a very simple shader.
function initShaders(gl) 
{
  var vertexShaderSource = document.getElementById('vertex-shader').innerText;
  var fragmentShaderSource = document.getElementById('fragment-shader').innerText;
 
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
  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) 
  {
    alert("Unable to initialize the shader program.");
    var str = "";
    str += "VS:\n" + gl.getShaderInfoLog(vertexShader) + "\n\n";
    str += "FS:\n" + gl.getShaderInfoLog(fragmentShader) + "\n\n";
    str += "PROG:\n" + gl.getProgramInfoLog(shaderProgram);
    alert(str);
  }
}

///// Draw the given triangle interpolating vertices color.
function renderTriangle(gl) 
{
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
 
  // start to draw (!)
  gl.drawArrays(gl.TRIANGLE_STRIP, 0, totalVertices);

  // disable the current shading program
  gl.useProgram(null);
}
