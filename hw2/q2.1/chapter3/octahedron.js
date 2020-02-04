/// CUBE DEFINTION
///
/// Cube is defined to be centered at the origin of the coordinate reference system.
/// Cube size is assumed to be 2.0 x 2.0 x 2.0 .

// a cube has 8 vertices (which are in a row obviously)
// it
function Octahedron() {
    this.name = "octahedron";

    // vertices definition
    ////////////////////////////////////////////////////////////

    // prettier-ignore
    // this.vertices = new Float32Array([
    // 	-1.0, -1.0,  1.0,
    // 	 1.0, -1.0,  1.0,
    // 	-1.0,  1.0,  1.0,
    // 	 1.0,  1.0,  1.0,
    // 	-1.0, -1.0, -1.0,
    // 	 1.0, -1.0, -1.0,
    // 	-1.0,  1.0, -1.0,
    // 	 1.0,  1.0, -1.0
    // ]);

    const extremePoint = Math.sqrt(2.0);

    // prettier-ignore
    // x, y, z
    this.vertices = new Float32Array([
        // the four vertices on the base
        extremePoint, 0.0, 0.0, 
        0.0, 0.0, -extremePoint,
        -extremePoint, 0.0, 0.0, 
        0.0, 0.0, extremePoint, 
        // the top point
        0.0, extremePoint, 0.0,
        // the bottom point
        0.0, -extremePoint, 0.0
    ]);

    // triangles definition
    ////////////////////////////////////////////////////////////

    // prettier-ignore
    // 8 faces
    this.triangleIndices = new Uint16Array([
        1, 0, 4, // front, top
        1, 0, 5, // front, bottom
        2, 3, 4, // back, top
        2, 3, 5, // back, bottom
        2, 1, 4, // left, top
        2, 1, 5, // left, bottom
        3, 0, 4, // right, top
        3, 0, 5, // right, bottom
	]);

    this.numVertices = this.vertices.length / 3;
    this.numTriangles = this.triangleIndices.length / 3;
}
