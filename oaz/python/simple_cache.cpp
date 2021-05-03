#include "oaz/cache/simple_cache.hpp"

#include <boost/python.hpp>
#include <boost/python/def.hpp>
#include <boost/python/module.hpp>

#include "Python.h"
#include "oaz/games/game.hpp"

namespace p = boost::python;

BOOST_PYTHON_MODULE(simple_cache) { // NOLINT
  PyEval_InitThreads();

  p::class_<oaz::cache::SimpleCache, std::shared_ptr<oaz::cache::SimpleCache>,
            boost::noncopyable, p::bases<oaz::cache::Cache> >(
      "SimpleCache", p::init<const oaz::games::Game&, size_t>())
      .add_property("n_hits", &oaz::cache::SimpleCache::GetNumberOfHits)
      .add_property("n_objects", &oaz::cache::SimpleCache::GetNumberOfObjects)
      .add_property("size", &oaz::cache::SimpleCache::GetSize);
}
