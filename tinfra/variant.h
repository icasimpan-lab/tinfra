//
// Copyright (c) 2013, Zbigniew Zagorski
// This software licensed under terms described in LICENSE.txt
//

#ifndef tinfra_variant_h_included
#define tinfra_variant_h_included

#include "any.h"
#include <string>
#include <map>
#include <vector>
#include <iosfwd>

namespace tinfra {

class variant {
    tinfra::any value;
    
public:
    struct none_type {};
    typedef std::string                 key_type;
    typedef std::string                 index_type;
    
    typedef std::map<key_type, variant> dict_type;
    typedef std::vector<variant>        array_type;
    typedef std::string                 string_type;
    
public:
    variant(): value(tinfra::any::from_copy(none_type())) {} // TBD, fix it!
    variant(int v): value(tinfra::any::from_copy(v)) {}
    variant(string_type const& v): value(tinfra::any::from_copy(v)) {}
    
    variant(tinfra::any const& v): value(v) {}
    
    static variant none();
    static variant array();
    static variant dict();
    
    // type checkers
    
    bool is_array() const;
    bool is_dict() const;
    bool is_string() const;
    bool is_int() const;
    bool is_double() const;
    bool is_bool() const;
    bool is_none() const;
    
    // cast getters
    dict_type&       get_dict();
    dict_type const& get_dict() const;
    
    array_type&       get_array();
    array_type const& get_array() const;
    
    string_type&       get_string();
    string_type const& get_string() const;
    
    int&       get_int();
    int const& get_int() const;
    
    double&       get_double();
    double const& get_double() const;

    bool&       get_bool();
    bool const& get_bool() const;
    
    // forcing setters getters
    void set_dict(dict_type const& = dict_type());
    void set_array(array_type const& = array_type());
    void set_string(string_type const&);
    
    void set_int(int);
    void set_double(double);
    void set_bool(bool);
    
    // general query
    // valid for dicts, arrays and strings 
    size_t size() const;
    
    // dict query
    variant& operator[](key_type const& key);
    variant const& operator[](key_type const& key) const;
    std::vector<key_type> dict_keys() const;
    
    // array query
    variant& operator[](int index);
    variant const& operator[](int index) const;
};

inline
variant variant::none()
{
    return variant(tinfra::any::from_copy(variant::none_type()));
}
inline
variant variant::array()
{
    return variant(tinfra::any::from_copy(variant::array_type()));
}

inline
variant variant::dict()
{
    return variant(tinfra::any::from_copy(variant::dict_type()));
}

inline bool variant::is_none() const {
    return this->value.type() == typeid(none_type);
}

inline bool variant::is_array() const {
    return this->value.type() == typeid(array_type);
}

inline bool variant::is_dict() const
{
    return this->value.type() == typeid(dict_type);
}

inline bool variant::is_string() const
{
    return this->value.type() == typeid(string_type);
}

inline bool variant::is_int() const
{
    return this->value.type() == typeid(int);
}

inline bool variant::is_double() const
{
    return this->value.type() == typeid(double);
}

inline bool variant::is_bool() const
{
    return this->value.type() == typeid(bool);
}

inline variant::dict_type&       variant::get_dict() {
    TINFRA_ASSERT(is_dict());
    return this->value.get<dict_type>();
}
inline variant::dict_type const& variant::get_dict() const {
    TINFRA_ASSERT(is_dict());
    return this->value.get<dict_type>();
}

inline variant::array_type&       variant::get_array() {
    TINFRA_ASSERT(is_array());
    return this->value.get<array_type>();
}
inline variant::array_type const& variant::get_array() const {
    TINFRA_ASSERT(is_array());
    return this->value.get<array_type>();
}

inline variant::string_type&       variant::get_string() {
    TINFRA_ASSERT(is_string());
    return this->value.get<string_type>();
}
inline variant::string_type const& variant::get_string() const {
    TINFRA_ASSERT(is_string());
    return this->value.get<string_type>();
}

inline int&       variant::get_int() {
    TINFRA_ASSERT(is_int());
    return this->value.get<int>();
}
inline int const& variant::get_int() const {
    TINFRA_ASSERT(is_int());
    return this->value.get<int>();
}

inline double&       variant::get_double() {
    TINFRA_ASSERT(is_double());
    return this->value.get<double>();
}
inline double const& variant::get_double() const {
    TINFRA_ASSERT(is_double());
    return this->value.get<double>();
}

inline bool&       variant::get_bool() {
    TINFRA_ASSERT(is_bool());
    return this->value.get<bool>();
}
inline bool const& variant::get_bool() const {
    TINFRA_ASSERT(is_bool());
    return this->value.get<bool>();
}


inline void variant::set_dict(dict_type const& v) {
    this->value = tinfra::any::from_copy(v);
}

inline void variant::set_array(array_type const& v) {
    this->value = tinfra::any::from_copy(v);
}

inline void variant::set_string(string_type const& v) {
    this->value = tinfra::any::from_copy(v);
}
inline void variant::set_int(int v) {
    this->value = tinfra::any::from_copy(v);
}

inline void variant::set_double(double v) {
    this->value = tinfra::any::from_copy(v);
}
inline void variant::set_bool(bool v) {
    this->value = tinfra::any::from_copy(v);
}

// operator == 
// TBD, bools ints etc
inline bool operator==(variant const& n, std::string const& s) {
    return n.is_string() && n.get_string() == s;
}

inline bool operator==(variant const& n, const char* s)
{
    return n.is_string() && n.get_string() == s;
}

inline bool operator==(std::string const& s, variant const& n) {
    return n.is_string() && n.get_string() == s;
}

inline bool operator==(const char* s, variant const& n)
{
    return n.is_string() && n.get_string() == s;
}

std::ostream& operator <<(std::ostream& s, variant const& node);

} // end namespace tinfra

#endif
