#pragma once

#include <Se/IO/Serializer.hpp>
#include <Se/IO/Deserializer.hpp>

#include <memory>

namespace Se
{

/// A common root class for objects that implement both Serializer and Deserializer.
class AbstractFile : public Deserializer, public Serializer
{
public:
    /// Construct.
    AbstractFile() : Deserializer() { }
    /// Construct.
    explicit AbstractFile(unsigned int size) : Deserializer(size) { }
    /// Destruct.
    ~AbstractFile() override = default;
    /// Change the file name. Used by the resource system.
    /// @property
    virtual void SetName(const String& name) { name_ = name; }
    /// Return whether is open.
    /// @property
    virtual bool IsOpen() const { return true; }
    /// Return absolute file name in file system.
    /// @property
    virtual const String& GetAbsoluteName() const { return name_; }
    /// Close the file.
    virtual void Close() {}

#ifndef SWIG
    // A workaround for SWIG failing to generate bindings because both IAbstractFile and IDeserializer provide GetName() method. This is
    // fine because IAbstractFile inherits GetName() from IDeserializer anyway.

    /// Return the file name.
    String GetName() const override { return name_; }
#endif

protected:
    /// File name.
    String name_;
};

using AbstractFilePtr = std::shared_ptr<AbstractFile>;

}