//
// Created by per on 10.07.20.
//

#include "vertex_index.hpp"
#include "iostream"

size_t VertexIndex::get_vertex_start(uint32_t vertex) {
  return vertex_starts[vertex];
}

void VertexIndex::set_vertex_start(uint32_t vertex, size_t start) {
  if (vertex_starts[vertex] == UNDEFINED) {
    v.insert(vertex);
  }
  vertex_starts[vertex] = start;
}

uint32_t VertexIndex::get_vertex_degree(uint32_t vertex) {
  return degrees[vertex];
}

void VertexIndex::set_vertex_degree(uint32_t vertex, uint32_t degree) {
  degrees[vertex] = degree;
}


bool VertexIndex::is_undefined(uint32_t vertex) {
  return vertex_starts[vertex] == UNDEFINED;
}

void VertexIndex::set_undefined(uint32_t vertex) {
  vertex_starts[vertex] = UNDEFINED;
  degrees[vertex] = UNDEFINED;
  v.erase(vertex);
}

const set<uint32_t> &VertexIndex::vertices() const {
  return v;
}