//
// Created by per on 10.07.20.
//

#ifndef PCSR2_VERTEXINDEX_H
#define PCSR2_VERTEXINDEX_H


#include <cstdint>
#include <vector>
#include <limits>
#include <iterator>
#include <functional>
#include <set>

using namespace std;

class VertexIndex {
    set<uint32_t> v;
    vector<size_t> vertex_starts;
    vector<uint32_t> degrees;
    bool track = false;

public:
    const static uint32_t UNDEFINED = std::numeric_limits<uint32_t>::max();

    // Hacky way to use UNDEFINED. Compiler complains it's not defined for use in constructor
    explicit VertexIndex(uint32_t max_vertices) :
            vertex_starts(max_vertices, std::numeric_limits<uint32_t>::max()),
            degrees(max_vertices, std::numeric_limits<uint32_t>::max()),
            v() {};

    size_t get_vertex_start(uint32_t vertex);

    void set_vertex_start(uint32_t vertex, size_t start);

    uint32_t get_vertex_degree(uint32_t vertex);

    void set_vertex_degree(uint32_t vertex, uint32_t degree);

    bool is_undefined(uint32_t vertex);

    void set_undefined(uint32_t vertex);

    const set<uint32_t> &vertices() const;

    void start_tracking();
};


#endif //PCSR2_VERTEXINDEX_H
