//
// Created by per on 20.07.20.
//

#include "parallel_two_phase_insert.hpp"


/**
 * Copyright (C) 2018 Dean De Leo, email: dleo[at]cwi.nl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "parallel_insert.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>
#include <data_structures/pcsr/graph_types.h>

#include "common/configuration.hpp"
#include "common/database.hpp"
#include "common/errorhandling.hpp"
#include "common/miscellaneous.hpp"
#include "common/spin_lock.hpp"
#include "data_structures/interface.hpp"
#include "data_structures/parallel.hpp"
#include "distributions/driver.hpp"
#include "distributions/interface.hpp"
#include "data_structures/rma/baseline/packed_memory_array.hpp"
#include "distributions/graph_distribution.hpp"

using namespace std;
using namespace common;
using namespace data_structures::rma::baseline;

namespace experiments {

    namespace {

        class DistributionParallel {
        public:
            distributions::Interface *m_distribution;
            SpinLock m_lock;
            uint64_t m_position = 0; // current position in the distribution

        public:
            DistributionParallel(distributions::Interface *distribution) : m_distribution(distribution) {
              assert(distribution != nullptr);
            }

            // Fetch up to num_keys from the distribution. Return the window in the distribution for the
            // keys to use as the interval: [position, position + count).
            struct FetchOutput {
                uint64_t m_position;
                uint64_t m_count;
            };

            FetchOutput fetch(size_t num_keys) {
              scoped_lock<SpinLock> lock{m_lock}; // synchronise
              assert(m_position <= m_distribution->size() && "Overflow");
              auto position = m_position;
              size_t count = std::min(m_distribution->size() - m_position, num_keys);
              m_position += count;
              return FetchOutput{position, count};
            }

            // Get the key at the given position
            int64_t get(uint64_t index) {
              assert(index < m_distribution->size() && "Overflow");
              return m_distribution->key(index);
            }
        };

        static void pin_thread_to_socket() {
#if defined(HAVE_LIBNUMA)
          pin_thread_to_numa_node(0);
#endif
        }

        static
        void validate_index(PackedMemoryArray &ds, VertexIndex *vertex_index,
                            unordered_map<uint32_t, uint32_t> &gold_standard) {
          if (gold_standard == NULL) {
            ds.
          }

          for (const auto edge : gold_standard) {
            auto src = edge.first;
            auto dst = edge.second;
            if (ds.get_at(vertex_index->get_vertex_start(src)) != TO_EDGE(src, dst)) {
              cout << "Validation failed" << endl;
              cout << " SRC " << src;
              cout << "Position according to index " << vertex_index->get_vertex_start(src) << endl;
              cout << "Position according to find: " << ds.find(TO_EDGE(src, dst)) << endl;
              cout << "Value" << ds.get_at(vertex_index->get_vertex_start(src)) << endl;
              assert(false);
            }
          }
        }

        static
        void thread_execute_inserts(int worker_id, data_structures::Interface *data_structure,
                                    DistributionParallel *distribution, atomic<int> *startup_counter) {

          cout << "distribution size " << distribution->m_distribution->size() << endl;
          pin_thread_to_socket();

          assert(data_structure != nullptr && distribution != nullptr);

          constexpr uint64_t keys_to_fetch = 8;
          auto distribution_window = distribution->fetch(keys_to_fetch);

          // invoke the init callback
          data_structures::ParallelCallbacks *parallel_callbacks = dynamic_cast<data_structures::ParallelCallbacks *>(data_structure);
          if (parallel_callbacks != nullptr) { parallel_callbacks->on_init_worker(worker_id); }

          // wait for all threads to init
          barrier();
          (*startup_counter)--;
          while (*startup_counter > 0) /* nop */;
          barrier();

          unordered_map<uint32_t, uint32_t> gold_standard_vertex_index(ARGREF(size_t, "vertices"));
//          PackedMemoryArray &ds = dynamic_cast<PackedMemoryArray &>(*data_structure);
//          VertexIndex *vertex_index = ds.get_vertex_index();

          while (distribution_window.m_count > 0) {
            for (size_t i = 0; i < distribution_window.m_count; i++) {
              int64_t key = (distribution->get(distribution_window.m_position + i));
              int64_t value = key * 100;
              data_structure->insert(key, value);

              // Simple test case for vertex index.
//            // Update gold standard
//              auto src = TO_SRC(key);
//              auto dst = TO_DST(key);
//              if (gold_standard_vertex_index.count(src) == 0) {
//                gold_standard_vertex_index.insert(make_pair(src, dst));
//              } else if (gold_standard_vertex_index.find(src)->second > dst) {
//                gold_standard_vertex_index.insert_or_assign(src, dst);  // gold standard not updated properly why?
//              }

//            auto gold_standard = gold_standard_vertex_index.find(src);
//
//            cout << "Key: " << key << " SRC " << src << " dst " << dst << " edge " << endl;
//            cout << "Position according to index " << vertex_index->get_vertex_start(src) << endl;
//            cout << "find says: " << ds.find(TO_EDGE(gold_standard_vertex_index.find(src)->first, gold_standard_vertex_index.find(src)->second)) << endl;
//            cout << "PMA says: " << ds.get_at(vertex_index->get_vertex_start(src)) << endl;
//            assert(ds.get_at(vertex_index->get_vertex_start(src)) == TO_EDGE(gold_standard->first, gold_standard->second));
//              validate_index(ds, vertex_index, gold_standard_vertex_index);

//              cout << "Done" << endl;
            }

            // fetch the next chunk of the keys to insert
            distribution_window = distribution->fetch(keys_to_fetch);

//            if (ds.size() > 900000000) {
//              vertex_index->track = true;
//            }
          }

//          cout << "Validating" << endl;


          // invoke the clean up callback
          if (parallel_callbacks != nullptr) { parallel_callbacks->on_destroy_worker(worker_id); }

          // done
        }

    }

    ParallelTwoPhaseInsert::ParallelTwoPhaseInsert(std::shared_ptr<data_structures::Interface> data_structure,
                                                   uint64_t insert_threads)
            : m_data_structure(data_structure), m_insert_threads(insert_threads) {
      if (data_structure.get() == nullptr) RAISE_EXCEPTION(ExperimentError, "Null pointer for the PMA interface");
    }

    ParallelTwoPhaseInsert::~ParallelTwoPhaseInsert() {

    }

    void ParallelTwoPhaseInsert::preprocess() {
      if (m_data_structure.get() == nullptr) RAISE_EXCEPTION(ExperimentError, "Null pointer");

      // initialize the elements that we need to add
      LOG_VERBOSE("Generating the set of elements to insert ... ");
      m_distribution = distributions::generate_distribution();
      distributions::GraphDistribution::use_offset = true;
      m_insert_distribution = distributions::generate_distribution();
    }

    void ParallelTwoPhaseInsert::run() {
      // number of threads to start
      atomic<int> num_threads_to_start = m_insert_threads;

      // init the wrapper to fetch the keys from the distribution synchronously
      DistributionParallel distribution{m_distribution.get()};
      DistributionParallel insert_distribution{m_insert_distribution.get()};

      cout << "Insert distribution size: " << m_insert_distribution->size() << endl;

      // allow scans to execute
      ::data_structures::global_parallel_scan_enabled = true;

      // threads managed
      std::vector<thread> threads;
      threads.reserve(num_threads_to_start * 2);


      // invoke the callback for the init of the main thread
      ::data_structures::ParallelCallbacks *parallel_callbacks = dynamic_cast<::data_structures::ParallelCallbacks *>(m_data_structure.get());
      if (parallel_callbacks != nullptr) { parallel_callbacks->on_init_main(m_insert_threads); }

      // start the insertion threads
      LOG_VERBOSE("Starting `" << m_insert_threads << "' insertion threads ... ");
      for (size_t i = 0; i < m_insert_threads; i++) {
        threads.emplace_back(thread_execute_inserts, /* worker id = */ (int) i, m_data_structure.get(),
                             &distribution,
                             &num_threads_to_start);
      }

      // wait for all threads to start
      LOG_VERBOSE("Waiting for all threads to start...");
      barrier();
      while (num_threads_to_start > 0) /* nop */;
      barrier();

      LOG_VERBOSE("Executing the experiment...");
      Timer load_timer{true};
      barrier();

      // ... zzZz ...

      // wait for the experiment to complete
      for (size_t i = 0; i < m_insert_threads; i++) { threads[i].join(); }
      if (parallel_callbacks != nullptr) { parallel_callbacks->on_complete(); } // flush the asynchronous updates
      ::data_structures::global_parallel_scan_enabled = false;
      for (size_t i = m_insert_threads; i < m_insert_threads; i++) { threads[i].join(); }

      // stop the timer
      barrier();
      load_timer.stop();
      barrier();

      if (dynamic_cast<PackedMemoryArray*>(m_data_structure.get()) != nullptr) {
        PackedMemoryArray* ds = dynamic_cast<PackedMemoryArray*>(m_data_structure.get());
//        ds->start_vertex_index();
      }

      num_threads_to_start = m_insert_threads;
      for (size_t i = 0; i < m_insert_threads; i++) {
        threads.emplace_back(thread_execute_inserts, /* worker id = */ (int) i, m_data_structure.get(),
                             &insert_distribution,
                             &num_threads_to_start);
      }

      // wait for all threads to start
      LOG_VERBOSE("Waiting for all threads to start...");
      barrier();
      while (num_threads_to_start > 0) /* nop */;
      barrier();

      LOG_VERBOSE("Executing the experiment...");
      Timer insert_timer{true};
      barrier();

      // ... zzZz ...

      // wait for the experiment to complete
      for (size_t i = m_insert_threads; i < m_insert_threads * 2; i++) { threads[i].join(); }
      if (parallel_callbacks != nullptr) { parallel_callbacks->on_complete(); } // flush the asynchronous updates
      ::data_structures::global_parallel_scan_enabled = false;

      // stop the timer
      barrier();
      insert_timer.stop();
      barrier();


      // invoke the clean up callback
      if (parallel_callbacks != nullptr) { parallel_callbacks->on_destroy_main(); }

//      config().db()->add("parallel_insert")
//              ("loading", load_timer.microseconds());
//              ("inserting", insert_timer.microseconds());

      double loading_milliseconds = load_timer.milliseconds();
      LOG_VERBOSE("Loading took:" << load_timer.milliseconds() << " milliseconds" << endl);
      LOG_VERBOSE("Inserting took:" << insert_timer.milliseconds() << " milliseconds" << endl);
    }


} /* namespace pma */
