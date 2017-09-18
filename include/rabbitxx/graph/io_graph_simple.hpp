#ifndef RABBITXX_GRAPH_SIMPLE_GRAPH_HPP
#define RABBITXX_GRAPH_SIMPLE_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

#include <rabbitxx/graph/bgl_base_graph.hpp>
#include <rabbitxx/trace/otf2_trace_event.hpp>

namespace rabbitxx {

    namespace graph { namespace detail {

        using simple_io_graph_impl = boost::adjacency_list<
                                                        boost::vecS,
                                                        boost::vecS,
                                                        boost::directedS,
                                                        otf2_trace_event>;
    }} // namespace graph::detail

    using SimpleIoGraph = graph::graph<graph::detail::simple_io_graph_impl>;

} // namespace rabbitxx


#endif /* RABBITXX_GRAPH_SIMPLE_GRAPH_HPP */
