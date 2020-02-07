const midpoint = (a, b) => {
    return [
        (a[0] + b[0]) / parseFloat(2),
        (a[1] + b[1]) / parseFloat(2),
        (a[2] + b[2]) / parseFloat(2)
    ];
};

const getPointFromIndex = (points, index) => {
    return points.slice(index * 3, index * 3 + 3);
};

const projectVertexToSphere = (point, radiusSphere) => {
    // treat the point as a vector, and find its length (with respect to the origin)
    const length = Math.sqrt(point[0] ** 2 + point[1] ** 2 + point[2] ** 2);
    const scale = radiusSphere / length;

    // return a new point, which is the scaled version
    point[0] = point[0] * scale;
    point[1] = point[1] * scale;
    point[2] = point[2] * scale;
    return point;
};

const split = (indicesInitial, pointsInitial) => {
    // do the face splitting here
    let indicesFinal = [];
    let pointsFinal = [];
    for (var i = 0; i < indicesInitial.length; i += 3) {
        const face = [
            indicesInitial[i],
            indicesInitial[i + 1],
            indicesInitial[i + 2]
        ];
        // console.log("Face: ", face);
        // get the 3 points that make up the face
        const points = [
            getPointFromIndex(pointsInitial, face[0]), // 0
            getPointFromIndex(pointsInitial, face[1]), // 1
            getPointFromIndex(pointsInitial, face[2]) // 2
        ];
        // console.log("Points: ", points);

        // make 3 more points by splitting the triangle
        const newPoints = [
            midpoint(points[0], points[1]),
            midpoint(points[1], points[2]),
            midpoint(points[2], points[0])
        ];
        // console.log("Midpoints: ", newPoints);

        // these points will take on these identies (order matters)
        pointsFinal.push([...points, ...newPoints]);
        pointsFinal = pointsFinal.flat(Infinity);
        // console.log("All points: ", pointsFinal);

        // create a new indices/faces array referencing these points (order
        // doesn't really matter)
        // prettier-ignore

        const newFaces = [
            (i*2)+2, (i*2)+4, (i*2)+5, // top
            (i*2)+5, (i*2)+3, (i*2)+0, // left
            (i*2)+5, (i*2)+4, (i*2)+3, // middle
            (i*2)+4, (i*2)+1, (i*2)+3  // right
        ];
        // console.log("Faces: ", newFaces);

        indicesFinal = indicesFinal.concat(newFaces);
        // console.log("All faces: ", indicesFinal);
        // now, we need to make the new faces array

        // console.log("END FOR ONE TRIANGLE");
    }
    return {
        indices: indicesFinal,
        points: pointsFinal
    };
};

function Octahedron(splits) {
    this.name = "octahedron";

    // vertices definition
    ////////////////////////////////////////////////////////////

    // these are x, y, z coordinates
    const extremePoint = Math.sqrt(2.0);
    // prettier-ignore
    let points = [
        // the four points on the base
        extremePoint, 0.0, 0.0, 
        0.0, 0.0, -extremePoint,
        -extremePoint, 0.0, 0.0, 
        0.0, 0.0, extremePoint, 
        // the top point
        0.0, extremePoint, 0.0,
        // the bottom point
        0.0, -extremePoint, 0.0,
    ];

    // prettier-ignore
    // 8 faces, 1 row for each face/triangle
    let indices = [
        1, 0, 5, // front, bottom
        1, 0, 4, // front, top
        2, 3, 4, // back, top
        2, 3, 5, // back, bottom
        2, 1, 4, // left, top
        2, 1, 5, // left, bottom
        3, 0, 4, // right, top
        3, 0, 5, // right, bottom
    ]

    for (var i = 0; i < splits; i++) {
        const splitResult = split(indices, points);
        points = splitResult.points;
        indices = splitResult.indices;
        console.log("Vertices: ", points);
        console.log("Indices: ", indices);
    }

    // do the projection
    for (var i = 0; i < points.length; i += 3) {
        const p = [points[i], points[i + 1], points[i + 2]];
        const projected = projectVertexToSphere(p, extremePoint);
        points[i] = projected[0];
        points[i + 1] = projected[1];
        points[i + 2] = projected[2];
    }

    // prettier-ignore
    // x, y, z
    this.vertices = new Float32Array(points);

    // prettier-ignore
    this.triangleIndices = new Uint16Array(indices);

    this.numVertices = this.vertices.length / 3;
    this.numTriangles = this.triangleIndices.length / 3;
}
