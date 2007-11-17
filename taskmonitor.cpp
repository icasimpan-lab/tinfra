#include <string>
#include <vector>
#include <iostream>
#include <ostream>
#include <typeinfo>
#include <exception>
#include <fstream>

#include <tinfra/Symbol.h>
using namespace std;

#include <tinfra/tinfra.h>
#include <tinfra/tinfra_lex.h>
#include <tinfra/XMLPrinter.h>

using tinfra::Symbol;
using tinfra::ManagedType;

// 
// classes 
//

///
// Address
///

enum yesno {
    no, yes
};

namespace tinfra {
template<> 
struct LexicalInterpreter<yesno> {
    static void to_string(const yesno& v, std::string& dest) {
        static const string ys = "yes";
        static const string ns = "no";
        dest = (v == no) ? ns : ys;
    }
    static void from_string(const char* v, yesno& dest) {
        dest = (v[0] == 'y') ? yes : no;
    }
};
}
namespace S {
    // symbols
    extern Symbol active;
    extern Symbol closed;
    extern Symbol created;
    extern Symbol deadline;
    extern Symbol description;
    extern Symbol end;
    extern Symbol fileversion;
    extern Symbol fullname;
    extern Symbol group;
    extern Symbol hide;
    extern Symbol id;
    extern Symbol master_task;
    extern Symbol master_tasks;
    extern Symbol measure_id;
    extern Symbol members;
    extern Symbol members_data;
    extern Symbol name;
    extern Symbol persistent;
    extern Symbol process_measures;
    extern Symbol progress;
    extern Symbol project;
    extern Symbol project_phases;
    extern Symbol projects;
    extern Symbol projectphase_id;
    extern Symbol shortname;
    extern Symbol start;
    extern Symbol started;
    extern Symbol state;
    extern Symbol time_load;
    extern Symbol tasks;
    extern Symbol version;
}

// Member definition
struct Member {
    yesno   active;
    string  fullname;
    Symbol  id;
    string  shortname;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,    active);
        f(S::fullname,   fullname);
        f(S::id,  id);
        f(S::shortname,   shortname);
    }
};

// Group definition

struct Group {
    string fullname;
    vector<Member> members;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::fullname,  fullname);
        f(S::members,   members);
    }
};

struct Project {
    yesno    active;
    string   fullname;
    Symbol   id;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,  active);
        f(S::fullname,   fullname);
        f(S::id,  id);
    }
};

struct ProcessMeasure {
    Symbol   id;
    string   name;
        
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct ProjectPhase {
    Symbol   id;
    string   name;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::name,  name);
    }
};

struct MasterTask {
    Symbol id;
    Symbol project;
    string state;
    yesno  active;
    yesno  persistent;
    string description;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::id,  id);
        f(S::project,  project);
        f(S::state,  state);
        f(S::active,  active);
        f(S::persistent,  persistent);
        f(S::description, description);
    }
};

struct timestamp {
    long timestamp;
};

struct TimeLoadEntry {
    timestamp start;
    timestamp end;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::start,  start);
        f(S::end,    end);
    }
};

enum taskstate {
    prepared, active, closed
};

struct Task {
    string      name;
    Symbol      project;
    Symbol      master_task;
    timestamp   created;
    timestamp   started;
    int    progress;
    taskstate   state;
    timestamp   closed;
    timestamp   deadline;
    yesno       hide;
    Symbol      measure_id;
    Symbol      projectphase_id;
    string      description;
    vector<TimeLoadEntry> time_load;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::name,  name);
        f(S::project,  project);
        f(S::master_task,  master_task);
        f(S::created,  created);
        f(S::started,  started);
        f(S::progress,  progress);
        f(S::state,  state);
        f(S::closed,  closed);
        f(S::deadline, deadline);
        f(S::hide, hide);
        f(S::measure_id, measure_id);
        f(S::projectphase_id, projectphase_id);
        f(S::description, description);
        f(S::time_load, time_load);
    }
};

struct MemberData {
    yesno    active;
    Symbol   id;
    vector<Task>        tasks;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::active,  active);
        f(S::id,  id);
        f(S::tasks,  tasks);
    }
};
struct FileVersion {
    int version;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::version, version);
    }
};

struct TaskMonitor {
    FileVersion     fileversion;
    
    Group           group;
    
    vector<Project> projects;
    vector<ProcessMeasure> process_measures;
    
    vector<ProjectPhase>   project_phases;
    
    vector<MasterTask>     master_tasks;
    vector<MemberData>     members_data;
    
    template <typename F>
    void apply(F& f) const
    {
        f(S::group, group);
        f(S::fileversion,  fileversion);
        f(S::projects,  projects);
        f(S::process_measures,  process_measures);
        f(S::project_phases,  project_phases);
        f(S::master_tasks,  master_tasks);
        f(S::members_data,  members_data);
    }
};

namespace tinfra {
	template<> struct TypeTraits<TaskMonitor>: public ManagedStruct<TaskMonitor> {};
	template<> struct TypeTraits<Group>: public ManagedStruct<Group> {};
        template<> struct TypeTraits<Member>: public ManagedStruct<Member> {};
        template<> struct TypeTraits<MemberData>: public ManagedStruct<MemberData> {};
        template<> struct TypeTraits<Project>: public ManagedStruct<Project> {};
        template<> struct TypeTraits<ProjectPhase>: public ManagedStruct<ProjectPhase> {};
        template<> struct TypeTraits<ProcessMeasure>: public ManagedStruct<ProcessMeasure> {};
        template<> struct TypeTraits<MasterTask>: public ManagedStruct<MasterTask> {};
        template<> struct TypeTraits<FileVersion>: public ManagedStruct<FileVersion> {};
        template<> struct TypeTraits<Task>: public ManagedStruct<Task> {};
        template<> struct TypeTraits<TimeLoadEntry>: public ManagedStruct<TimeLoadEntry> {};
        template<> struct TypeTraits<timestamp>: public Fundamental<timestamp> {};
        template<> struct TypeTraits<taskstate>: public Fundamental<taskstate> {};

	template<typename T> struct TypeTraits<vector<T> >: public STLContainer<vector<T> > {};
}

ostream& operator << (ostream& o, const Symbol& s)  { return o << s.getName().c_str(); }
ostream& operator << (ostream& o, const timestamp& s)  { return o << s.timestamp; }
ostream& operator << (ostream& o, const taskstate& s)  { 
    return o << ( s == prepared ? "prepared" :
                  s == active   ? "active":
                  s == closed   ? "closed" : "<error>"); 
}
namespace S {
    // symbols
    Symbol active = "active";
    Symbol closed = "closed";;
    Symbol created = "created";;
    Symbol deadline = "deadline";;
    Symbol description = "description";;
    Symbol end = "end";;
    Symbol fileversion = "fileversion";
    Symbol fullname = "fullname";;
    Symbol group = "group";
    Symbol hide = "hide";
    Symbol id = "id";
    Symbol master_task = "master_task";
    Symbol master_tasks = "master_tasks";
    Symbol measure_id = "measure_id";
    Symbol members = "members";
    Symbol members_data = "members_data";
    Symbol name = "name";
    Symbol persistent = "persistent";
    Symbol process_measures = "process_measures";
    Symbol progress = "progress";
    Symbol project = "project";
    Symbol project_phases = "project_phases";
    Symbol projects = "projects";
    Symbol projectphase_id = "projectphase_id";
    Symbol shortname = "shortname";
    Symbol start = "start";
    Symbol started = "started";
    Symbol state = "state";
    Symbol time_load = "time_load";
    Symbol tasks = "tasks";
    Symbol version = "version";
}
void revert_mapping(xml::symbol_mapping const& src, xml::symbol_mapping& dest)
{
    for(xml::symbol_mapping::const_iterator i = src.begin(); i != src.end(); ++i )
        dest[i->second] = i->first;
}

void initialize_xml_out_mapping(xml::symbol_mapping& xml_out_mapping)
{
    xml_out_mapping[tinfra::TypeTraits<ProcessMeasure>::symbol()] = Symbol("process-measure");
    xml_out_mapping[tinfra::TypeTraits<ProjectPhase>::symbol()] = Symbol("project-phase");
    xml_out_mapping[tinfra::TypeTraits<Task>::symbol()] = Symbol("task");
    xml_out_mapping[tinfra::TypeTraits<TimeLoadEntry>::symbol()] = Symbol("time-load-entry");
    xml_out_mapping[tinfra::TypeTraits<MemberData>::symbol()] = Symbol("member-data");
    xml_out_mapping[tinfra::TypeTraits<Project>::symbol()] = Symbol("project");
    xml_out_mapping[tinfra::TypeTraits<Member>::symbol()] = Symbol("member");
    
    xml_out_mapping[S::process_measures] = Symbol("process-measures");
    xml_out_mapping[S::project_phases] = Symbol("project-phases");
    xml_out_mapping[S::master_tasks] = Symbol("master-tasks");
    xml_out_mapping[S::members_data] = Symbol("members-data");
    xml_out_mapping[S::time_load] = Symbol("time-load");    
    xml_out_mapping[tinfra::TypeTraits<ProjectPhase>::symbol()] = Symbol("project-phase");
}

void initialize_xml_in_mapping(xml::symbol_mapping& mapping)
{
    xml::symbol_mapping tmp;
    initialize_xml_out_mapping(tmp);
    revert_mapping(tmp, mapping);
}

void test1()
{
    TaskMonitor tm;
    tm.fileversion.version = 9;
    tm.group.fullname = "Group1";
    
    xml::symbol_mapping xml_symbol_mapping;
    initialize_xml_out_mapping(xml_symbol_mapping);
    
    tm.process_measures.push_back(tinfra::construct<ProcessMeasure>()
                                                           (S::id, Symbol("req-1"))
                                                           (S::name, string("XXX")));
    tm.project_phases.push_back(tinfra::construct<ProjectPhase>()
                                                           (S::id, Symbol("m1"))
                                                           (S::name, string("Test")));
    xml::XMLPrinter::write(cout, Symbol("taskmonitor"), tm, xml_symbol_mapping);
}

#include <expat.h>

class XMLStream {
public:
    enum event_type {
        START_TAG = 0,
        END_TAG = 1,
        CHAR_DATA = 2
    };
    struct Event {
        const char* tag_name() const { return _tag_name.c_str(); }
        const char* content() const { return _content.c_str(); }
        
        const char** args() const { return (const char**)_args; }
        Event()
        {
            _args = 0;
        }
        ~Event() {
            if( _args ) {
                char** ia = _args;
                while( *ia ) {
                    free(ia[0]); // name
                    free(ia[1]); // value
                    ia += 2;
                }
                delete[] _args;
            }
        }
        event_type type;
        string _tag_name;
        string _content;
        char** _args;
        
        void report()
        {
            cerr << "XMLEvent " << type << " name " << _tag_name.c_str() << endl;
        }
    };
    int cursor;
    XMLStream(): cursor(0)
    {
    }
    vector<Event*> events;
    
    void add_tag_start(const char* name, const char** args)
    {
        Event* e = new Event();
        e->type = START_TAG;
        e->_tag_name = name;
        int n = 0;
        {
            const char** ia = args;
            while( *ia ) {
                ia += 2;
                n++;
            }
        }
        //cout << "TAG " << name << "(";
        {
            e->_args = new char*[n*2+1];
            char** idest = e->_args;
            const char** isrc  = args;
            while( *isrc ) {
                idest[0] = strdup(isrc[0]);
                idest[1] = strdup(isrc[1]);
                //cout << idest[0] << "=" << idest[1] << " ";
                idest += 2;
                isrc  += 2;
            }
            idest[0] = 0;
        }
        //cout << ")" << endl;
        events.push_back(e);
    }
    void add_tag_end(const char* name)
    {
        Event* e = new Event();
        e->type = END_TAG;
        e->_tag_name = name;
        events.push_back(e);
    }
    void add_chardata(const char* data, int len)
    {
        //cerr << "ignored char data ..." << endl;
    }
    Event* peek() {
        if( cursor < events.size() ) {
            //cerr << "PEEK: "; events[cursor]->report();
            return events[cursor];
        } else return NULL;
    }
    Event* read() {
        if( cursor < events.size() ) {
            //cerr << "READ: "; events[cursor]->report();
            return events[cursor++];
        } else {
            return NULL; // EOF
        }
    }
    
    ~XMLStream()
    {
        for( vector<Event*>::iterator i = events.begin(); i != events.end(); ++i ) delete *i;
        events.erase(events.begin(), events.end());
    }
};


class XMLStreamReader {
    
    XML_Parser parser;
    XMLStream& target;
    
public:
    XMLStreamReader(XMLStream& target): target(target) 
    {
        initParser();
    }

    void initParser()
    {
        parser = XML_ParserCreate("UTF-8");        
        XML_SetStartElementHandler(parser, &XMLStreamReader::startElementHandler);
        XML_SetEndElementHandler(parser, &XMLStreamReader::endElementHandler);
        XML_SetCharacterDataHandler(parser, &XMLStreamReader::characterDataHandler);
        XML_SetUserData(parser, this);
    }
    bool parse(const char* filename)
    {
        std::filebuf in;
        in.open(filename,std::ios::in);
        if( !in.is_open() ) {
            cerr << "can't read file: " << filename << endl;
            return false;
        }
        return parse(in);
    }
    bool parse(std::streambuf& in)
    {
        while(true) {
            
            int BUFF_SIZE = 16384;
            
            char *buff = (char*)XML_GetBuffer(parser, BUFF_SIZE);
            if (buff == NULL) {
                cerr << "A" << endl; return false;
            }

            int bytes_read = in.sgetn(buff, BUFF_SIZE);
            if (bytes_read < 0) {
                cerr << "B" << endl; return false;
            }

            if (! XML_ParseBuffer(parser, bytes_read, bytes_read == 0)) {
                cerr << "XML parse error: " << XML_ErrorString(XML_GetErrorCode(parser)) << 
                        " at " << XML_GetCurrentLineNumber(parser) << ":" << XML_GetCurrentColumnNumber(parser) << endl;
                return false;
            }

            if (bytes_read == 0) return true;
        }
    }
    
    ~XMLStreamReader()
    {
        XML_ParserFree(parser);
    }
    
    void startElement(const char* name, const char** attributes)
    {
        //cout << "S" << name << endl;
        target.add_tag_start(name, attributes);
    }
    void endElement(const char* name)
    {
        target.add_tag_end(name);
    }
    void characterData(const char* data, int len)
    {
        target.add_chardata(data, len);
    }
    static void XMLCALL startElementHandler(void *userData,
                                   const XML_Char *name,
                                   const XML_Char **atts)
    {
        ((XMLStreamReader*)userData)->startElement(name, atts);
    }
    
    static void XMLCALL endElementHandler(void *userData, const XML_Char *name)
    {
        ((XMLStreamReader*)userData)->endElement(name);
    }
    
    static void XMLCALL characterDataHandler(void *userData, const XML_Char *s, int len)
    {
        ((XMLStreamReader*)userData)->characterData(s,len);
    }
};

class XMLStreamMutator {
    XMLStream& xml_stream;
    Symbol target_tag;
    const xml::symbol_mapping& mapping;
public:
    XMLStreamMutator(XMLStream& xml_stream, Symbol target,xml::symbol_mapping const& mapping): xml_stream(xml_stream), target_tag(target),mapping(mapping) {}
    
    template <typename T>
    void managed_struct(T& object, const tinfra::Symbol& object_symbol)
    {
        if( object_symbol != target_tag ) return;
        //cerr << "XMLStreamMutator::managed_struct( symbol=" << object_symbol.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;
        {
            XMLStream::Event* e = seek_to_start(object_symbol);
            if( !e ) return;
            apply_args<T>(object, e->args());
        }        
        
        XMLStream::Event* e = xml_stream.peek();
        while(e) {
            if( e == 0 ) return;
            if( e->type == XMLStream::START_TAG) {
                Symbol subtag_symbol = map(e->tag_name());
                XMLStreamMutator subtag_processor(xml_stream, subtag_symbol, mapping);
                tinfra::tt_mutate<T>(object, subtag_processor);
                e = xml_stream.peek();
            } else if( e->type == XMLStream::END_TAG) {
                e = xml_stream.read();
                break;
            } else if( e->type == XMLStream::CHAR_DATA) {
                // TODO: don't know how to deal with them so
                e = xml_stream.read(); // ignore
            } else {
                e = xml_stream.read(); // ignore all the rest
            }
        }
        //cerr << " __exit XMLStreamMutator::managed_struct( symbol=" << object_symbol.c_str() << ")" << endl;
    }
    template <typename T>
    void list_container(T& container, tinfra::Symbol const& container_symbol, tinfra::Symbol const& item_symbol)
    {   
        if( container_symbol != target_tag ) return;
        //cerr << "XMLStreamMutator::list_container( list ... symbol=" << container_symbol.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;        
        {
            XMLStream::Event* e = seek_to_start(container_symbol);
            if( !e ) return;
            //apply_args<T>(object, e->args());
        }
        
        XMLStream::Event* e = xml_stream.peek();
        while(e) {
            if( e->type == XMLStream::START_TAG) {
                Symbol subtag_symbol = map(e->tag_name());
                XMLStreamMutator subtag_processor(xml_stream, subtag_symbol, mapping);
                typename T::value_type element;
                tinfra::TypeTraits<typename T::value_type>::mutate(element, subtag_symbol, subtag_processor);
                container.push_back(element);
                e = xml_stream.peek();
            } else if( e->type == XMLStream::END_TAG) {
                e = xml_stream.read();
                return;
            } else if( e->type == XMLStream::CHAR_DATA) {
                // TODO: don't know how to deal with them so
                e = xml_stream.read(); // ignore
            } else {
                e = xml_stream.read(); // ignore all the rest
            }
        }
           //cerr << "XMLStreamMutator::list_container( list ... symbol=" << container_symbol.c_str() << ")" << endl;        
     
    }
    
    template<typename T>
    void operator()(tinfra::Symbol const& a, T& target) {
        if( !(a == target_tag) ) return;
        cerr << "WTF ! XMLStreamMutator::()( symbol=" << a.c_str() << " type=" << tinfra::TypeTraits<T>::name() << ")" << endl;
        if( seek_to_start(a.c_str()) )
            seek_to_end(a.c_str());
    }
    
private:
    XMLStream::Event* seek_to_start(Symbol name)
    {
        //cerr << "XMLStreamMutator::seek_to_start(" << name << ")" << endl;
        while(true) {
            XMLStream::Event* e = xml_stream.read();
            if( e == 0 ) {
                return 0; // FIXME: it's an error
            }            
            if( e->type == XMLStream::START_TAG) {
                Symbol b = map(e->tag_name());
                if( b == name) {
                    return e;
                }                
            } else if( e->type == XMLStream::END_TAG) {
                cerr << "XMLStreamMutator::seek_to_start(" << name << ") FAILED END" << endl;
                return 0;
            }
        }
        //cerr << "XMLStreamMutator::seek_to_start(" << name << ") FAILED ???" << endl;
        return 0; // XXX: or throw
    }
    XMLStream::Event* seek_to_end(const char* tag_name)
    {
        //cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ")" << endl;
        while(true) {
            XMLStream::Event* e = xml_stream.read();
            if( e == 0 ) {
                return 0; // FIXME: it's an error
            }            
            if( e->type == XMLStream::END_TAG) {                
                if( strcmp(e->tag_name(),tag_name) == 0 ) {
                    cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ") OK!" << endl;
                    return e;
                }                
            } else if( e->type == XMLStream::START_TAG) {
                seek_to_end(e->tag_name());
            }
        }
        //cerr << "XMLStreamMutator::seek_to_end(" << tag_name << ") FAILED ???" << endl;
        return 0; // XXX: or throw
    }
    
    template<typename T>
    void apply_args(T& target, const char** args)
    {
        const char** ia = args;        
        if( !ia ) return;        
        while( *ia ) {
            const char* name = map(ia[0]);
            const char* value = ia[1];
            cerr << tinfra::TypeTraits<T>::name() << ":" << name << " <= " << value << endl;
            tinfra::lexical_set<T>(target, tinfra::Symbol(name), value);
            ia += 2;
        }
    }
    
    tinfra::Symbol map(tinfra::Symbol const& other) {
        if( mapping.size() == 0 ) return other;
        xml::symbol_mapping::const_iterator r = mapping.find(other);
        if( r == mapping.end() ) return other;
        return r->second;
    }
};


template <typename T>
void xml_read(XMLStream& xmlStream, const Symbol& root, T& target, xml::symbol_mapping const mapping)
{
    XMLStreamMutator subtag_processor(xmlStream,  root, mapping);
    tinfra::TypeTraits<T>::mutate(target, root, subtag_processor);

}

void test2()
{
    TaskMonitor tm;
    {
        xml::symbol_mapping xml_symbol_mapping;
        initialize_xml_in_mapping(xml_symbol_mapping);
        
        XMLStream s;
        XMLStreamReader sr(s);
        sr.parse("taskmonitor2.xml");
        
        xml_read<TaskMonitor>(s, Symbol("taskmonitor"), tm, xml_symbol_mapping);
        cout << "readed!" << endl;
    }
    {
        xml::symbol_mapping xml_symbol_mapping;
        initialize_xml_out_mapping(xml_symbol_mapping);
        
        xml::XMLPrinter::write(cout, Symbol("taskmonitor"), tm, xml_symbol_mapping);
        cout << "written!" << endl;
    }
    cout << "bye!" << endl;
}


int main2()
{
    //test1();
    test2();
    return 0;
}

int main()
{
    try {
        main2();
    } catch( std::exception& e) {
        cerr << "fatal error: " << e.what() << endl;
    }
}
