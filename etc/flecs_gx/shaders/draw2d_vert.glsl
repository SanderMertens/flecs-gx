out vec4 color;

void main() {
  vec4 pos4 = vec4(v_position, 1.0);
  gl_Position = u_mat_vp * i_mat_m * pos4;
  color = vec4(i_color, 0.0);
}
