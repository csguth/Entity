#ifndef NETLIST_HPP
#define NETLIST_HPP

#include <Entity/Core/SystemWithDeletion.hpp>
#include <Entity/Core/KeyWrapper.hpp>
#include <Entity/Core/Composition.hpp>

ENTITY_ENTITY_DECLARATION(InputPort)
ENTITY_ENTITY_DECLARATION(OutputPort)
ENTITY_ENTITY_DECLARATION(ModuleDecl)
ENTITY_ENTITY_DECLARATION(ModuleInst)
ENTITY_ENTITY_DECLARATION(Wire)

template <typename EntityType>
struct MappedSystem
{
    MappedSystem() :
        system(),
        map(system)
    {}

    Entity::System<EntityType> system;
    decltype(Entity::makeKeyWrapper<std::string>(system)) map;
};

class Netlist
{
public:
    Netlist() :
        m_moduleDecls(),
        m_inputPorts(),
        m_outputPorts(),
        m_moduleInsts(),
        m_wires(),
        m_moduleWires(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_wires.system)),
        m_insts(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_moduleInsts.system)),
        m_modulesInsts(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_moduleInsts.system)),
        m_moduleInputs(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_inputPorts.system)),
        m_moduleOutputs(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_outputPorts.system))
    { }

    ModuleDecl addOrGetModuleDecl(std::string name)
    {
        ModuleDecl decl = m_moduleDecls.map.addOrGet(name);
        if (topLevel() == ModuleDecl{})
            topLevel(decl);
        return decl;
    }

    InputPort addOrGetInputPort(ModuleDecl module, std::string name)
    {
        auto port = m_inputPorts.map.addOrGet(m_moduleDecls.map.key(module) + "." + name);
        if (m_moduleInputs.parent(port) != module)
            m_moduleInputs.addChild(module, port);
        return port;
    }

    OutputPort addOrGetOutputPort(ModuleDecl module, std::string name)
    {
        auto port = m_outputPorts.map.addOrGet(m_moduleDecls.map.key(module) + "." + name);
        if (m_moduleOutputs.parent(port) != module)
            m_moduleOutputs.addChild(module, port);
        return port;
    }

    ModuleInst addOrGetModuleInst(ModuleDecl parent, ModuleDecl decl, std::string name)
    {
        auto inst = m_moduleInsts.map.addOrGet(m_moduleDecls.map.key(parent) + "." + name);
        m_insts.addChild(decl, inst);
        m_modulesInsts.addChild(parent, inst);
        return inst;
    }

    Wire addOrGetWire(ModuleDecl decl, std::string name)
    {
        auto wire = m_wires.map.addOrGet(m_moduleDecls.map.key(decl) + "." + name);
        if (m_moduleWires.parent(wire) != decl)
            m_moduleWires.addChild(decl, wire);
        return wire;
    }

    ModuleDecl parent(ModuleInst inst) const
    {
        return m_modulesInsts.parent(inst);
    }

    ModuleDecl decl(ModuleInst inst) const
    {
        return m_insts.parent(inst);
    }

    const std::string& name(ModuleInst inst) const
    {
        return m_moduleInsts.map.key(inst);
    }

    const std::string& name(ModuleDecl decl) const
    {
        return m_moduleDecls.map.key(decl);
    }

    const std::string& name(Wire wire) const
    {
        return m_wires.map.key(wire);
    }

    const std::string& name(InputPort port) const
    {
        return m_inputPorts.map.key(port);
    }

    const std::string& name(OutputPort port) const
    {
        return m_outputPorts.map.key(port);
    }

    auto inputPortsSize(ModuleDecl module) const
    {
        return m_moduleInputs.childrenSize(module);
    }

    auto outputPortsSize(ModuleDecl module) const
    {
        return m_moduleOutputs.childrenSize(module);
    }

    auto instsSize(ModuleDecl module) const
    {
        return m_insts.childrenSize(module);
    }

    auto wiresSize(ModuleDecl module) const
    {
        return m_moduleWires.childrenSize(module);
    }

    std::size_t moduleDeclsSize() const
    {
        return m_moduleDecls.system.size();
    }

    std::size_t moduleInstsSize() const
    {
        return m_moduleInsts.system.size();
    }

    void topLevel(ModuleDecl decl)
    {
        m_topLevel = decl;
    }

    ModuleDecl topLevel() const
    {
        return m_topLevel;
    }

    auto moduleDecls() const
    {
        return m_moduleDecls.system.asRange();
    }

    auto moduleInsts() const
    {
        return m_moduleInsts.system.asRange();
    }

    auto inputPorts(ModuleDecl decl) const
    {
        return m_moduleInputs.children(decl);
    }

    auto outputPorts(ModuleDecl decl) const
    {
        return m_moduleOutputs.children(decl);
    }

    auto wires(ModuleDecl decl) const
    {
        return m_moduleWires.children(decl);
    }

    auto insts(ModuleDecl decl) const
    {
        return m_insts.children(decl);
    }

private:
    MappedSystem<ModuleDecl> m_moduleDecls;
    MappedSystem<InputPort> m_inputPorts;
    MappedSystem<OutputPort> m_outputPorts;
    MappedSystem<ModuleInst> m_moduleInsts;
    MappedSystem<Wire> m_wires;

    decltype(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_wires.system)) m_moduleWires;
    decltype(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_moduleInsts.system)) m_insts;
    decltype(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_moduleInsts.system)) m_modulesInsts;
    decltype(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_inputPorts.system)) m_moduleInputs;
    decltype(Entity::makeComposition<Entity::Both>(m_moduleDecls.system, m_outputPorts.system)) m_moduleOutputs;

    ModuleDecl m_topLevel;


};

#endif // NETLIST_HPP
