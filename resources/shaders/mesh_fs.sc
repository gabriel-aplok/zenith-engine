$input v_color0, v_uv0

#include <common.sh>

uniform vec4 u_tint;
SAMPLER2D(s_texColor, 0);

void main()
{
    vec4 base = v_color0;
    if (texture2D(s_texColor, v_uv0).a > 0.0)
    {
        base *= texture2D(s_texColor, v_uv0);
    }
    gl_FragColor = base * u_tint;
}
