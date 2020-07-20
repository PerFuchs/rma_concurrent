//
// Created by per on 20.07.20.
//

#ifndef RMA_PARALLEL_INSERT_TWO_PHASE_H
#define RMA_PARALLEL_INSERT_TWO_PHASE_H


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

#pragma once

#include <cinttypes>
#include <memory>

#include "interface.hpp"

namespace data_structures { class Interface; } // forward declaration
namespace distributions { class Interface; } // forward declaration

namespace experiments {

    class ParallelTwoPhaseInsert : public Interface {
    private:
        std::shared_ptr<data_structures::Interface> m_data_structure;
        std::shared_ptr<distributions::Interface> m_distribution;
        std::shared_ptr<distributions::Interface> m_insert_distribution;
        const uint64_t m_insert_threads;

    protected:
        void preprocess() override;
        void run() override;

    public:
        ParallelTwoPhaseInsert(std::shared_ptr<data_structures::Interface> interface, uint64_t insert_threads);

        virtual ~ParallelTwoPhaseInsert();
    };

} /* namespace experiments */

#endif //RMA_PARALLEL_INSERT_TWO_PHASE_H
