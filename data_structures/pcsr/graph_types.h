//
// Created by per on 10.07.20.
//

#ifndef PCSR2_GRAPH_TYPES_H
#define PCSR2_GRAPH_TYPES_H

#define SRC_MASK 0xffffffff00000000
#define DST_MASK 0x00000000ffffffff

#define TO_SRC(edge) (edge & SRC_MASK) >> 32
#define TO_DST(edge) edge & DST_MASK

#define TO_EDGE(src, dst) (((int64_t) src) << 32) + dst

#endif //PCSR2_GRAPH_TYPES_H
