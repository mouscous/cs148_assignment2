// default.vert

/*
  This simple GLSL vertex shader does exactly what 
  OpenGL would do -- It transforms the vertex positions
  using the default OpenGL transformation. It also passes
  through the texture coordinate, normal coordinate, and some
  other good stuff so we can use them in the fragment shader.
*/

// This 'varying' vertex output can be read as an input
// by a fragment shader that makes the same declaration.
varying vec3 modelPos;
varying vec3 lightSourcePos;
varying vec3 normal;

void main()
{
    // Render the shape using standard OpenGL position transforms.
    gl_Position = ftransform();

    // we may need this in the fragment shader...
    modelPos = (gl_ModelViewMatrix * gl_Vertex).xyz;

	// send the normal to the fragment shader
	normal = (gl_NormalMatrix * gl_Normal).xyz;

    // pass the light source position to the fragment shader
    lightSourcePos = gl_LightSource[0].position.xyz;
}
