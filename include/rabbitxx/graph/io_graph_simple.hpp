#ifndef RABBITXX_GRAPH_SIMPLE_GRAPH_HPP
#define RABBITXX_GRAPH_SIMPLE_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

#include <rabbitxx/graph/bgl_base_graph.hpp>
#include <rabbitxx/trace/otf2_trace_event.hpp>

namespace rabbitxx {

    namespace graph { namespace detail {

        using simple_io_graph_impl = boost::adjacency_list<
                                                        boost::vecS, // store out-edges of vertex in std::vector
                                                        boost::vecS, // store vertices in a std::vector
                                                        //boost::directedS, // the graph is directed
                                                        boost::bidirectionalS, // the graph is directed
                                                        otf2_trace_event>; // use otf2_trace_event as vertex property
    }} // namespace graph::detail

    using SimpleIoGraph = graph::graph<graph::detail::simple_io_graph_impl>;

} // namespace rabbitxx


#endif /* RABBITXX_GRAPH_SIMPLE_GRAPH_HPP */
