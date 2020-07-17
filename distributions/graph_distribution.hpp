//
// Created by per on 16.07.20.
//

#ifndef RMA_GRAPH_DISTRIBUTION_HPP
#define RMA_GRAPH_DISTRIBUTION_HPP

#include <vector>
#include <data_structures/pcsr/graph_types.h>

#include "interface.hpp"

using namespace std;

namespace distributions {

    struct Edge {
        uint32_t src;
        uint32_t dst;
    };

    class GraphDistribution : public Interface {

    public:
        static bool use_offset;
        static void set_use_offset(bool use_offset) { GraphDistribution::use_offset = use_offset; }

        explicit GraphDistribution(const string& filepath, size_t limit, size_t offset = 0);

        [[nodiscard]] size_t size() const {
          return edges.size();
        };

        [[nodiscard]] int64_t key(size_t index) const {
          int64_t edge = TO_EDGE(edges[index].src, edges[index].dst);
          return edge;
        };


        unique_ptr<Interface> view(size_t start, size_t length) { return unique_ptr<Interface>(nullptr); }// do we need this? };


    private:
        vector<Edge> edges;

    };

    std::unique_ptr<Interface> make_graph(const string& filepath, size_t limit, size_t offset);

}

#endif //RMA_GRAPH_DISTRIBUTION_HPP
