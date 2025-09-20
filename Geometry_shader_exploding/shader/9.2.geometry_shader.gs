#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VS_OUT {
    vec2 texCoords;
} gs_in[];
out vec2 TexCoords;

uniform float explosionTime;

vec4 explode(vec4 p, vec3 n) {
    float speed = 6.0;
    // move along normal steadily
    return p + vec4(n * explosionTime * speed, 0.0);
}

vec3 getNormal() {
    vec3 a = vec3(gl_in[1].gl_Position - gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position - gl_in[0].gl_Position);
    return normalize(cross(a, b));
}

void main() {
    vec3 normal = getNormal();
    for (int i = 0; i < 3; ++i) {
        gl_Position = explode(gl_in[i].gl_Position, normal);
        TexCoords   = gs_in[i].texCoords;
        EmitVertex();
    }
    EndPrimitive();
}