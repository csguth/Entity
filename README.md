# Entity

This is an Hierarchical Data-Oriented Entity System (wow) highly inspired on [bitsquid's blog post series](http://bitsquid.blogspot.co.uk/2014/08/building-data-oriented-entity-system.html).

## From [Wikipedia](https://en.wikipedia.org/wiki/Entity%E2%80%93component%E2%80%93system):
> Entity-component system (ECS) is an architectural pattern that is mostly used in game development. An ECS follows the Composition over inheritance principle that allows greater flexibility in defining entities where every object in a game's scene is an entity (e.g. enemies, bullets, vehicles, etc.). Every Entity consists of one or more components which add additional behavior or functionality. Therefore, the behavior of an entity can be changed at runtime by adding or removing components. This eliminates the ambiguity problems of deep and wide inheritance hierarchies that are difficult to understand, maintain and extend. Common ECS approaches are highly compatible and often combined with data oriented design techniques.

## Bundled Dependencies
- [range-v3](https://github.com/ericniebler/range-v3)
- [Catch](https://github.com/philsquared/Catch)

## External Dependencies
- [CMake](https://cmake.org/)
- [SFML](https://www.sfml-dev.org/)
- [Boost](http://www.boost.org)

## How to compile and run the tests
*(Tested on Linux Manjaro)*

### 1. Ensure you have all the dependencies installed:
```
sudo pacman -S sfml boost cmake
```
### 2. Clone and `cd Entity`:
```
git submodule update --init --recursive
mkdir Build && cd Build
cmake ..
make -j
./
```
### 3. Run the tests:
```
./Entity/Core/SystemTest
./Entity/Core/PropertyTest
./Entity/Core/HierarchyTest
./Entity/Core/TupleVectorTest
./Entity/Graph/GraphTest
```

## Current features
- Entity System without erase operation
- Entity System with erase operation
- Property creation for both types of Entity Systems
- Composition (hierarchy)
  - *Strong:* Erasing an Entity will erase its children
  - *Weak:* As you can imagine, erasing an entity will not erase its children
  

## Built on top of the Core Entity System
- Graph
  - Directed and Undirected Graph (the former is more mature than the latter)
  - Dijkstra
  - Shortest Path View (ranges-v3)
  
## TODO
- Improve this readme :)
- Make Dijkstra work on both Graph and Digraph
- Create Graphs with support for erase operation
- Create a wiki for presenting the memory model
