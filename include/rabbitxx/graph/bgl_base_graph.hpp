#ifndef RABBITXX_GRAPH_BGL_BASE_GRAPH_HPP
#define RABBITXX_GRAPH_BGL_BASE_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>

#include <string>
#include <algorithm>
#include <memory>

namespace rabbitxx { namespace graph {

    template<typename GraphImpl>
    class graph
    {
        public:
            using impl_type = GraphImpl;
            using vertex_descriptor = typename boost::graph_traits<GraphImpl>::vertex_descriptor;
            using vertex_iterator = typename boost::graph_traits<GraphImpl>::vertex_iterator;
            using edge_descriptor = typename boost::graph_traits<GraphImpl>::edge_descriptor;
            using edge_iterator = typename boost::graph_traits<GraphImpl>::edge_iterator;
            using adjacency_iterator = typename boost::graph_traits<GraphImpl>::adjacency_iterator;
            using vertex_type = typename boost::vertex_bundle_type<GraphImpl>::type;
            using edge_type = typename boost::vertex_bundle_type<GraphImpl>::type;
            using vertex_range = std::pair<vertex_iterator, vertex_iterator>;
            using edge_range = std::pair<edge_iterator, edge_iterator>;
            using edge_add_t = std::pair<edge_descriptor, bool>;
            using degree_size_t = typename boost::graph_traits<GraphImpl>::degree_size_type;

            static constexpr vertex_descriptor null_vertex()
            {
                return boost::graph_traits<GraphImpl>::null_vertex();
            }

            graph() noexcept : graph_(std::make_unique<GraphImpl>())
            {
            }

            GraphImpl* get() noexcept
            {
                return graph_.get();
            }

            vertex_descriptor add_vertex(const vertex_type& v)
            {
                const auto vd = boost::add_vertex(v, *graph_);
                return vd;
            }

            edge_add_t add_edge(const vertex_descriptor& vd_from,
                                const vertex_descriptor& vd_to)
            {
                assert(vd_from != null_vertex());
                assert(vd_to != null_vertex());
                return boost::add_edge(vd_from, vd_to, *graph_);
            }

            //TODO need const version
            vertex_range vertices()
            {
                return boost::vertices(*graph_);
            }

            const vertex_range vertices() const
            {
                return boost::vertices(*graph_);
            }

            edge_range edges()
            {
                return boost::edges(*graph_);
            }

            std::vector<vertex_descriptor> vertex_descriptors()
            {
                std::vector<vertex_descriptor> vds(num_vertices());
                auto v_range = boost::vertices(*graph_);
                std::copy(v_range.first, v_range.second, std::begin(vds));
                return vds;
            }

            std::vector<edge_descriptor> edge_descriptors()
            {
                std::vector<edge_descriptor> eds(num_edges());
                auto e_range = edges();
                std::copy(e_range.first,  e_range.second, std::begin(eds));
                return eds;
            }

            std::size_t num_vertices() const
            {
                return boost::num_vertices(*graph_);
            }

            std::size_t num_edges() const
            {
                return boost::num_edges(*graph_);
            }

            degree_size_t out_degree(vertex_descriptor vd) const
            {
                assert(vd != null_vertex());
                return boost::out_degree(vd, *graph_);
            }

            vertex_type& operator[](const vertex_descriptor& vd) const
            {
                assert(vd != null_vertex());
                return graph_->operator[](vd);
            }

        private:
            std::unique_ptr<GraphImpl> graph_;
    };

}} // namespace rabbitxx::graph

#endif /* RABBITXX_GRAPH_BGL_BASE_GRAPH_HPP */
