file(GLOB EntityCore_SOURCES_LIST "Core/*.hpp" "Core/*.ipp" )
file(GLOB EntityGraph_SOURCES_LIST "Graph/*.hpp" "Graph/*.ipp" )

set(Entity_SOURCES_LIST ${EntityCore_SOURCES_LIST} ${EntityGraph_SOURCES_LIST} PARENT_SCOPE)
