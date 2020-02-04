attribute vec3 a_position;
attribute vec3 a_color;
varying vec3 vertexcolor;
void main(void)
{
  vertexcolor = a_color;
  gl_Position = vec4(a_position, 1.0);
}
