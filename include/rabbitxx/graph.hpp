#ifndef __RABBITXX_GRAPH_HPP__
#define __RABBITXX_GRAPH_HPP__

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

#endif // __RABBITXX_GRAPH_HPP__
