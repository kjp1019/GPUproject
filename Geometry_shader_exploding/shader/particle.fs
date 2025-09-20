#version 330 core
out vec4 FragColor;

void main()
{
    float d = length(gl_PointCoord - vec2(0.5));
    // 0.0~0.4 ������ �ε巴��, �� ���Ŀ� ���� ����
    float alpha = 1.0 - smoothstep(0.0, 0.4, d);
    // �߽� ����� �ܰ� ���� interpolate
    vec3 color = mix(vec3(1.0,0.8,0.2), vec3(0.6,0.4,0.1), d);
    FragColor = vec4(color, alpha);
}