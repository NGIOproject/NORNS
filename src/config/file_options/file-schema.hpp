/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

/*
 * This file implements the classes necessary to describe the structure 
 * of a configuration file, as well as the options that are accepted and
 * how to process them when found.
 *
 * A typical configuration file can be defined as follows:
 *
 * const file_schema config_schema = declare_file({
 *      declare_section(
 *          "section0",             // section name
 *          true,                   // section is mandatory
 *          declare_group({         // define a group of related options
 *              declare_option<
 *                  uint32_t        // type for option value to be converted to
 *              >(
 *                  "option0",      // option name in file
 *                  true,           // mandatory
 *                  8,              // default value (optional)
 *                  number_parser   // converter function 
 *                                  //   std::string -> uint32_t (optional)
 *              ),
 *              {{ other option definitions... }}
 *          })
 *      ),
 *      declare_section(
 *          "section1",             // section name
 *          true,                   // section is mandatory
 *          declare_list({          // define a list of options that can appear
 *                                  // several times
 *              declare_option<
 *                  std::string     // type for option value to be converted to
 *              >(
 *                  "option1",      // option name in file
 *                  true            // mandatory
 *              ),
 *              declare_option<
 *                  std::string     // type for option value to be converted to
 *              >(
 *                  "option2",      // option name in file
 *                  false           // mandatory
 *              )
 *          })
 *      ),
 *      {{ other section definitions... }}
 * });
 *
 */

#ifndef __FILE_SCHEMA_HPP__
#define __FILE_SCHEMA_HPP__

#include <yaml-cpp/yaml.h>
#include <boost/variant.hpp>
#include <boost/any.hpp>

//#include "utils.h"

namespace file_options {

/*! Special option name that can be used to accept any non-defined
 * options in list_schema definitions */
static const std::string match_any = ".*";

// forward declarations
struct section_schema;
struct option_schema;

enum class opt_type : bool {
    optional,
    mandatory,
};

enum class sec_type : bool {
    optional,
    mandatory,
};

enum class grp_type : bool {
    optional,
    mandatory,
};


/*! Class to represent a file organization as a key-value map 
 * of section definitions, where file_schema[section_name] returns
 * the parsed section contents.
 *
 * Since we are implicitly using std::map, all its typical 
 * functions can be used */
using file_schema = std::map<std::string, section_schema>;


/*! Class to handle a group of related options as a key-value map 
 * where group_schema[option_name] returns the parsed (and converted) 
 * option_value
 * Since we are implicitly using std::map, all its typical 
 * functions can be used */
using group_schema = std::map<std::string, option_schema>;


/*! Class to represent a list of options that can be arbitrarily 
 * repeated in a section.
 *
 * Since we inherit from group_schema, which is implicitly using 
 * std::map, all its typical functions can be used */
struct list_schema : public group_schema {
    // inherit constructors from group_schema
    using group_schema::group_schema;
};

/*! Abstract base class to handle the semantics of a configuration option.
 * By deriving specialized classes from this base class, we can provide
 * a simple interface for the user to define an option (see typed_value<T>). */
struct option_type {
    /*! Returns true if a default value was provided for this option when constructed */
    virtual bool has_default_value() const = 0;

    /*! Returns true if a converter function was provided for this option when constructed */
    virtual bool has_converter() const = 0;

    /*! Returns the default value of this function if it was provided 
     * when constructed. If it was not provided, the return value is 
     * undefined. */
    virtual boost::any default_value() const = 0;

    /*! Conversion function from std::string to the type defined for the option */
    virtual boost::any convert(const std::string& key, const std::string& value) const = 0;

    /*! Destructor to avoid leaking */
    virtual ~option_type() {}
};

/*! Type definition for conversion function */
template <typename T>
using ParserFun = std::function<T(const std::string&, const std::string&)>;

/*! Public helper trait for conversion function type */
template <typename T>
using converter = ParserFun<T>;

/*! Concrete class to handle the semantics of a configuration option 
 * of type T. It stores the default value of type T provided by the user,
 * as well as a specialized conversion function std::string -> T to be used,
 * when parsing the option's value. */
template <typename T>
struct typed_value : public option_type {

    /*! Constructor for an option without default value 
     * or conversion function */
    typed_value() 
        : m_has_default_value(false),
          m_has_converter(false) { }

    /*! Constructor for an option with default value 
     * and no conversion function */
    explicit typed_value(const T default_value) 
        : m_has_default_value(true),
          m_default_value(default_value),
          m_has_converter(false) { }

    /*! Constructor for an option with a conversion 
     * function and no default value */
    explicit typed_value(const ParserFun<T>& converter)
        : m_has_default_value(false),
          m_has_converter(true),
          m_converter(converter) { }

    /*! Constructor for an option with a default value 
     * and a conversion function */
    typed_value(const T default_value, const ParserFun<T>& converter) 
        : m_has_default_value(true),
          m_default_value(default_value),
          m_has_converter(true),
          m_converter(converter) { }

    /*! Returns true if a default value was provided for this option when constructed */
    bool has_default_value() const override {
        return m_has_default_value;
    }

    /*! Returns true if a converter function was provided for this option when constructed */
    bool has_converter() const override {
        return m_has_converter;
    }

    /*! Returns the default value of this function if it was provided 
     * when constructed. If it was not provided, the return value is 
     * undefined. */
    boost::any default_value() const override {
        return m_default_value;
    }

    /*! If a converter function has been provided for this option, the 
     * function is applied to 'value' and the result (hopefully of type T) is 
     * returned to the caller. If no function is provided, an undefined value
     * is returned, given that we don't know how to transform std::string to T.
     * If the conversion function fails, the exception is propagated to the user */
    boost::any convert(const std::string& key, const std::string& value) const override {

        if(m_has_converter) {
            return m_converter(key, value);
        }

        return {};
    }

    bool m_has_default_value;   /*!< True if a default value was provided */
    T m_default_value;          /*!< Default value provided by the user */
    bool m_has_converter;       /*!< True if a conversion function was provided */
    ParserFun<T> m_converter;   /*!< Conversion fucntion provided by the user */
};

/*! Creates a typed_value<T> instance. This function is the primary method
 * to create a typed_value instance for a specific type, which can later be
 * passed to the group_schema and list_schema constructors.
 *
 * Overload without default value or conversion function */
template <typename T>
std::shared_ptr<typed_value<T>> type() {
    return std::make_shared<typed_value<T>>();
}

/*! Creates a typed_value<T> instance. This function is the primary method
 * to create a typed_value instance for a specific type, which can later be
 * passed to the group_schema and list_schema constructors.
 *
 * Overload with default value only */
template <typename T>
std::shared_ptr<typed_value<T>> type(const T& default_value) {
    return std::make_shared<typed_value<T>>(default_value);
}

/*! Creates a typed_value<T> instance. This function is the primary method
 * to create a typed_value instance for a specific type, which can later be
 * passed to the group_schema and list_schema constructors.
 *
 * Overload with conversion function only */
template <typename T>
std::shared_ptr<typed_value<T>> type(const ParserFun<T>& converter) {
    return std::make_shared<typed_value<T>>(converter);
}

/*! Creates a typed_value<T> instance. This function is the primary method
 * to create a typed_value instance for a specific type, which can later be
 * passed to the group_schema and list_schema constructors.
 *
 * Overload with default value and conversion function */
template <typename T>
std::shared_ptr<typed_value<T>> type(const T& default_value, const ParserFun<T>& converter) {
    return std::make_shared<typed_value<T>>(default_value, converter);
}

/*! Class used to handle the description of a section */
struct section_schema {

    using SchemaType = boost::variant<group_schema, list_schema>;

    section_schema(bool mandatory, SchemaType&& schema)
        : m_mandatory(mandatory),
          m_schema(schema) {}

    bool is_mandatory() const {
        return m_mandatory;
    }

    bool m_mandatory;
    SchemaType m_schema;
};

/*! Class used to handle the description of an option */
struct option_schema {

    /*! Creates an instance of option_schema and stores whether
     * the option is mandatory or not, and also its type */
    option_schema(bool mandatory, std::shared_ptr<option_type> type)
        : m_mandatory(mandatory),
          m_typed_value(type) {}

    /*! Returns true if the option has been defined as mandatory */
    bool is_mandatory() const {
        return m_mandatory;
    }

    /*! Returns true if a default value has been defined for the option */
    bool has_default_value() const {
        return m_typed_value->has_default_value();
    }

    /*! Returns the default value defined for m_typed_value, if any.
     * Otherwise, the return value is undefined */
    boost::any default_value() const {
        return m_typed_value->default_value();
    }
    
    /*! Returns true if a converter function has been defined for m_typed_value */
    bool has_converter() const {
        return m_typed_value->has_converter();
    }

    /*! Calls the converter function defined for m_typed_value, if any. 
     * Otherwise, the result is undefined */
    boost::any convert(const std::string& name, const std::string& value) const {
        return m_typed_value->convert(name, value);
    }

    bool m_mandatory;                           /*!< True if the option is mandatory */
    std::shared_ptr<option_type> m_typed_value; /*!< Specific semantics for this option */
};

// some helpers to simplify the syntax of a file declaration

using OptionSchemaType = std::pair<const std::string, option_schema>;

template <typename T>
OptionSchemaType 
declare_option(const std::string& name, opt_type ot) {
    return {name, {static_cast<bool>(ot), type<T>()}};
}

template <typename T>
OptionSchemaType 
declare_option(const std::string& name, opt_type ot, const T& default_value) {
    return {name, {static_cast<bool>(ot), type<T>(default_value)}};
}

template <typename T>
OptionSchemaType 
declare_option(const std::string& name, opt_type ot, 
               const ParserFun<T>& parser) {
    return {name, {static_cast<bool>(ot), type<T>(parser)}};
}

template <typename T>
OptionSchemaType 
declare_option(const std::string& name, opt_type ot, const T& default_value, 
               const ParserFun<T>& parser) {
    return {name, {static_cast<bool>(ot), type<T>(default_value, parser)}};
}

list_schema 
declare_list(const std::initializer_list<OptionSchemaType>&& args) {
    return {args};
}

group_schema 
declare_group(const std::initializer_list<OptionSchemaType>&& args) {
    return {args};
}

using SectionSchemaType = std::pair<const std::string, section_schema>;

SectionSchemaType 
declare_section(const std::string& name, sec_type st, 
                const group_schema&& schema) {
    return {name, {static_cast<bool>(st), {schema}}};
}

SectionSchemaType 
declare_section(const std::string& name, sec_type st, 
                const list_schema&& schema) {
    return {name, {static_cast<bool>(st), {schema}}};
}

/*! Declares a file_schema as a list of section_schema definitions */
file_schema 
declare_file(const std::initializer_list<SectionSchemaType>&& args) {
    return {args};
}

} // namespace file_options

#endif /* __FILE_SCHEMA_HPP__ */
