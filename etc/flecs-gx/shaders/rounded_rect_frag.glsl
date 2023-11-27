in vec4 pos;
in vec2 size;
in vec3 color;
in float corner_radius;
in float stroke_width;
in vec3 stroke_color;

out vec4 frag_color;

float sd_rounded_rect(vec2 pos, vec2 size, float corner_radius) {
  vec2 dpos = abs(pos) - 0.5 * size;
  vec2 q = dpos + corner_radius;
  float l = length(max(q, vec2(0.0)));
  float m = min(max(q.x, q.y), 0.0);
  return l + m - corner_radius;
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
