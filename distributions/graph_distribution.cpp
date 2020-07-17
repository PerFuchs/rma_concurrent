//
// Created by per on 16.07.20.
//

#include <iostream>
#include <cassert>
#include <sys/stat.h>

#include "graph_distribution.hpp"

using namespace std;

namespace distributions {

    bool GraphDistribution::use_offset = false;

    std::size_t fsize(const string& fname)
    {
      struct stat st;
      if (0 == stat(fname.c_str(), &st)) {
        return st.st_size;
      }
      perror("stat issue");
      return -1L;
    }


    vector<Edge> read_binary(const string& filename, size_t limit, size_t offset) {
      cout << "Reading binary graph file: " << filename << endl;

      FILE *file = fopen(filename.c_str(), "rb");
      assert(file && "Filename incorrect");

      size_t fileSize = fsize(filename);
      size_t numberOfEdges = fileSize / sizeof(Edge) - offset * sizeof(Edge);

      fseek(file, offset * sizeof(Edge), SEEK_SET);

      vector<Edge> edges;
      edges.reserve(numberOfEdges);

      size_t edge_counter = 0;
      Edge edge = {0, 0};
      while (edge_counter <= limit && fread(&edge, sizeof(edge), 1, file) > 0) {
        edges.push_back(edge);

        edge_counter++;
      }

      fclose(file);
      return edges;
    }

    GraphDistribution::GraphDistribution(const string& filepath, size_t limit, size_t offset) {
      edges = read_binary(filepath, limit, offset);
    }

    std::unique_ptr<Interface> make_graph(const string& filepath, size_t limit, size_t offset) {
      if (!GraphDistribution::use_offset || offset == numeric_limits<size_t>::max()) {
        offset = 0;
      }
      return std::unique_ptr<Interface> (new
      GraphDistribution(filepath, limit, offset));
    }
}