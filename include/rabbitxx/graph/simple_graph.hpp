#ifndef __RABBITXX_GRAPH_SIMPLE_GRAPH_HPP__
#define __RABBITXX_GRAPH_SIMPLE_GRAPH_HPP__

#include <rabbitxx/graph/bgl_base_graph.hpp>
#include <rabbitxx/graph/otf2_trace_event.hpp>

#include <boost/graph/adjacency_list.hpp>

namespace rabbitxx {

    using simple_graph_impl = boost::adjacency_list<
                                        boost::vecS,
                                        boost::vecS,
                                        boost::directedS,
                                        otf2_trace_event>;

    using SimpleGraph = rabbitxx::graph::graph<simple_graph_impl>;

} // namespace rabbitxx


#endif /* __RABBITXX_GRAPH_SIMPLE_GRAPH_HPP__ */
