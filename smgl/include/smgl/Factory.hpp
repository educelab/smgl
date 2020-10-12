#pragma once

/** @file */

#include <exception>

namespace smgl
{
namespace policy
{
/** @brief Default factory error policy */
template <class IdentifierType, class ProductType>
class DefaultFactoryError
{
public:
    /** @brief Default exception thrown when registered type not found */
    class unknown_identifier : public std::exception
    {
    public:
        /** Default constructor */
        explicit unknown_identifier(const IdentifierType& unknownId);

        /** Get exception message */
        const char* what() const noexcept override;

    private:
        /** Unknown identifier */
        IdentifierType unknownId_;
    };

protected:
    /**
     * Executes when unregistered identifier encountered
     *
     * @throws unknown_identifier
     */
    static ProductType OnUnknownType(const IdentifierType& id);

    /**
     * Executes when unregistered type-info encountered
     *
     * @throws unknown_identifier
     */
    static IdentifierType OnUnnamedType(const std::type_info& info);
};

}  // namespace policy

namespace detail
{
/**
 * @brief Abstract Factory class
 *
 * This factory uses RTTI to allow looking up an existing object's registered
 * type identifier.
 *
 * @tparam BaseClass Base type of object managed by factory
 * @tparam IdentifierType Identifier type (e.g. std::string)
 * @tparam AbstractProduct Product generated by factory creation method
 * @tparam ProductCreator Type of functor which produces AbstractProduct
 * @tparam FactoryErrorPolicy Policy for handling errors
 */
// clang-format off
template <
    class BaseClass,
    typename IdentifierType,
    class AbstractProduct = BaseClass*,
    typename ProductCreator = std::function<AbstractProduct()>,
template <typename, class> class FactoryErrorPolicy = policy::DefaultFactoryError
>
// clang-format on
class Factory : public FactoryErrorPolicy<IdentifierType, AbstractProduct>
{
public:
    /** Convenience alias for identifiers */
    using IDType = IdentifierType;

    /** Register identifier, type, and creation functor */
    bool Register(
        const IDType& id, ProductCreator creator, const std::type_info& info);

    /** Deregister type using identifier */
    bool Deregister(const IDType& id);

    /** Create AbstractProduct from registered identifier */
    AbstractProduct CreateObject(const IDType& id);

    /** Get a registered type's identifier */
    IDType GetTypeIdentifier(const std::type_info& info);

    /** Get a list of registered identifiers */
    std::vector<IDType> GetRegisteredIdentifiers() const;

private:
    /** Alias for std::type_info::hash() -> IdentifierType structure */
    using NameMap = std::unordered_map<size_t, IdentifierType>;
    /** Alias for IdentifierType -> ProduceCreator structure */
    using TypeMap = std::unordered_map<IdentifierType, ProductCreator>;
    /** Holds mappings from std::type_info::hash() -> IdentifierType */
    NameMap typeToIDMap_;
    /** Holds mappings from IdentifierType -> ProduceCreator */
    TypeMap idToCreatorMap_;
};

}  // namespace detail
}  // namespace smgl

#include "smgl/FactoryImpl.hpp"