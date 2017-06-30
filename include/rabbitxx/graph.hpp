#ifndef __RABBITXX_GRAPH_HPP__
#define __RABBITXX_GRAPH_HPP__

#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>

#include <memory>

namespace rabbitxx {

    using graph_impl = boost::adjacency_list<
                                boost::vecS,
                                boost::distributedS<boost::graph::distributed::mpi_process_group,
                                                    boost::vecS>,
                                boost::undirectedS>;

    template<typename GraphImpl>
    class graph
    {
        public:
            using vertex_descriptor = typename boost::graph_traits<GraphImpl>::vertex_descriptor;
            using edge_descriptor = typename boost::graph_traits<GraphImpl>::edge_descriptor;

            graph() : graph_(std::make_unique<GraphImpl>())
            {
            }

            ~graph()
            {
            }

            GraphImpl* get() noexcept
            {
                return graph_.get();
            }

            vertex_descriptor add_vertex()
            {
                return add_vertex(*graph_);
            }

            edge_descriptor add_edge(const vertex_descriptor& vd_from,
                                     const vertex_descriptor& vd_to)
            {
                return add_edge(vd_from, vd_to, *graph_);
            }

        private:
            std::unique_ptr<GraphImpl> graph_;
    };

    using Graph = graph<graph_impl>;

} // namespace rabbitxx

#endif // __RABBITXX_GRAPH_HPP__
