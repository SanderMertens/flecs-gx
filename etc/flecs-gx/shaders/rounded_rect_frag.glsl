in vec4 pos;
in vec2 size;
in vec3 color;
in vec4 corner_radius;
in float stroke_width;
in vec3 stroke_color;

out vec4 frag_color;

float sd_rounded_rect(vec2 pos, vec2 size, vec4 corner_radii) {
  vec2 rs = 0.0 > pos.y ? corner_radii.xy : corner_radii.zw;
  float radius = 0.0 > pos.x ? rs.x : rs.y;

  vec2 dpos = abs(pos) - 0.5 * size;
  vec2 q = dpos + radius;
  float l = length(max(q, vec2(0.0)));
  float m = min(max(q.x, q.y), 0.0);
  return l + m - radius;
}

void main() {
  float d = sd_rounded_rect(pos.xy, size, corner_radius);
  
  if (d > 0) {
    discard;
  } else if (d > -stroke_width) {
    frag_color = vec4(stroke_color, 1.0);
  } else {
    frag_color = vec4(color, 1.0);
  }
}
