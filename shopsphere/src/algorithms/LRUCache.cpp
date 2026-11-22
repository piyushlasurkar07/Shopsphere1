#include "algorithms/LRUCache.h"
#include <string>

namespace ShopSphere {
namespace Algorithms {

// Explicit template instantiations for commonly used types in ShopSphere
template class LRUCache<int, std::string>;
template class LRUCache<std::string, std::string>;

} // namespace Algorithms
} // namespace ShopSphere
