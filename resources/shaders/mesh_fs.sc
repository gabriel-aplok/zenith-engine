$input v_color0

#include <common.sh>

uniform vec4 u_tint;

void main()
{
    gl_FragColor = v_color0 * u_tint;
}
