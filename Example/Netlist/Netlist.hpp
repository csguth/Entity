#ifndef NETLIST_HPP
#define NETLIST_HPP

#include <Entity/Core/SystemWithDeletion.hpp>
#include <Entity/Core/KeyWrapper.hpp>
#include <Entity/Core/Composition.hpp>
#include <boost/variant/get.hpp>

ENTITY_ENTITY_DECLARATION(InputPort)
ENTITY_ENTITY_DECLARATION(OutputPort)
ENTITY_ENTITY_DECLARATION(ModuleDecl)
ENTITY_ENTITY_DECLARATION(ModuleInst)
ENTITY_ENTITY_DECLARATION(Wire)
ENTITY_ENTITY_DECLARATION(MappedPort)

template <typename EntityType>
struct MappedSystem
{
    MappedSystem() :
        system(),
        map(system)
    {}

    Entity::SystemWithDeletion<EntityType> system;
    decltype(Entity::makeKeyWrapper<std::string>(system)) map;
};

class Netlist
{
public:
    Netlist() :
        mDecls(),
        mInputs(),
        mOutputs(),
        mInsts(),
        mInstsWeak(mInsts.system),
        mWires(),
        mMappedPort(),
        mMappedPortWeak(mMappedPort),
        mDeclWires(Entity::makeComposition<Entity::Both>(mDecls.system, mWires.system)),
        mDeclInsts(Entity::makeComposition<Entity::Both>(mDecls.system, mInstsWeak)),
        mDeclChildInsts(Entity::makeComposition<Entity::Both>(mDecls.system, mInstsWeak)),
        mDeclInputs(Entity::makeComposition<Entity::Both>(mDecls.system, mInputs.system)),
        mDeclOutputs(Entity::makeComposition<Entity::Both>(mDecls.system, mOutputs.system)),
        mInstsMappedPorts(Entity::makeComposition<Entity::Both>(mInsts.system, mMappedPort)),
        mWiresMappedPorts(Entity::makeComposition<Entity::Both>(mWires.system, mMappedPortWeak)),
        mMappedPortsPorts(Entity::makeProperty<boost::variant<InputPort, OutputPort>>(mMappedPort))
    { }

    ModuleDecl addOrGetModuleDecl(std::string name)
    {
        return mDecls.map.addOrGet(name);
    }

    MappedPort addOrGetInputPort(ModuleDecl module, std::string theName)
    {
        auto port = mInputs.map.addOrGet(name(module) + "." + theName);
        if (mDeclInputs.parent(port) != module)
            mDeclInputs.addChild(module, port);
        return mapPort(ModuleInst{}, port, addOrGetWire(module, theName));
    }

    MappedPort addOrGetOutputPort(ModuleDecl module, std::string theName)
    {
        auto port = mOutputs.map.addOrGet(name(module) + "." + theName);
        if (mDeclOutputs.parent(port) != module)
            mDeclOutputs.addChild(module, port);
        return mapPort(ModuleInst{}, port, addOrGetWire(module, theName));
    }

    ModuleInst addOrGetModuleInst(ModuleDecl parent, ModuleDecl decl, std::string theName)
    {
        const bool orphan = parent == ModuleDecl{};
        const std::string parentName = [&]() -> std::string
        {
            if (orphan)
                return "";
            return name(parent) + ".";
        }();
        auto inst = mInsts.map.addOrGet(parentName + theName);
        mDeclInsts.addChild(decl, inst);
        if (!orphan)
            mDeclChildInsts.addChild(parent, inst);
        return inst;
    }

    Wire addOrGetWire(ModuleDecl decl, std::string name)
    {
        auto wire = mWires.map.addOrGet(mDecls.map.key(decl) + "." + name);
        if (mDeclWires.parent(wire) != decl)
            mDeclWires.addChild(decl, wire);
        return wire;
    }

    auto children(ModuleDecl decl) const
    {
        return mDeclChildInsts.children(decl);
    }

    ModuleDecl parent(ModuleInst inst) const
    {
        return mDeclChildInsts.parent(inst);
    }

    ModuleDecl decl(ModuleInst inst) const
    {
        return mDeclInsts.parent(inst);
    }

    const std::string& name(ModuleInst inst) const
    {
        return mInsts.map.key(inst);
    }

    const std::string& name(ModuleDecl decl) const
    {
        return mDecls.map.key(decl);
    }

    const std::string& name(Wire wire) const
    {
        return mWires.map.key(wire);
    }

    const std::string& name(InputPort port) const
    {
        return mInputs.map.key(port);
    }

    const std::string& name(OutputPort port) const
    {
        return mOutputs.map.key(port);
    }

    const std::string& name(boost::variant<InputPort, OutputPort> port) const
    {
        if (port.type() == typeid(InputPort))
            return name(boost::get<InputPort>(port));
        return name(boost::get<OutputPort>(port));
    }

    std::size_t childrenSize(ModuleDecl decl) const
    {
        return mDeclChildInsts.childrenSize(decl);
    }

    std::size_t inputPortsSize(ModuleDecl module) const
    {
        return mDeclInputs.childrenSize(module);
    }

    std::size_t outputPortsSize(ModuleDecl module) const
    {
        return mDeclOutputs.childrenSize(module);
    }

    std::size_t instsSize(ModuleDecl module) const
    {
        return mDeclInsts.childrenSize(module);
    }

    std::size_t wiresSize(ModuleDecl module) const
    {
        return mDeclWires.childrenSize(module);
    }

    std::size_t moduleDeclsSize() const
    {
        return mDecls.system.size();
    }

    auto inputPorts(ModuleDecl decl) const
    {
        return mDeclInputs.children(decl);
    }

    auto outputPorts(ModuleDecl decl) const
    {
        return mDeclOutputs.children(decl);
    }

    void topLevel(ModuleInst inst)
    {
        mTopLevel = inst;
        ranges::for_each(inputPorts(decl(inst)), [&](InputPort port)
        {
            auto portName = name(port);
            portName = portName.substr(portName.find_last_of(".") + 1);
            mapPort(inst, port, addOrGetWire(decl(inst), portName));
        });
        ranges::for_each(outputPorts(decl(inst)), [&](OutputPort port)
        {
            auto portName = name(port);
            portName = portName.substr(portName.find_last_of(".") + 1);
            mapPort(inst, port, addOrGetWire(decl(inst), portName));
        });
    }

    ModuleInst topLevel() const
    {
        return mTopLevel;
    }

    auto moduleDecls() const
    {
        return mDecls.system.asRange();
    }

    auto moduleInsts() const
    {
        return mInsts.system.asRange();
    }



    auto wires(ModuleDecl decl) const
    {
        return mDeclWires.children(decl);
    }

    auto insts(ModuleDecl decl) const
    {
        return mDeclInsts.children(decl);
    }

    template <typename PortType>
    MappedPort mapPort(ModuleInst inst, PortType port, Wire wire)
    {
        auto mapped = mMappedPort.add();
        mMappedPortsPorts[mapped] = port;
        if (mWiresMappedPorts.parent(mapped) != wire)
        {
            mWiresMappedPorts.addChild(wire, mapped);
        }
        if (inst != ModuleInst{} && mInstsMappedPorts.parent(mapped) != inst)
        {
            mInstsMappedPorts.addChild(inst, mapped);
        }
        return mapped;
    }

    boost::variant<InputPort, OutputPort> port(MappedPort port) const
    {
        return mMappedPortsPorts[port];
    }

    auto ports(Wire wire) const
    {
        return mWiresMappedPorts.children(wire);
    }

    auto ports(ModuleInst inst) const
    {
        return mInstsMappedPorts.children(inst);
    }

    Wire wire(MappedPort port) const
    {
        return mWiresMappedPorts.parent(port);
    }

    ModuleInst inst(MappedPort port) const
    {
        return mInstsMappedPorts.parent(port);
    }

private:
    MappedSystem<ModuleDecl> mDecls;
    MappedSystem<InputPort> mInputs;
    MappedSystem<OutputPort> mOutputs;
    MappedSystem<ModuleInst> mInsts;
    Entity::WeakAdapter<ModuleInst> mInstsWeak;
    MappedSystem<Wire> mWires;
    Entity::SystemWithDeletion<MappedPort> mMappedPort;
    Entity::WeakAdapter<MappedPort> mMappedPortWeak;


    decltype(Entity::makeComposition<Entity::Both>(mDecls.system, mWires.system)) mDeclWires;
    decltype(Entity::makeComposition<Entity::Both>(mDecls.system, mInstsWeak)) mDeclInsts;
    decltype(Entity::makeComposition<Entity::Both>(mDecls.system, mInstsWeak)) mDeclChildInsts;
    decltype(Entity::makeComposition<Entity::Both>(mDecls.system, mInputs.system)) mDeclInputs;
    decltype(Entity::makeComposition<Entity::Both>(mDecls.system, mOutputs.system)) mDeclOutputs;

    decltype(Entity::makeComposition<Entity::Both>(mInsts.system, mMappedPort)) mInstsMappedPorts;
    decltype(Entity::makeComposition<Entity::Both>(mWires.system, mMappedPortWeak)) mWiresMappedPorts;
    decltype(Entity::makeProperty<boost::variant<InputPort, OutputPort>>(mMappedPort)) mMappedPortsPorts;

    ModuleInst mTopLevel;


};

#endif // NETLIST_HPP
