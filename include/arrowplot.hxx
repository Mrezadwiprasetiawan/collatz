#pragma once
// ═══════════════════════════════════════════════════════════════════
//  arrowplot.hxx  —  header-only 3D geometry library
//
//  Produces raw triangle mesh data (flat float arrays, 7 floats/vert:
//  x y z r g b a) for:
//    · Sphere  per node
//    · Cylinder shaft  per segment
//    · Cone    arrowhead per segment
//
//  No rendering dependency — upload straight to GL_ARRAY_BUFFER.
// ═══════════════════════════════════════════════════════════════════

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace ap {

static constexpr float kPi = 3.14159265358979323846f;

// ──────────────────────────── Vec3 ──────────────────────────────

struct Vec3 {
  float x = 0, y = 0, z = 0;

  Vec3() = default;
  Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  Vec3 operator+(Vec3 o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vec3 operator-(Vec3 o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
  Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }

  float dot(Vec3 o) const { return x * o.x + y * o.y + z * o.z; }
  float length() const { return std::sqrt(x * x + y * y + z * z); }

  Vec3 normalized() const {
    float l = length();
    return l > 1e-7f ? Vec3{x / l, y / l, z / l} : Vec3{};
  }

  Vec3 cross(Vec3 o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
};

// ──────────────────────────── config ────────────────────────────

struct Config {
  // Layout
  float padding = 0.1f;  // NDC padding [0..1] on each axis

  // Node sphere
  float nodeRadius   = 0.04f;  // NDC radius
  int   sphereStacks = 12;     // latitude subdivisions
  int   sphereSlices = 20;     // longitude subdivisions

  // Shaft cylinder
  float shaftRadius = 0.008f;  // NDC radius of cylinder
  int   shaftSlices = 12;      // circumference subdivisions

  // Arrowhead cone
  float headLength = 0.06f;   // NDC length of cone
  float headRadius = 0.025f;  // NDC base radius of cone
  int   headSlices = 16;      // circumference subdivisions

  // Extra gap between geometry and node surface
  float tipGap = 0.0f;

  // Y axis: false = math Y-up, true = Y-down
  bool yDown = false;

  // Translate all geometry after NDC mapping.
  // Default {-0.5, -0.5, -0.5} shifts plot into negative space.
  float offsetX = 0.0f;
  float offsetY = 0.0f;
  float offsetZ = 0.0f;
};

// ──────────────────────────── colours ───────────────────────────

struct ColourScheme {
  float nodeR = 1, nodeG = 1, nodeB = 1, nodeA = 1;
  float shaftR = 0, shaftG = 0.83f, shaftB = 0.67f, shaftA = 1;
  float headR = 0, headG = 0.83f, headB = 0.67f, headA = 1;
};

// ──────────────────────────── mesh output ───────────────────────
// Vertex layout: x y z r g b a  (7 floats, stride = 28 bytes)

struct Mesh {
  std::vector<float> sphereVerts;  // node spheres
  std::vector<float> shaftVerts;   // cylinder shafts
  std::vector<float> headVerts;    // cone arrowheads

  int sphereVertCount() const { return (int)(sphereVerts.size() / 7); }
  int shaftVertCount() const { return (int)(shaftVerts.size() / 7); }
  int headVertCount() const { return (int)(headVerts.size() / 7); }
};

// ──────────────────────────── helpers ───────────────────────────

// Build an orthonormal frame {u, v} perpendicular to axis `d`
static void buildFrame(Vec3 d, Vec3& u, Vec3& v) {
  // pick a non-parallel reference vector
  Vec3 ref = (std::abs(d.x) < 0.9f) ? Vec3{1, 0, 0} : Vec3{0, 1, 0};
  u        = d.cross(ref).normalized();
  v        = d.cross(u);
}

static void pushVert(std::vector<float>& buf, Vec3 p, float r, float g, float b, float a) { buf.insert(buf.end(), {p.x, p.y, p.z, r, g, b, a}); }

// ──────────────────────────── ArrowPlot ─────────────────────────

class ArrowPlot {
 public:
  struct Point {
    float       x, y, z;
    std::string label;
  };

  // ── add points ───────────────────────────────────────────────

  void add(float x, float y, float z, const std::string& lbl = "") { pts_.push_back({x, y, z, lbl.empty() ? "P" + std::to_string(pts_.size() + 1) : lbl}); }

  void add(std::initializer_list<std::array<float, 3>> list) {
    for (auto& a : list) add(a[0], a[1], a[2]);
  }

  void clear() { pts_.clear(); }

  const std::vector<Point>& points() const { return pts_; }
  std::size_t               size() const { return pts_.size(); }

  // ── build ────────────────────────────────────────────────────

  Mesh build(const Config& cfg = {}, const ColourScheme& col = {}) const {
    if (pts_.size() < 2) throw std::invalid_argument("ArrowPlot: need at least 2 points");

    std::vector<Vec3> ndc = toNDC(cfg);
    Mesh              mesh;

    for (size_t i = 0; i < ndc.size(); ++i) {
      float t = (ndc.size() > 1) ? (float)i / (float)(ndc.size() - 1) : 0.f;
      // yellow (1,1,0) → red (1,0,0)
      float R = 1.f, G = 1.f - t, B = 0.f, A = col.nodeA;
      buildSphere(mesh.sphereVerts, ndc[i], cfg, R, G, B, A);
    }

    for (size_t i = 0; i + 1 < ndc.size(); ++i) buildArrow(mesh, ndc[i], ndc[i + 1], cfg, col);

    return mesh;
  }

 private:
  std::vector<Point> pts_;

  // ── coord mapping: data → NDC [-1, 1] ───────────────────────
  std::vector<Vec3> toNDC(const Config& cfg) const {
    float minX = pts_[0].x, maxX = minX;
    float minY = pts_[0].y, maxY = minY;
    float minZ = pts_[0].z, maxZ = minZ;

    for (auto& p : pts_) {
      minX = std::min(minX, p.x);
      maxX = std::max(maxX, p.x);
      minY = std::min(minY, p.y);
      maxY = std::max(maxY, p.y);
      minZ = std::min(minZ, p.z);
      maxZ = std::max(maxZ, p.z);
    }

    float usable = 2.f - 2.f * cfg.padding;
    auto  map    = [&](float v, float lo, float hi, bool flip) -> float {
      if (hi - lo < 1e-7f) return 0.f;
      float t = (v - lo) / (hi - lo);
      if (flip) t = 1.f - t;
      return -1.f + cfg.padding + t * usable;
    };

    std::vector<Vec3> out;
    for (auto& p : pts_)
      out.push_back({map(p.x, minX, maxX, false) + cfg.offsetX, map(p.y, minY, maxY, cfg.yDown) + cfg.offsetY, map(p.z, minZ, maxZ, false) + cfg.offsetZ});
    return out;
  }

  // ── sphere (UV sphere, triangle list) ───────────────────────
  void buildSphere(std::vector<float>& buf, Vec3 c, const Config& cfg, float R, float G, float B, float A) const {
    float r      = cfg.nodeRadius;
    int   stacks = cfg.sphereStacks;
    int   slices = cfg.sphereSlices;

    // precompute ring positions
    // stack 0 = south pole, stack stacks = north pole
    auto pt = [&](int stack, int slice) -> Vec3 {
      float phi   = kPi * (float)stack / stacks;      // 0..pi
      float theta = 2 * kPi * (float)slice / slices;  // 0..2pi
      return {c.x + r * std::sin(phi) * std::cos(theta), c.y + r * std::cos(phi), c.z + r * std::sin(phi) * std::sin(theta)};
    };

    for (int st = 0; st < stacks; ++st) {
      for (int sl = 0; sl < slices; ++sl) {
        Vec3 v00 = pt(st, sl);
        Vec3 v10 = pt(st + 1, sl);
        Vec3 v01 = pt(st, sl + 1);
        Vec3 v11 = pt(st + 1, sl + 1);

        // two triangles per quad
        pushVert(buf, v00, R, G, B, A);
        pushVert(buf, v10, R, G, B, A);
        pushVert(buf, v11, R, G, B, A);

        pushVert(buf, v00, R, G, B, A);
        pushVert(buf, v11, R, G, B, A);
        pushVert(buf, v01, R, G, B, A);
      }
    }
  }

  // ── cylinder between two points (open-ended) ─────────────────
  void buildCylinder(std::vector<float>& buf, Vec3 A, Vec3 B, float radius, int slices, float R, float G, float Bl, float Al) const {
    Vec3 axis = (B - A).normalized();
    Vec3 u, v;
    buildFrame(axis, u, v);

    // ring at A and ring at B
    auto ring = [&](Vec3 centre, int sl) -> Vec3 {
      float theta = 2 * kPi * (float)sl / slices;
      return centre + u * (radius * std::cos(theta)) + v * (radius * std::sin(theta));
    };

    for (int sl = 0; sl < slices; ++sl) {
      Vec3 a0 = ring(A, sl);
      Vec3 a1 = ring(A, sl + 1);
      Vec3 b0 = ring(B, sl);
      Vec3 b1 = ring(B, sl + 1);

      pushVert(buf, a0, R, G, Bl, Al);
      pushVert(buf, b0, R, G, Bl, Al);
      pushVert(buf, b1, R, G, Bl, Al);

      pushVert(buf, a0, R, G, Bl, Al);
      pushVert(buf, b1, R, G, Bl, Al);
      pushVert(buf, a1, R, G, Bl, Al);
    }
  }

  // ── cone (apex → base circle) ────────────────────────────────
  void buildCone(std::vector<float>& buf, Vec3 tip, Vec3 baseCenter, float baseRadius, int slices, float R, float G, float Bl, float Al) const {
    Vec3 axis = (baseCenter - tip).normalized();
    Vec3 u, v;
    buildFrame(axis, u, v);

    auto baseRing = [&](int sl) -> Vec3 {
      float theta = 2 * kPi * (float)sl / slices;
      return baseCenter + u * (baseRadius * std::cos(theta)) + v * (baseRadius * std::sin(theta));
    };

    for (int sl = 0; sl < slices; ++sl) {
      Vec3 b0 = baseRing(sl);
      Vec3 b1 = baseRing(sl + 1);

      // side face: tip → b0 → b1
      pushVert(buf, tip, R, G, Bl, Al);
      pushVert(buf, b0, R, G, Bl, Al);
      pushVert(buf, b1, R, G, Bl, Al);

      // base disc: baseCenter → b1 → b0
      pushVert(buf, baseCenter, R, G, Bl, Al);
      pushVert(buf, b1, R, G, Bl, Al);
      pushVert(buf, b0, R, G, Bl, Al);
    }
  }

  // ── shaft + arrowhead for one segment ───────────────────────
  void buildArrow(Mesh& mesh, Vec3 A, Vec3 B, const Config& cfg, const ColourScheme& col) const {
    Vec3 dir = (B - A).normalized();

    // cone tip lands on node surface
    Vec3 tip      = B - dir * (cfg.nodeRadius + cfg.tipGap);
    Vec3 coneBase = tip - dir * cfg.headLength;

    // shaft from node surface to cone base
    Vec3 shaftStart = A + dir * cfg.nodeRadius;
    Vec3 shaftEnd   = coneBase;

    buildCylinder(mesh.shaftVerts, shaftStart, shaftEnd, cfg.shaftRadius, cfg.shaftSlices, col.shaftR, col.shaftG, col.shaftB, col.shaftA);

    buildCone(mesh.headVerts, tip, coneBase, cfg.headRadius, cfg.headSlices, col.headR, col.headG, col.headB, col.headA);
  }
};

}  // namespace ap
