#include <bgfx_shader.sh>
#include "varying.def.sc"

void main()
{
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
    v_normal = a_normal;
    v_color0 = a_color0;
}
