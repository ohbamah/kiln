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
# include <cstring>
# include "exception_format.hpp"

# define CHAR2(x, y) (char[2]){x, y}

inline int to_int(const std::strong_ordering& so)
{
    if (so == std::strong_ordering::greater)
        return 1;
    if (so == std::strong_ordering::less)
        return -1;
    return 0;
}

// How elements are ordered after the `key`
enum class Kinds
{
    Undefined,
    Null,
    Map,
    List,
    String,
    Integer,
    Float,
    Boolean
};

struct LineMetrics
{
    LineMetrics() : is_comment(false), blockLevel(0), indentationSize(0)
    {}
    LineMetrics(bool is_comment, unsigned int blockLevel = 0, unsigned int indentationSize = 0) : is_comment(is_comment), blockLevel(blockLevel), indentationSize(indentationSize)
    {}
    ~LineMetrics() = default;

    const char*
    data;
    unsigned int
    size;
    bool
    is_comment;
    unsigned int
    blockLevel;
    unsigned int
    indentationSize;
};

struct MapArg
{
    MapArg() : data(""), size(0)
    {}
    MapArg(const char* data, uint size) : data(data), size(size)
    {}
    ~MapArg() = default;

    bool
    is_empty(void)
    {
        return this->data.empty();
    }

    bool
    is_anchor(void)
    {
        return this->is_anchor_;
    }

    std::string
    data;
    uint
    size;
    bool
    is_anchor_;
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

    typedef unsigned int    uint;
    typedef unsigned long   ulong;

public:
    DECL_EXCEPTION_FORMAT(KeyIsRequired, "Key is required")
    DECL_EXCEPTION_FORMAT(YmlFileError, "Failed to open a YML File")
    DECL_EXCEPTION_FORMAT(YmlEmptyFile, "The file passed is empty")

    DECL_EXCEPTION_FORMAT(BadYamlMapFormat, "BadYamlMapFormat");

    DECL_EXCEPTION_FORMAT(IsNotYamlMap, "Is not a Yaml Map")
    DECL_EXCEPTION_FORMAT(IsNotYamlList, "Is not a Yaml List")
    DECL_EXCEPTION_FORMAT(IsNotYamlString, "Is not a Yaml String")
    DECL_EXCEPTION_FORMAT(IsNotYamlBoolean, "Is not a Yaml Boolean")
    DECL_EXCEPTION_FORMAT(IsNotYamlNull, "Is not a Yaml Null")
    DECL_EXCEPTION_FORMAT(IsNotYamlFloat, "Is not a Yaml Float")
    DECL_EXCEPTION_FORMAT(IsNotYamlInteger, "Is not a Yaml Integer")

    DECL_EXCEPTION_FORMAT(ConversionFailed, "Conversion failed -- Bad Yaml type")

    DECL_EXCEPTION_FORMAT(IndexNoReachable, "Index no reachable")

    DECL_EXCEPTION_FORMAT(WtfKindDoesNotExist, "??? Kind does not exist, what are you currently doing bro ? Godly damn shit");

private:
    Node(Kinds kind) : kind_(kind), not_undefined(true), mustThrow(false)
    {
        if (kind == Kinds::Undefined || kind == Kinds::Null)
            this->not_undefined = false;
    }

public:
    Node(const char* filename) : kind_(Kinds::Undefined), not_undefined(true), mustThrow(false)
    {
        FILE*   f = fopen(filename, "r");
        if (f == nullptr)
            throw (YmlFileError("Failed to fopen %s", filename));
        struct stat st;
        fstat(fileno(f), &st);
        unsigned long size = st.st_size;
        if (size == 0)
            throw (YmlEmptyFile("File %s is empty", filename));
        char*   data = (char*)mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fileno(f), 0);
        fclose(f);
        if (data <= (char*)0)
            throw (YmlFileError("Failed to mmap %s", filename));
        this->Process(data);
        munmap(data, size);
    }

    Node(void) : kind_(Kinds::Undefined), not_undefined(true), mustThrow(false)
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

    // Will always throw if the map doesn't contain the `key`
    Node&
    require(const char key[])
    {
        return this->__require(key);
    }

    // Will always throw if the map doesn't contain the `key`
    Node&
    require(const std::string& key)
    {
        return this->__require(key.data());
    }

    // Like require with auto assignation
    // yml.required("ya") --> yml = yml.require("ya")
    // Will always throw if the map doesn't contain the `key`
    Node&
    required(const char key[])
    {
        return this->__required(key);
    }

    // Like require with auto assignation
    // yml.required("ya") --> yml = yml.require("ya")
    // Will always throw if the map doesn't contain the `key`
    Node&
    required(const std::string& key)
    {
        return this->__required(key.data());
    }

    // The current kind of the element
    Kinds&
    kind(void)
    {
        return this->__kind();
    }

    // If it's currently a map, and then try to get keyname of an embedded map inside
    const std::string&
    keyname(ulong index)
    {
        if (this->kind_ == Kinds::Map)
        {
            if (index >= this->Map().size())
                throw (IndexNoReachable("Because index %ld is out of bounds (size = %ld)", index, this->size()));
            unsigned long     i = 0 ;
            auto    it = this->Map().begin();
            for (    ;
                    it != this->Map().end() && i < index ;
                    ++it, ++i) ;
            return it->first;
        }
        throw (IndexNoReachable("Index %ld no reachable %s", index, this->kind_ != Kinds::Map ? "because it is not a map" : "because it is out of bounds"));
    }

    // The current keyname of the map
    // `key: String value` --> it will returns "key"
    const std::string&
    keyname()
    {
        // if (!this->state.parentKeyname.empty())
        //     return this->state.parentKeyname;
        throw (std::runtime_error("Is not a `single` map, maybe have a key but it's a map with many subkeys :/"));
    }

    operator TFloat&()
    {
        if (this->kind_ == Kinds::Float)
            return this->Float();
        throw (std::runtime_error("Convert a non-float to a float is impossible"));
    }

    operator TInt&()
    {
        if (this->kind_ == Kinds::Integer)
            return this->Int();
        throw (std::runtime_error("Convert a non-int to a int is impossible"));
    }

    operator TString&()
    {
        if (this->kind_ == Kinds::String)
            return this->String();
        throw (std::runtime_error("Convert a non-string to a string is impossible"));
    }

    operator TBool&()
    {
        if (this->kind_ == Kinds::Boolean)
            return this->Boolean();
        return this->not_undefined; // return false if the object is Undefined OR if it's Null
    }

    operator TList&()
    {
        if (this->kind_ == Kinds::List)
            return this->List();
        throw (std::runtime_error("Convert a non-list to a list is impossible"));
    }

    unsigned long
    size(void)
    {
        if (this->kind_ == Kinds::List)
            return this->List().size();
        else if (this->kind_ == Kinds::Map)
            return this->Map().size();
        throw (std::runtime_error("String or Boolean doesn't have any size() system"));
    }

    Node&
    return_or_throw(const char* key)
    {
        if (this->mustThrow)
            throw (std::runtime_error("Key not existing"));
        return this->Map().emplace(key, Node(Kinds::Undefined)).first->second;
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
    Process(char* data)
    {
        this->collectLinesMetrics(data);
        // this->DebugLinesMetrics();
        this->StartParsing();
    }

    void
    StartParsing(void)
    {
        uint i = 0;
        this->Parse(i, 0, MapArg());
    }

    void
    Parse(uint& i, uint blockLevel, MapArg mapArg)
    {
        (void)mapArg;
        // if (!mapArg.is_empty())
        // {
        //     InterpretMapArg(mapArg);
        //     if (!mapArg.is_anchor())
        //         return ;
        // }
        while (i < Node::lines.size())
        {
            std::string keyname;
            SkipComments(i);
            if (i >= Node::lines.size())
                break ;

            // std::cout << "parse ? i=" << i << " blockLevel=" << blockLevel << " currentBlockLevel=" << Node::lines[i].blockLevel << std::endl;
            if (Node::lines[i].blockLevel < blockLevel)
                break ;

            FetchKeyname(keyname, i, 0);
            if (!keyname.empty())
                HandleMap(i, keyname, blockLevel);
            else if (IsList(Node::lines[i].data))
                HandleList(i, blockLevel);
            else
                HandleString(i);
        }
    }

    bool
    IsList(const char* data)
    {
        return data[0] == '-' && data[1] == ' ';
    }

    void
    InterpretMapArg(MapArg mapArg)
    {
        if (mapArg.is_anchor())
            Node::anchors.emplace(mapArg.data, this);
        else
        {
            // WTF
            std::cout << "maparg: \"" << mapArg.data << "\"" << std::endl;
            if (mapArg.data == "true")
            {
                this->kind_ = Kinds::Boolean;
                this->SetBoolean(true);
            }
            else if (mapArg.data == "false")
            {
                this->kind_ = Kinds::Boolean;
                this->SetBoolean(false);
            }
            else
            {
                this->kind_ = Kinds::String;
                this->SetString(mapArg.data);
            }
        }
    }

    void
    HandleMap(uint& i, std::string& keyname, uint currentBlockLevel)
    {
        // std::cout << "HandleMap: keyname='" << keyname << "' i=" << i << std::endl;
        this->kind_ = Kinds::Map;

        MapArg map_arg = this->ParseMapArg(Node::lines[i].data, Node::lines[i].size, keyname.size() + 1);
        ++i;
        // std::cout << "HandleMap: after ++i, i=" << i << " size=" << Node::lines.size() << std::endl;

        Node child;
        child.parentKeyname = keyname;
        // Traiter la valeur sur la même ligne (si elle existe)
        if (!map_arg.is_empty()) {
            // std::cout << "HandleMap: setting string value: '" << map_arg.data << "'" << std::endl;
            // child.SetString(map_arg.data);
            // child.kind_ = Kinds::String;
            if (map_arg.data == "true")
            {
                child.kind_ = Kinds::Boolean;
                child.SetBoolean(true);
            }
            else if (map_arg.data == "false")
            {
                child.kind_ = Kinds::Boolean;
                child.SetBoolean(false);
            }
            else
            {
                child.kind_ = Kinds::String;
                child.SetString(map_arg.data);
            }
        }
        // Sinon, parser les enfants (si ils existent)
        else if (i < Node::lines.size() && Node::lines[i].blockLevel > currentBlockLevel) {
            // std::cout << "HandleMap: calling child.Parse() with blockLevel " << Node::lines[i].blockLevel << std::endl;
            child.Parse(i, Node::lines[i].blockLevel, MapArg());
        } else {
            // std::cout << "HandleMap: no value and no children (empty node)" << std::endl;
        }
        
        // std::cout << "HandleMap: calling AddMap()" << std::endl;
        this->AddMap(keyname, child);
        // std::cout << "HandleMap: done" << std::endl;
    }

    void
    HandleString(uint& i)
    {
        this->kind_ = Kinds::String;

        ++i;
    }

    void
    HandleList(uint& i, uint currentBlockLevel)
    {
        this->kind_ = Kinds::List;

        if (i < Node::lines.size() && IsList(Node::lines[i].data) && Node::lines[i].blockLevel == currentBlockLevel)
        {
            while (i < Node::lines.size() && IsList(Node::lines[i].data) && Node::lines[i].blockLevel == currentBlockLevel)
            {
                // std::cout << "HandleList: processing item at line " << i << std::endl;

                // Extract the list item value (after "- ")
                const char* data = Node::lines[i].data;
                uint size = Node::lines[i].size;

                Node child;
                if (size > 2) {
                    // Parse the value after "- "
                    MapArg item_arg = this->ParseMapArg(data, size, 2); // start at position 2 (after "- ")
                    if (!item_arg.is_empty()) {
                        child.SetString(item_arg.data);
                        child.kind_ = Kinds::String;
                        // std::cout << "HandleList: added list item '" << item_arg.data << "'" << std::endl;
                    } else {
                        std::string keyname;
                        this->FetchKeyname(keyname, i, 2);
                        if (keyname.empty())
                            throw (std::exception()); // void list
                        Node    subchild;
                        subchild.Parse(i, Node::lines[i].blockLevel, MapArg());
                        child.AddMap(keyname, subchild);
                    }
                }

                this->AddList(child);
                ++i;  // Simple increment, no recursive Parse()
            }
        }
        else
            ++i;
    }

    void
    PasteAlias(uint& i)
    {
        (void)i;
    }

    MapArg
    ParseMapArg(const char* data, uint size, ulong arg_pos)
    {
        // std::cout << "ParseMapArg: data='" << std::string(data, size) << "' arg_pos=" << arg_pos << " size=" << size << std::endl;
        char        quote = 0;
        bool        in_quote = false;
        // bool        is_alias = false;
        MapArg      map_arg;
        uint        i = 0;

        this->SkipSpaces(data, arg_pos);
        // std::cout << "ParseMapArg: after SkipSpaces, arg_pos=" << arg_pos << std::endl;
        if (data[arg_pos] == '&')
            map_arg.is_anchor_ = true;
        // else if (data[arg_pos] == '*')
            // is_alias = true;
        for (; arg_pos < size ; ++arg_pos, ++i)
        {
            if (data[arg_pos] == '\n')
            {
                if (map_arg.data[map_arg.data.size() - 1] == ' ')
                    map_arg.data.pop_back();
                break;
            }
            else if (data[arg_pos] == '#' && !in_quote)
            {
                if (map_arg.data[map_arg.data.size() - 1] == ' ')
                    map_arg.data.pop_back();
                break ;
            }
            if (!in_quote)
                quote = this->is_quote(data[arg_pos]);
            if (quote && data[arg_pos] == quote)
            {
                in_quote = !in_quote;
                if (!in_quote)
                    quote = 0;
            }
            else
            {
                if (data[arg_pos] == ' ' && !in_quote)
                {
                    map_arg.data.push_back(' ');
                    this->SkipSpaces(data, arg_pos);
                    if (!data[arg_pos] || data[arg_pos] == '#' || data[arg_pos] == '\n')
                        break;
                    --arg_pos;
                }
                else if (data[arg_pos] == '#' && !in_quote)
                    break ;
                else
                    map_arg.data.push_back(data[arg_pos]);
            }
        }
        map_arg.size = i;
        if (map_arg.data[map_arg.data.size() - 1] == ' ')
            map_arg.data.pop_back();
        // if (is_alias)
        //     map_arg.data = 
        // std::cout << "Map argument: '" << map_arg.data << "'\n\n\n";
        return map_arg;
    }

    void
    CheckKindOfLiteral(uint& i)
    {
        (void)i;
    }

    void
    SkipComments(uint& i)
    {
        for (; i < Node::lines.size() && (Node::lines[i].is_comment || !Node::lines[i].data[0] || Node::lines[i].data[0] == '\n') ; ++i) ;
    }

    std::string
    FetchKeyname(std::string& keyname, uint& i, uint at)
    {
        char        quote = 0;
        bool        in_quote = false;
        uint        size = Node::lines[i].size;
        const char* data = Node::lines[i].data;

        if (size >= 2 && data[at] == '-' && data[at + 1] == ' ')
            return "";

        for (uint j = at ; j < size ; ++j)
        {
            if (!in_quote)
                quote = this->is_quote(data[j]);
            if (quote && data[j] == quote)
                in_quote = !in_quote;
            else if (data[j] == ':' && !in_quote)
                return keyname;
            else if (!in_quote)
                keyname.push_back(data[j]);
        }
        return "";
    }

    void
    PrintLineData(uint i)
    {
        const char* data = Node::lines[i].data;
        uint        size = Node::lines[i].size;

        for (uint j = 0 ; j < size ; ++j)
            printf("%d\n", data[j]);
        std::cout << std::endl;
    }

    std::string
    parse_string(const char* data, ulong& i)
    {
        (void)data;
        (void)i;
        return "";
    }

    Node&
    parse_list(const char* data, ulong& i)
    {
        (void)data;
        (void)i;
        return *this;
    }

    Node&
    parse_map(const char* data, ulong& i)
    {
        (void)data;
        (void)i;
        return *this;
    }

    void
    skip_line(const char* data, ulong& i)
    {
        for ( ; data[i] && data[i] != '\n' ; ++i)
            ;
        if (data[i] == '\n') ++i;
    }

    // skip comments too
    void
    skip_empty_lines(const char* data, ulong& i)
    {
        ulong   backup = i;
        for ( ; data[i] && (data[i] == ' ' || data[i] == '#') ; ++i)
        {
            if (data[i] == '#')
            {
                skip_line(data, i);
                backup = i;
            }
            if (data[i] == '\n')
                backup = i + 1;
        }
        if (data[i] != ' ')
            i = backup;
    }

    char
    is_quote(char c)
    {
        return (c == '\'' || c == '"') ? c : 0;
    }

    void
    SkipThisLine(const char* data, ulong& i)
    {
        for ( ; data[i] && data[i] != '\n' ; ++i) ;
        if (data[i] == '\n') ++i;
    }

    ulong
    NextLineOffset(const char* data, ulong i)
    {
        for ( ; data[i] && data[i] != '\n' ; ++i) ;
        if (data[i] == '\n') ++i;
        return i;
    }

    void
    SkipSpaces(const char* data, ulong& i)
    {
        for ( ; data[i] && data[i] == ' ' ; ++i) ;
    }

    uint
    NextLineIndentation(const char* data, ulong i)
    {
        this->SkipThisLine(data, i);
        return this->CountIndentation(data, i);
    }

    uint
    CountIndentation(const char* data, ulong& i)
    {
        uint    indent;
        // condition to enter in
        for ( ; data[i] && (data[i] == ' ' || data[i] == '\n' || data[i] == '#') ; )
        {
            // handle comments
            if (data[i] == '#')
            {
                for ( ; data[i] && data[i] != '\n' ; ++i);
                if (data[i] == '\n') ++i;
            }
            // count indentation (and reset if needed)
            indent = 0;
            for ( ; data[i] && data[i] == ' ' ; ++i, ++indent) ;
            // skip newline
            if (data[i] == '\n')
                ++i;
        }
        return indent;
    }

    void
    collectLinesMetrics(char* data)
    {
        ulong   i = 0;
        uint    line_index = 0;

        while (data[i])
        {
            SkipBlankLines(data, i);
            CreateLineMetrics(data, i, line_index);
        }
    }

    void
    SkipBlankLines(const char* data, ulong& i)
    {
        for ( ; data[i] && data[i] == '\n' ; ++i) ;
    }

    void
    DebugLinesMetrics(void)
    {
        for(uint i = 0 ; i < Node::lines.size() ; ++i)
        {
            // std::cout << "Block level: " << Node::lines[i].blockLevel;
            // std::cout << " | ";
            // std::cout << "Indentation size: " << Node::lines[i].indentationSize;
            // std::cout << " | ";
            // std::cout << "Is comment ? " << (Node::lines[i].is_comment ? "true" : "false") << std::endl;
            for (ulong j = 0 ; j < Node::lines[i].size ; ++j)
                std::cout << Node::lines[i].data[j];
            std::cout << std::endl;
        }
    }

    void
    CreateLineMetrics(char* data, ulong& i, uint& line_index)
    {
        bool    is_comment = false;
        uint    min_index = 0;
        uint    max_index = line_index > 0 ? line_index - 1 : 0;
        uint    current_indent_size = CountLineIndentation(data, i, is_comment);

        Node::lines.emplace_back(LineMetrics());
        Node::lines[line_index].indentationSize = current_indent_size;
        Node::lines[line_index].is_comment = is_comment;
        if (i == 0)
            Node::lines[line_index].blockLevel = 0;
        else if (i == 1)
            FetchCorrectBlockLevel(current_indent_size, 0, 0, max_index);
        else
        {
            for (ulong j = 0 ; j < line_index ; ++j)
            {
                if (!Node::lines[j].is_comment)
                {
                    if (current_indent_size <= Node::lines[j].indentationSize)
                        min_index = j;
                    if (current_indent_size >= Node::lines[j].indentationSize)
                        max_index = j;
                }
            }
            FetchCorrectBlockLevel(current_indent_size, line_index, min_index, max_index);
        }
        Node::lines[line_index].data = &data[i];

        ulong j = i;
        for (; data[i] && data[i] != '\n' ; ++i) ;
        if (data[i] == '\n')
        {
            Node::lines[line_index].size = i - j;
            ++i;
        }
        ++line_index;
    }

    inline
    void
    FetchCorrectBlockLevel(uint current_indent_size, uint line_index, uint min_index, uint max_index)
    {
        if (current_indent_size == Node::lines[max_index].indentationSize)
            Node::lines[line_index].blockLevel = Node::lines[max_index].blockLevel;
        else if (current_indent_size == Node::lines[min_index].indentationSize)
            Node::lines[line_index].blockLevel = Node::lines[min_index].blockLevel;
        else if (current_indent_size > Node::lines[max_index].indentationSize)
            Node::lines[line_index].blockLevel = Node::lines[max_index].blockLevel + 1;
        else if (current_indent_size < Node::lines[min_index].indentationSize)
            Node::lines[line_index].blockLevel = Node::lines[max_index].blockLevel - 1;
            // throw (std::exception()); // Bad indentation
    }

    void
    SkipLine(const char* data, ulong& i)
    {
        for (; data[i] && data[i] != '\n' ; ++i) ;
        if (data[i] == '\n') ++i;
    }

    uint
    CountLineIndentation(const char* data, ulong& i, bool& is_comment)
    {
        uint    indentSize = 0;

        if (data[i] == '#')
            is_comment = true;
        for (; data[i] && data[i] == ' ' ; ++i)
        {
            for (; data[i] == ' ' ; ++i)
            {
                ++indentSize;
            }
            if (data[i] == '#')
            {
                is_comment = true;
                return 0;
            }
            return indentSize;
        }
        return indentSize;
    }

    void
    __trimLastSpaces(std::string& identifier)
    {
        while (!identifier.empty() && identifier.back() == ' ')
            identifier.pop_back(); 
    }

    Node&
    __required(const char key[])
    {
        Node copy = this->require(key);  // Faire une copie d'abord
        *this = copy;                    // Puis assigner la copie
        return *this;
    }

    Node&
    __require(const char key[])
    {
        if (this->kind_ != Kinds::Map)
            throw (IsNotYamlMap("You can't access to the key %s because is not a map", key));
        else
        {
            if (this->Map().contains(key) == false)
                throw (KeyIsRequired("Key `%s` does not exist", key));
            return this->Map()[key];
        }
    }

    Kinds&
    __kind(void)
    {
        return this->kind_;
    }

    Node&
    at(const char key[])
    {
        if (this->kind_ == Kinds::Undefined)
        {
            static Node undefined_node(Kinds::Undefined);
            return undefined_node;
        }
        if (this->kind_ != Kinds::Map)
            throw (IsNotYamlMap("You can't access to the key %s because is not a map", key));
        else
        {
            if (this->Map().contains(key) == false)
                return this->return_or_throw(key);
            return this->Map()[key];
        }
    }

    Node&
    at(unsigned long index)
    {
        if (this->kind_ == Kinds::List)
            return this->List()[index];
        else if (this->kind_ == Kinds::Map)
        {
            if (index >= this->Map().size())
                throw (IndexNoReachable("Because index %ld is out of bounds (size = %ld)", index, this->size()));
            unsigned long     i = 0 ;
            auto    it = this->Map().begin();
            for (    ;
                    it != this->Map().end() && i < index ;
                    ++it, ++i) ;
            return it->second;
        }
        throw (IndexNoReachable("Index %ld no reachable %s", index, (this->kind_ != Kinds::Map && this->kind_ != Kinds::List) ? "because it is neither a map or a list" : "because it is out of bounds"));
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

    void
    SetFloat(TFloat v)
    {
        this->_union.emplace<TFloat>(v);
    }

    void
    SetInt(TInt v)
    {
        this->_union.emplace<TInt>(v);
    }

    void
    SetString(TString v)
    {
        this->_union.emplace<TString>(v);
    }

    void
    SetBoolean(TBool v)
    {
        this->_union.emplace<TBool>(v);
    }

    void
    AddList(Node& node)
    {
        try
        {
            this->List().emplace_back(node);
        } catch (...) {
            this->_union.emplace<TList>();
            this->List().emplace_back(node);
        }
    }

    void
    AddMap(const std::string& keyname, Node& node)
    {
        try
        {
            this->Map().emplace(std::pair<std::string, Node>(keyname, node));
        } catch (...) {
            this->_union.emplace<TMap>();
            this->Map().emplace(std::pair<std::string, Node>(keyname, node));
        }
    }

    inline thread_local static
    std::map<std::string, Node*>
    anchors;

    std::string
    parentKeyname;

    std::variant<
        TList,
        TString,
        TInt,
        TFloat,
        TBool,
        TMap
    >
    _union;

    Kinds
    kind_;

    bool
    not_undefined;

    bool
    mustThrow;

    inline thread_local static
    std::vector<LineMetrics>
    lines;
};

#endif
