#ifndef NODES_HPP
#define NODES_HPP

# include <string>
# include <sys/mman.h>
# include <sys/stat.h>
# include <stdlib.h>
# include <set>
# include <vector>
# include <map>
# include <utility>
# include <variant>
# include <type_traits>
# include <exception>
# include <iostream>
# include <stack>

# define CHAR2(x, y) (char[2]){x, y}

// How elements are ordered after the `key`
enum class YAP_Kinds
{
    Undefined,
    Null,
    Map,
    List,
    String,
    Integer,
    Float,
    // SingleMap,
    Boolean
};

enum class YAP_Specials
{
    Void,
    Anchor,
    Alias
};

struct State
{
    State(uint blocklevel, uint indentSize, bool isFlowStyle, char pBlockStyle[2])
        : blockLevel(blocklevel),
          indentationSize(indentSize),
          isFlowStyle(isFlowStyle)
    {
        this->blockStyle[0] = pBlockStyle[0];
        this->blockStyle[1] = pBlockStyle[1];
    }
    ~State() = default;

    unsigned int
    blockLevel;
    unsigned int
    indentationSize;

    bool
    isFlowStyle;
    char
    blockStyle[2];
};

struct Node
{
private:
    typedef std::vector<Node>               TList;
    typedef std::string                     TString;
    typedef int                             TInt;
    typedef float                           TFloat;
    typedef bool                            TBool;
    typedef std::map<std::string, Node>     TMap;
    // typedef std::pair<std::string, Node>    TSingleMap;

    typedef unsigned int    uint;
    typedef unsigned long   ulong;

    struct KeyIsRequired : std::exception { virtual const char* what(void) const noexcept override { return "Key is required"; } };
    struct FailedToOpenYMLFile : std::exception { virtual const char* what(void) const noexcept override { return "FailedToOpenYMLFile"; } };

    Node(YAP_Kinds kind) : kind_(kind), undefined(false), mustThrow(false)
    {
        if (kind == YAP_Kinds::Undefined)
            this->undefined = true;
    }

public:
    Node(const char* filename) : kind_(YAP_Kinds::Undefined), undefined(false), mustThrow(false)
    {
        FILE*   f = fopen(filename, "r");
        struct stat st;
        fstat(fileno(f), &st);
        unsigned long size = st.st_size;
        char*   data = (char*)mmap(nullptr, size, PROT_READ, MAP_SHARED, fileno(f), 0);
        fclose(f);
        if (data <= (char*)0)
            throw (FailedToOpenYMLFile());
        this->Parse(data);
        munmap(data, size);
    }

    Node(void) : kind_(YAP_Kinds::Undefined), undefined(false), mustThrow(false)
    {}
    ~Node() = default;

    Node&
    operator[](unsigned long index)
    {
        return this->at(index);
    }

    Node&
    operator[](const char key[])
    {
        return this->at(key);
    }

    Node&
    operator[](const std::string& key)
    {
        return this->at(key.data());
    }

    Node&
    require(const char key[])
    {
        return this->__require(key);
    }

    Node&
    require(const std::string& key)
    {
        return this->__require(key.data());
    }

    Node&
    required(const char key[])
    {
        return this->__required(key);
    }

    Node&
    required(const std::string& key)
    {
        return this->__required(key.data());
    }

    YAP_Kinds&
    kind(void)
    {
        return this->__kind();
    }

    const std::string&
    keyname(ulong index)
    {
        if (this->kind_ == YAP_Kinds::Map)
        {
            if (index >= this->Map().size())
                throw (std::runtime_error("Index too large for the map.size"));
            unsigned long     i = 0 ;
            auto    it = this->Map().begin();
            for (    ;
                    it != this->Map().end() && i < index ;
                    ++it, ++i) ;
            return it->first;
        }
        throw (std::runtime_error("Is not a map, can't have any keyname if there is no key :/"));
    }

    const std::string&
    keyname()
    {
        // if (this->kind_ == YAP_Kinds::SingleMap)
            // return this->SingleMap().first;
        if (this->kind_ == YAP_Kinds::Map && this->Map().size() == 1)
            return this->Map().begin()->first;
        throw (std::runtime_error("Is not a `single` map, maybe have a key but it's a map with many subkeys :/"));
    }

    operator TFloat&()
    {
        if (this->kind_ == YAP_Kinds::Float)
            return this->Float();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::Float)
            return this->__get_node_of_map();
        throw (std::runtime_error("Convert a non-float to a float is impossible"));
    }

    operator TInt&()
    {
        if (this->kind_ == YAP_Kinds::Integer)
            return this->Int();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::Integer)
            return this->__get_node_of_map();
        throw (std::runtime_error("Convert a non-int to a int is impossible"));
    }

    operator char*()
    {
        if (this->kind_ == YAP_Kinds::String)
            return this->String().data();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::String)
            return ((std::string&)this->__get_node_of_map()).data();
        throw (std::runtime_error("Convert a non-string to a string is impossible"));
    }

    operator TString&()
    {
        if (this->kind_ == YAP_Kinds::String)
            return this->String();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::String)
            return this->__get_node_of_map();
        throw (std::runtime_error("Convert a non-string to a string is impossible"));
    }

    operator TBool&()
    {
        if (this->kind_ == YAP_Kinds::Boolean)
            return this->Boolean();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::Boolean)
            return this->__get_node_of_map();
        return this->undefined; // return false if the object doesn't exist
    }

    operator TList&()
    {
        if (this->kind_ == YAP_Kinds::List)
            return this->List();
        else if (this->kind_ == YAP_Kinds::Map && this->__get_subkind_of_map() == YAP_Kinds::List)
            return this->__get_node_of_map();
        throw (std::runtime_error("Convert a non-list to a list is impossible"));
    }

    unsigned long
    size(void)
    {
        if (this->kind_ == YAP_Kinds::List)
            return this->List().size();
        else if (this->kind_ == YAP_Kinds::Map)
            return this->Map().size();
        throw (std::runtime_error("String or Boolean doesn't have any size() system"));
    }

    Node&
    return_or_throw(const char* key)
    {
        if (this->mustThrow)
            throw (std::runtime_error("Key not existing"));
        return this->Map().emplace(key, Node(YAP_Kinds::Undefined)).first->second;
    }

    // Only for attempted non-existing key
    void
    active_throw_exception(void)
    {
        this->mustThrow = true;
    }

    // Only attempted for non-existing key
    void
    deactive_throw_exception(void)
    {
        this->mustThrow = false;
    }

private:
    void
    Parse(const char* data)
    {
        ulong   i = 0;
        // State   state(CountIndentation(data, i), false, CHAR2('\0', '\0'));

        while (data[i])
        {
            // this->states.emplace(state);
        }
    }

    uint
    CountIndentation(const char* data, ulong& i)
    {
        uint    indent;
        for ( ; true ; )
        {
            indent = 0;
            for ( ; data[i] && data[i] == ' ' ; ++i, ++indent) ;
            if (data[i] == '\n')
                ++i;
        }
        return indent;
    }

    YAP_Kinds&
    __get_subkind_of_map(void)
    {
        if (this->kind_ == YAP_Kinds::Map &&
            this->Map().size() == 1)
            return this->Map().begin()->second.kind_;
        throw (std::runtime_error("quoicoubeh kind"));
    }

    Node&
    __get_node_of_map(void)
    {
        if (this->kind_ == YAP_Kinds::Map &&
            this->Map().size() == 1)
            return this->Map().begin()->second;
        throw (std::runtime_error("quoicoubeh node"));
    }

    Node&
    __required(const char key[])
    {
        *this = this->require(key);
        return *this;
    }

    Node&
    __require(const char key[])
    {
        if (this->kind_ != YAP_Kinds::Map)
            throw (KeyIsRequired());
        if (this->kind_ == YAP_Kinds::Map)
        {
            if (this->Map().contains(key) == false)
                throw (KeyIsRequired());
            return this->Map()[key];
        }
        throw (std::runtime_error("Is not a map, you can't access by a key value"));
    }

    YAP_Kinds&
    __kind(void)
    {
        return this->kind_;
    }




    Node&
    at(const char key[])
    {
        if (this->kind_ == YAP_Kinds::Map)
        {
            if (this->Map().contains(key) == false)
                return this->return_or_throw(key);
            return this->Map()[key];
        }
        throw (std::runtime_error("Is not a map, you can't access by a key value"));
    }

    Node&
    at(unsigned long index)
    {
        if (this->kind_ == YAP_Kinds::List)
            return this->List()[index];
        else if (this->kind_ == YAP_Kinds::Map)
        {
            if (index >= this->Map().size())
                throw (std::runtime_error("Index too large for the map.size"));
            unsigned long     i = 0 ;
            auto    it = this->Map().begin();
            for (    ;
                    it != this->Map().end() && i < index ;
                    ++it, ++i) ;
            return it->second;
        }
        throw (std::runtime_error("Is not a list/map, you can't access by a int index"));
    }

    TFloat&
    Float()
    {
        return std::get<TFloat>(this->_union);
    }

    TInt&
    Int()
    {
        return std::get<TInt>(this->_union);
    }

    TString&
    String()
    {
        return std::get<TString>(this->_union);
    }

    TBool&
    Boolean()
    {
        return std::get<TBool>(this->_union);
    }

    TMap&
    Map()
    {
        return std::get<TMap>(this->_union);
    }

    TList&
    List()
    {
        return std::get<TList>(this->_union);
    }

    // TSingleMap&
    // SingleMap()
    // {
    //     return std::get<TSingleMap>(this->_union);
    // }

    std::map<std::string, Node*>
    anchors;

    std::stack<State>
    states;

    YAP_Specials
    special;

    std::variant<
        TList,
        TString,
        TInt,
        TFloat,
        TBool,
        // TSingleMap,
        TMap
    >
    _union;

    YAP_Kinds
    kind_;

    bool
    undefined;

    bool
    mustThrow;
};

typedef Node    Yap;

#endif
