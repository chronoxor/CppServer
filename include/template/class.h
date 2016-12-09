/*!
    \file class.h
    \brief Template class definition
    \author Ivan Shynkarenka
    \date 26.05.2016
    \copyright MIT License
*/

#ifndef CPPTEMPLATE_TEMPLATE_CLASS_H
#define CPPTEMPLATE_TEMPLATE_CLASS_H

namespace CppTemplate {

//! Template class
/*!
    Template class details.

    Thread-safe.
*/
class Template
{
public:
    //! Default class constructor
    /*!
        \param filed - Filed value
    */
    explicit Template(int filed) noexcept : _field(filed) {}
    Template(const Template&) noexcept = default;
    Template(Template&&) noexcept = default;
    ~Template() noexcept = default;

    Template& operator=(const Template&) noexcept = default;
    Template& operator=(Template&&) noexcept = default;

    //! Get field value
    int field() const noexcept { return _field; }

    //! Some method
    /*!
        \param parameter - Method parameter
        \return Method result
    */
    int Method(int parameter) const noexcept;

    //! Some static method
    /*!
        \param parameter - Static method parameter
        \return Static method result
    */
    static int StaticMethod(int parameter) noexcept;

private:
    int _field;
};

/*! \example template_class.cpp Template class example */

} // namespace CppTemplate

#include "class.inl"

#endif // CPPTEMPLATE_TEMPLATE_CLASS_H
