#include <iostream>
#include <vector>

#include "./mesh.h"

using namespace std;

Mesh::Mesh() {
  _cur_mtl = -1;
}

// This will be called by the obj parser
void Mesh::AddVertex(const Vec3f& v) {
  // TODO
  Vertex x;
  x.location = v;
  _vertices.push_back(x);
  // updates the bounding box
  _bb(v);
}

// This will be called by the obj parser
void Mesh::AddTextureVertex(const Vec3f& v) {
  // TODO
  _tex_vertices.push_back(v);
}

// p is the list of indices of vertices for this polygon.  For example,
// if p = {0, 1, 2} then the polygon is a triangle with the zeroth, first and
// second vertices added using AddVertex.
//
// pt is the list of texture indices for this polygon, similar to the
// actual vertices described above.
void Mesh::AddPolygon(const std::vector<int>& p, const std::vector<int>& pt) {
  // TODO
  Polygon n;
  for (int i = 0; i < p.size(); i++) {
    n.verts.push_back(&_vertices.at(p[i]));
    if (pt[i] != -1)
      n.tex_verts.push_back(_tex_vertices.at(pt[i]));
    else
      n.tex_verts.push_back(Vec3f::makeVec(0.0f, 0.0f, 0.0f));
    // cout << "vertex at i: " << _vertices.at(p[i]) << endl;
    // cout << "tex vertex at i: " << _tex_vertices.at(p[i]) << "\n" << endl;
  }
  // make normal
  Vec3f a = (*n.verts[1]).location - (*n.verts[0]).location;
  Vec3f b = (*n.verts[2]).location - (*n.verts[0]).location;
  n.normal = a.crossProduct(b);
  n.normal = n.normal.unit();
  _polygons.push_back(n);
  // updates the poly2mat map
  _polygon2material.push_back(_cur_mtl);
}

// Computes a normal for each vertex.
void Mesh::compute_normals() {
  // TODO don't forget to normalize your normals!
  // make sure all verts start with no normal
  for (int i = 0; i < _vertices.size(); ++i) {
    _vertices[i].v_normal = Vec3f::makeVec(0.0, 0.0, 0.0);
  }
  // add polygon normal to vertices
  // Polygon w/o vertices in sponza
  cout << "start" << endl;
  for (int i = 0; i < _polygons.size(); i++) {
    for (int j = 0; j < _polygons[i].verts.size(); j++) {
      if (j == 0)
        cout << "First" << endl;
      Polygon x = _polygons[i];
      cout << "No Vertex" << endl;
      Vertex v = *x.verts[j];
      cout << "both found" << endl;
      (*_polygons[i].verts[j]).v_normal += _polygons[i].normal;
    }
  }
  cout << "end" << endl;
  // normalize normals
/*
  Vec3f x;
  for (int i = 0; i < _polygons.size(); i++) {
    for (int j = 0; j < _polygons[i].verts.size(); j++) {
      x = _polygons[i].verts[j].v_normal.unit();
      _polygons[i].verts[j].v_normal = x;
    }
  }
*/
  for (int i = 0; i < _vertices.size(); ++i) {
    _vertices[i].v_normal = _vertices[i].v_normal.unit();
  }
}








