# pragma once

// STL
#include <utility> /* std::make_pair */

// GRAYBAT
#include <graybat/graphPolicy/Traits.hpp>


namespace graybat {

    namespace pattern {

        template<typename T_GraphPolicy>
        struct InStar {

          
            using GraphPolicy       = T_GraphPolicy;
            using VertexDescription = graybat::graphPolicy::VertexDescription<GraphPolicy>;
            using EdgeDescription   = graybat::graphPolicy::EdgeDescription<GraphPolicy>;
            using GraphDescription  = graybat::graphPolicy::GraphDescription<GraphPolicy>;
            using EdgeProperty      = graybat::graphPolicy::EdgeProperty<GraphPolicy>;
            using VertexProperty    = graybat::graphPolicy::VertexProperty<GraphPolicy>;  


            const unsigned verticesCount;

            InStar(const unsigned verticesCount) :
                verticesCount(verticesCount) {

            }
    

            GraphDescription operator()(){
                std::vector<VertexDescription> vertices(verticesCount);
                std::vector<EdgeDescription> edges;

                for(unsigned i = 0; i < vertices.size(); ++i){
                    vertices.at(i) = std::make_pair(i, VertexProperty());
                }

                    
                for(unsigned i = 0; i < vertices.size(); ++i){
                    if(i != 0){
                        edges.push_back(std::make_pair(std::make_pair(vertices[i].first, vertices[0].first), EdgeProperty()));
                    }
		
                }

                return std::make_pair(vertices,edges);
            }

        };

    } /* namespace pattern */

} /* namespace graybat */
