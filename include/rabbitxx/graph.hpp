#ifndef RABBITXX_GRAPH_HPP
#define RABBITXX_GRAPH_HPP

#include <rabbitxx/graph/builder/builder.hpp>

#include <utility>

namespace rabbitxx {

    template<typename Builder, typename... Args>
    auto make_graph(Args&&... args)
    {
//         using Graph = typename Builder::graph_type;
        Builder builder;
        return std::move(builder(std::forward<Args>(args)...));
    }

} // namespace rabbitxx

#endif // RABBITXX_GRAPH_HPP
