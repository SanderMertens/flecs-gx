out vec4 pos;
out vec2 size;
out vec3 color;
out float corner_radius;
out float stroke_width;
out vec3 stroke_color;

void main() {
  mat4 mat_mvp = u_mat_vp * i_mat_m;
  
  vec4 tl = vec4(-0.5, -0.5, 0, 0) * i_mat_m;
  vec4 br = vec4(0.5, 0.5, 0, 0) * i_mat_m;

  vec4 pos4 = vec4(v_position, 1.0);
  size = vec2(br.x - tl.x, br.y - tl.y);
  pos = pos4 * i_mat_m;
  color = i_color;
  corner_radius = i_corner_radius;
  stroke_width = i_stroke_width;
  stroke_color = i_stroke_color;

  gl_Position = mat_mvp * pos4;
}
